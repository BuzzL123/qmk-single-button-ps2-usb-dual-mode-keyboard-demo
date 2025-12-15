// ps2_device.c
#include "ps2_device.h"
#include "quantum.h"  // QMK main header with GPIO functions
#include <string.h>

// Configuration
static uint8_t ps2_clk_pin;
static uint8_t ps2_data_pin;

// State variables
static ps2_state_t ps2_state = PS2_STATE_IDLE;
static bool ps2_enabled = true;
static ps2_led_state_t ps2_leds = {0};
static uint8_t current_scancode_set = 2;

// Send buffer
#define PS2_SEND_BUFFER_SIZE 16
static uint8_t send_buffer[PS2_SEND_BUFFER_SIZE];
static uint8_t send_buffer_head = 0;
static uint8_t send_buffer_tail = 0;

// Timing (in microseconds)
#define PS2_CLK_HALF_PERIOD 40  // 40us = ~12.5kHz clock
#define PS2_INTER_BYTE_DELAY 2  // 2ms delay between bytes

// Helper functions using QMK GPIO API
static inline void ps2_clk_high(void) {
    setPinInput(ps2_clk_pin);  // Release to pullup (high-Z with pullup)
}

static inline void ps2_clk_low(void) {
    writePinLow(ps2_clk_pin);
    setPinOutput(ps2_clk_pin);
}

static inline void ps2_data_high(void) {
    setPinInput(ps2_data_pin);  // Release to pullup (high-Z with pullup)
}

static inline void ps2_data_low(void) {
    writePinLow(ps2_data_pin);
    setPinOutput(ps2_data_pin);
}

static inline bool ps2_clk_read(void) {
    return readPin(ps2_clk_pin);
}

static inline bool ps2_data_read(void) {
    return readPin(ps2_data_pin);
}

static bool ps2_send_byte(uint8_t data) {
    uint8_t parity = 1;

    // Start bit
    ps2_data_low();
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD);

    // Data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        if (data & (1 << i)) {
            ps2_data_high();
            parity ^= 1;
        } else {
            ps2_data_low();
        }
        wait_us(PS2_CLK_HALF_PERIOD);
        ps2_clk_low();
        wait_us(PS2_CLK_HALF_PERIOD);
        ps2_clk_high();
        wait_us(PS2_CLK_HALF_PERIOD);
    }

    // Parity bit (odd parity)
    if (parity) {
        ps2_data_high();
    } else {
        ps2_data_low();
    }
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD);

    // Stop bit - data MUST stay high
    ps2_data_high();
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD);
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD);

    // CRITICAL: Ensure stable idle state before returning
    // Both clock and data must be high (released to pullup)
    ps2_data_high();  // Ensure data is released (high)
    ps2_clk_high();   // Ensure clock is released (high)

    // Wait for lines to stabilize - this prevents the next start bit
    // from being detected prematurely
    wait_us(PS2_CLK_HALF_PERIOD * 4);  // Extra time for idle state

    return true;
}

static void ps2_handle_command(uint8_t cmd) {
    switch (cmd) {
        case PS2_CMD_SET_LEDS:
            ps2_send_byte(PS2_ACK);
            ps2_state = PS2_STATE_WAIT_RESPONSE;
            break;

        case PS2_CMD_ECHO:
            ps2_send_byte(PS2_ECHO_RESPONSE);
            break;

        case PS2_CMD_SET_SCANCODE_SET:
            ps2_send_byte(PS2_ACK);
            ps2_state = PS2_STATE_WAIT_RESPONSE;
            break;

        case PS2_CMD_IDENTIFY:
            ps2_send_byte(PS2_ACK);
            ps2_send_byte(0xAB);  // Keyboard ID
            ps2_send_byte(0x83);  // MF2 keyboard
            break;

        case PS2_CMD_SET_TYPEMATIC:
            ps2_send_byte(PS2_ACK);
            ps2_state = PS2_STATE_WAIT_RESPONSE;
            break;

        case PS2_CMD_ENABLE:
            ps2_enabled = true;
            ps2_send_byte(PS2_ACK);
            break;

        case PS2_CMD_DISABLE:
            ps2_enabled = false;
            ps2_send_byte(PS2_ACK);
            break;

        case PS2_CMD_SET_DEFAULTS:
            ps2_enabled = true;
            current_scancode_set = 2;
            ps2_leds.scroll_lock = 0;
            ps2_leds.num_lock = 0;
            ps2_leds.caps_lock = 0;
            ps2_send_byte(PS2_ACK);
            break;

        case PS2_CMD_RESET:
            ps2_send_byte(PS2_ACK);
            wait_ms(100);  // BAT takes time
            ps2_send_byte(PS2_BAT_SUCCESS);
            break;

        default:
            ps2_send_byte(PS2_ACK);
            break;
    }
}

void ps2_device_init(uint8_t clk_pin, uint8_t data_pin) {
    ps2_clk_pin = clk_pin;
    ps2_data_pin = data_pin;

    // Initialize pins with pull-ups using QMK functions
    setPinInputHigh(ps2_clk_pin);   // Input with pullup
    setPinInputHigh(ps2_data_pin);  // Input with pullup

    // Start with pins as inputs (idle high)
    ps2_clk_high();
    ps2_data_high();

    ps2_state = PS2_STATE_IDLE;
    ps2_enabled = true;
}

void ps2_device_task(void) {
    static uint32_t last_send_time = 0;

    // Check if host is trying to send us data (clock held low)
    // For now, we're ignoring host-to-device communication
    // This would be implemented here if needed
    (void)ps2_clk_read();  // Suppress unused warning
    (void)ps2_data_read(); // Suppress unused warning

    // Send queued data if available
    if (send_buffer_head != send_buffer_tail && ps2_state == PS2_STATE_IDLE) {
        // Add inter-byte delay to ensure receiver is ready
        uint32_t now = timer_read32();
        if (last_send_time != 0 && (now - last_send_time) < PS2_INTER_BYTE_DELAY) {
            return;  // Wait longer before sending next byte
        }

        uint8_t data = send_buffer[send_buffer_tail];
        ps2_state = PS2_STATE_SENDING;

        if (ps2_send_byte(data)) {
            send_buffer_tail = (send_buffer_tail + 1) % PS2_SEND_BUFFER_SIZE;
            last_send_time = timer_read32();
        }

        ps2_state = PS2_STATE_IDLE;
    }
}

bool ps2_device_send_key_make(uint8_t scancode) {
    if (!ps2_enabled) return false;

    uint8_t next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) return false; // Buffer full

    send_buffer[send_buffer_head] = scancode;
    send_buffer_head = next_head;

    return true;
}

bool ps2_device_send_key_break(uint8_t scancode) {
    if (!ps2_enabled) return false;

    // Send break prefix (0xF0) then scancode
    uint8_t next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) return false;

    send_buffer[send_buffer_head] = PS2_SCANCODE_A_BREAK_PREFIX;
    send_buffer_head = next_head;

    next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) return false;

    send_buffer[send_buffer_head] = scancode;
    send_buffer_head = next_head;

    return true;
}

ps2_led_state_t ps2_device_get_leds(void) {
    return ps2_leds;
}

bool ps2_device_is_enabled(void) {
    return ps2_enabled;
}

// Keep ps2_handle_command available for future host-to-device implementation
// Mark as used to avoid compiler warnings
void __attribute__((used)) ps2_device_process_host_command(uint8_t cmd) {
    ps2_handle_command(cmd);
}
