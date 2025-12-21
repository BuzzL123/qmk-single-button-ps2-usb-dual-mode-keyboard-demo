// ps2_keyboard.c - FIXED VERSION with better media key debugging
#include "ps2_keyboard.h"
#include "quantum.h"  // QMK main header with GPIO functions

#include "report.h"  // For report_keyboard_t, etc.

// Timing (in microseconds)
#define PS2_CLK_HALF_PERIOD 50  // 50us = 10kHz clock (was 40us = 12.5kHz)
#define PS2_INTER_BYTE_DELAY 2  // 2ms delay between bytes

// Previous keyboard report to detect key changes device
static report_keyboard_t previous_report = {0};

// Configuration
static uint8_t ps2_clk_pin;
static uint8_t ps2_data_pin;

// State variables
static ps2_state_t ps2_state = PS2_STATE_IDLE;
static bool ps2_enabled = true;
static ps2_led_state_t ps2_leds = {0};

// Special key send functions
bool ps2_keyboard_send_printscreen_make(void);
bool ps2_keyboard_send_printscreen_break(void);
bool ps2_keyboard_send_pause(void);  // Pause only sends on make, no break!
bool ps2_keyboard_send_raw_byte(uint8_t byte);

// Send buffer
#define PS2_SEND_BUFFER_SIZE 32
static uint8_t send_buffer[PS2_SEND_BUFFER_SIZE];
static uint8_t send_buffer_head = 0;
static uint8_t send_buffer_tail = 0;

// Previous media key to handle repeats
static uint16_t previous_media_key = 0;

// Convert Consumer Control usage code to PS/2 scancode
static ps2_mapping_t consumer_to_ps2_scancode(uint16_t usage) {
    for (size_t i = 0; i < PS2_CONSUMER_MAPPINGS_SIZE; i++) {
        if (ps2_consumer_mappings[i].usage_code == usage) {
            return ps2_consumer_mappings[i].mapping;
        }
    }
    // Unknown consumer control code
    uprintf("[PS2] UNMAPPED consumer control: 0x%04X\n", usage);
    return (ps2_mapping_t){0, false, PS2_KEY_NORMAL};
}

// Convert QMK keycode to PS/2 scancode
ps2_mapping_t qmk_to_ps2_scancode(uint16_t keycode) {
    // FIXED: Check basic keycodes in main lookup table
    // Only use the lookup table if keycode is within bounds AND < 256
    if (keycode < 0x100 && keycode < PS2_SCANCODE_LOOKUP_SIZE) {
        ps2_mapping_t mapping = ps2_scancode_lookup[keycode];
        if (mapping.scancode != 0) {
            return mapping;
        }
    }

    // Check extended keys (media, system, F13-F24)
    // This handles all keycodes >= 0x100 and any unmapped basic keys
    for (size_t i = 0; i < PS2_EXTENDED_KEYS_SIZE; i++) {
        if (ps2_extended_keys[i].qmk_keycode == keycode) {
            return ps2_extended_keys[i].mapping;
        }
    }

    // Unknown keycode - log it for debugging
    uprintf("[PS2] UNMAPPED keycode: 0x%04X\n", keycode);
    return (ps2_mapping_t){0, false, PS2_KEY_NORMAL};
}

typedef struct {
    uint8_t mod_bit;
    uint8_t scancode;
    bool needs_e0;
    ps2_special_key_type_t special_type;
} ps2_modifier_mapping_t;

static const ps2_modifier_mapping_t modifier_mappings[] = {
    {0x01, PS2_LCTRL, false, PS2_KEY_NORMAL},   // MOD_LCTL
    {0x02, PS2_LSHIFT, false, PS2_KEY_NORMAL},  // MOD_LSFT
    {0x04, PS2_LALT, false, PS2_KEY_NORMAL},    // MOD_LALT
    {0x08, PS2_LGUI, true, PS2_KEY_NORMAL},     // MOD_LGUI (needs E0)
    {0x10, PS2_RCTRL, true, PS2_KEY_NORMAL},    // MOD_RCTL (needs E0)
    {0x20, PS2_RSHIFT, false, PS2_KEY_NORMAL},  // MOD_RSFT
    {0x40, PS2_RALT, true, PS2_KEY_NORMAL},     // MOD_RALT (needs E0)
    {0x80, PS2_RGUI, true, PS2_KEY_NORMAL},     // MOD_RGUI (needs E0)
};

// Typematic state (Needed because PS/2 device must handle repeats itself unlike USB)
static struct {
    uint16_t keycode;       // Which QMK keycode is held
    bool active;            // Is typematic armed?
    uint32_t press_time;    // When key was first pressed
    uint32_t last_repeat;   // When we last sent a repeat
    uint16_t delay_ms;      // Delay before repeating starts
    uint16_t rate_ms;       // Time between repeats
    ps2_mapping_t mapping;  // Full mapping info (scancode + E0 prefix flag)
} typematic_state = {
    .keycode = 0,
    .active = false,
    .press_time = 0,
    .last_repeat = 0,
    .delay_ms = 500,  // Default 500ms delay
    .rate_ms = 33,     // Default ~30Hz repeat rate
    .mapping = {0, false, PS2_KEY_NORMAL}
};

void ps2_keyboard_typematic_arm(uint16_t keycode, uint8_t scancode) {
    // Don't arm typematic for modifier keys
    if ((keycode >= KC_LCTL && keycode <= KC_RGUI) ||  // Modifiers
        keycode == KC_CAPS ||  // Caps Lock
        keycode == KC_NUM ||   // Num Lock
        keycode == KC_SCRL ||  // Scroll Lock
        keycode == KC_PSCR ||  // Print Screen
        keycode == KC_PAUS) {  // Pause
        return;
    }

    typematic_state.keycode = keycode;
    typematic_state.active = true;
    typematic_state.press_time = timer_read32();
    typematic_state.last_repeat = timer_read32();

    // Store the complete mapping to preserve E0 prefix info
    typematic_state.mapping = qmk_to_ps2_scancode(keycode);
}

void ps2_keyboard_typematic_stop(uint16_t keycode) {
    if (typematic_state.keycode == keycode) {
        typematic_state.active = false;
    }
}

void ps2_keyboard_typematic_disable(void) {
    // Completely disable typematic (used when switching modes)
    typematic_state.active = false;
    typematic_state.keycode = 0;
    typematic_state.mapping.scancode = 0;
    typematic_state.mapping.needs_e0_prefix = false;
    typematic_state.mapping.special_type = PS2_KEY_NORMAL;
}

void ps2_keyboard_typematic_task(void) {
    if (!typematic_state.active) return;

    uint32_t now = timer_read32();
    uint32_t held_time = now - typematic_state.press_time;

    // Has initial delay passed?
    if (held_time >= typematic_state.delay_ms) {
        uint32_t since_repeat = now - typematic_state.last_repeat;

        // Time for another repeat?
        if (since_repeat >= typematic_state.rate_ms) {
            uprintf("[PS2] Typematic repeat: keycode=0x%04X, scancode=0x%02X%s\n",
                    typematic_state.keycode, typematic_state.mapping.scancode,
                    typematic_state.mapping.needs_e0_prefix ? ", E0 prefix" : "");

            if (typematic_state.mapping.needs_e0_prefix) {
                ps2_keyboard_send_key_make(PS2_PREFIX_E0);
            }
            ps2_keyboard_send_key_make(typematic_state.mapping.scancode);
            typematic_state.last_repeat = now;
        }
    }
}

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

    // Ensure idle state before starting
    ps2_data_high();
    ps2_clk_high();
    wait_us(100);  // Wait for idle

    // Start bit (data low, then clock pulse)
    ps2_data_low();
    wait_us(PS2_CLK_HALF_PERIOD * 2);  // Data setup time

    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD * 2);  // Clock low period
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD * 2);  // Clock high period

    // Data bits (LSB first)
    for (int i = 0; i < 8; i++) {
        // Set data line FIRST
        if (data & (1 << i)) {
            ps2_data_high();
            parity ^= 1;
        } else {
            ps2_data_low();
        }
        wait_us(PS2_CLK_HALF_PERIOD * 2);  // Data setup time

        // Then toggle clock
        ps2_clk_low();
        wait_us(PS2_CLK_HALF_PERIOD * 2);
        ps2_clk_high();
        wait_us(PS2_CLK_HALF_PERIOD * 2);
    }

    // Parity bit (odd parity)
    if (parity) {
        ps2_data_high();
    } else {
        ps2_data_low();
    }
    wait_us(PS2_CLK_HALF_PERIOD * 2);  // Data setup time

    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD * 2);
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD * 2);

    // Stop bit - data MUST be high
    ps2_data_high();
    wait_us(PS2_CLK_HALF_PERIOD * 2);

    ps2_clk_low();
    wait_us(PS2_CLK_HALF_PERIOD * 2);
    ps2_clk_high();
    wait_us(PS2_CLK_HALF_PERIOD * 2);

    // CRITICAL: Long inter-byte delay
    // Both clock and data must be high (idle) for sufficient time
    ps2_data_high();
    ps2_clk_high();
    wait_us(300);  // Much longer inter-byte delay (minimum 300us)

    return true;
}

static void ps2_handle_command(uint8_t cmd) {
    switch (cmd) {
        case PS2_CMD_SET_LEDS:
            // Wait for LED data byte (will be implemented when needed)
            break;

        // Echo back
        case PS2_CMD_ECHO:
            ps2_send_byte(PS2_ECHO_RESPONSE);
            break;

        // For now, we only support Set 2
        case PS2_CMD_SET_SCANCODE_SET:
            ps2_send_byte(PS2_ACK);
            break;

        // Respond with keyboard ID (AB 83)
        case PS2_CMD_IDENTIFY:
            ps2_send_byte(PS2_ACK);
            wait_ms(PS2_INTER_BYTE_DELAY);
            ps2_send_byte(0xAB);
            wait_ms(PS2_INTER_BYTE_DELAY);
            ps2_send_byte(0x83);
            break;

        // Enable/Disable commands
        case PS2_CMD_ENABLE:
            ps2_enabled = true;
            ps2_send_byte(PS2_ACK);
            break;

        // Disables keyboard sending
        case PS2_CMD_DISABLE:
            ps2_enabled = false;
            ps2_send_byte(PS2_ACK);
            break;

        // Set Defaults command
        case PS2_CMD_SET_DEFAULTS:
            ps2_send_byte(PS2_ACK);
            break;

        // Reset command
        case PS2_CMD_RESET:
            ps2_send_byte(PS2_ACK);
            wait_ms(PS2_INTER_BYTE_DELAY);
            ps2_send_byte(PS2_BAT_SUCCESS);
            break;

        default:
            ps2_send_byte(PS2_RESEND);
            break;
    }
}

void ps2_keyboard_init(uint8_t clk_pin, uint8_t data_pin) {
    ps2_clk_pin = clk_pin;
    ps2_data_pin = data_pin;

    // Set pins as inputs with pullups
    setPinInputHigh(ps2_clk_pin);
    setPinInputHigh(ps2_data_pin);

    ps2_enabled = true;
    ps2_state = PS2_STATE_IDLE;

    // Initialize LED state
    ps2_leds.caps_lock = 0;
    ps2_leds.num_lock = 0;
    ps2_leds.scroll_lock = 0;

    uprintf("[PS2] Device initialized on CLK=%d, DATA=%d\n", clk_pin, data_pin);
}

bool ps2_keyboard_send_printscreen_make(void) {
    // PrintScreen make: E0 12 E0 7C
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
    ps2_keyboard_send_raw_byte(0x12);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
    ps2_keyboard_send_raw_byte(PS2_PSCREEN);
    return true;
}

bool ps2_keyboard_send_printscreen_break(void) {
    // PrintScreen break: E0 F0 7C E0 F0 12
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_F0);
    ps2_keyboard_send_raw_byte(PS2_PSCREEN);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_F0);
    ps2_keyboard_send_raw_byte(0x12);
    return true;
}

bool ps2_keyboard_send_pause(void) {
    // Pause make: E1 14 77 E1 F0 14 F0 77
    // Pause has NO break code - only sends on make!
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E1);
    ps2_keyboard_send_raw_byte(0x14);
    ps2_keyboard_send_raw_byte(PS2_PAUSE);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_E1);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_F0);
    ps2_keyboard_send_raw_byte(0x14);
    ps2_keyboard_send_raw_byte(PS2_PREFIX_F0);
    ps2_keyboard_send_raw_byte(PS2_PAUSE);
    return true;
}

static bool ps2_buffer_has_space(uint8_t needed) {
    uint8_t used;
    if (send_buffer_head >= send_buffer_tail) {
        used = send_buffer_head - send_buffer_tail;
    } else {
        used = PS2_SEND_BUFFER_SIZE - (send_buffer_tail - send_buffer_head);
    }
    return (PS2_SEND_BUFFER_SIZE - used) >= needed;
}

void ps2_keyboard_task(void) {
    if (send_buffer_head != send_buffer_tail) {
        uint8_t byte = send_buffer[send_buffer_tail];
        if (ps2_send_byte(byte)) {
            send_buffer_tail = (send_buffer_tail + 1) % PS2_SEND_BUFFER_SIZE;
        }
    }

    ps2_keyboard_typematic_task();
}

bool ps2_keyboard_send_raw_byte(uint8_t byte) {
    uint8_t next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) {
        uprintf("[PS2] WARNING: Send buffer full! Dropping byte 0x%02X\n", byte);
        return false;
    }

    send_buffer[send_buffer_head] = byte;
    send_buffer_head = next_head;

    return true;
}

bool ps2_keyboard_send_key_make(uint8_t scancode) {
    if (!ps2_enabled) return false;

    uint8_t next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) {
        // Buffer full - this shouldn't happen in normal use!
        uprintf("[PS2] WARNING: Send buffer full! Dropping scancode 0x%02X\n", scancode);
        return false; // Buffer full
    }

    send_buffer[send_buffer_head] = scancode;
    send_buffer_head = next_head;

    return true;
}

bool ps2_keyboard_send_key_break(uint8_t scancode) {
    if (!ps2_enabled) return false;

    // Send break prefix (0xF0) then scancode
    uint8_t next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) return false;

    send_buffer[send_buffer_head] = PS2_PREFIX_F0;
    send_buffer_head = next_head;

    next_head = (send_buffer_head + 1) % PS2_SEND_BUFFER_SIZE;
    if (next_head == send_buffer_tail) return false;

    send_buffer[send_buffer_head] = scancode;
    send_buffer_head = next_head;

    return true;
}

ps2_led_state_t ps2_keyboard_get_leds(void) {
    return ps2_leds;
}

bool ps2_device_is_enabled(void) {
    return ps2_enabled;
}

static uint8_t ps2_keyboard_leds(void) {
    ps2_led_state_t leds = ps2_keyboard_get_leds();
    return (leds.caps_lock << 1) | (leds.num_lock) | (leds.scroll_lock << 2);
}

static void ps2_send_keyboard(report_keyboard_t *report) {
    if (report->keys[0] != 0 || report->keys[1] != 0) {
        uprintf("[PS2] Report contains keys: ");
        for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
            if (report->keys[i] != 0) {
                uprintf("0x%02X ", report->keys[i]);
            }
        }
        uprintf("\n");
    }

    // Handle modifier changes
    uint8_t mod_changes = previous_report.mods ^ report->mods;

    if (mod_changes) {
        for (uint8_t i = 0; i < 8; i++) {
            const ps2_modifier_mapping_t *mapping = &modifier_mappings[i];

            if (mod_changes & mapping->mod_bit) {
                bool is_pressed = report->mods & mapping->mod_bit;

                if (is_pressed) {
                    uprintf("[PS2] Modifier pressed: 0x%02X (scancode: 0x%02X%s)\n",
                            mapping->mod_bit, mapping->scancode,
                            mapping->needs_e0 ? ", E0 prefix" : "");
                    if (mapping->needs_e0) {
                        ps2_keyboard_send_key_make(PS2_PREFIX_E0);
                    }
                    ps2_keyboard_send_key_make(mapping->scancode);
                // ... inside ps2_send_keyboard ...
                } else {
                    uprintf("[PS2] Modifier released: 0x%02X (scancode: 0x%02X%s)\n",
                            mapping->mod_bit, mapping->scancode,
                            mapping->needs_e0 ? ", E0 prefix" : "");

                    // Corrected Release Sequence for Extended Modifiers
                    if (mapping->needs_e0) {
                        ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
                    }
                    ps2_keyboard_send_key_break(mapping->scancode);
                }
            }
        }
    }

    // Handle regular key releases
    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t prev_keycode = previous_report.keys[i];
        bool still_pressed = false;

        if (prev_keycode == 0) continue;

        for (int j = 0; j < KEYBOARD_REPORT_KEYS; j++) {
            if (report->keys[j] == prev_keycode) {
                still_pressed = true;
                break;
            }
        }

        if (!still_pressed) {
            ps2_mapping_t mapping = qmk_to_ps2_scancode(prev_keycode);
            if (mapping.scancode != 0) {
                if (mapping.special_type == PS2_KEY_PRINTSCREEN) {
                    ps2_keyboard_send_printscreen_break();
                    continue;
                }

                if (mapping.special_type == PS2_KEY_PAUSE) {
                    continue;
                }

                uprintf("[PS2] Key released: keycode=0x%04X, scancode=0x%02X%s\n",
                        prev_keycode, mapping.scancode,
                        mapping.needs_e0_prefix ? ", E0 prefix" : "");
                if (mapping.needs_e0_prefix) {
                    ps2_keyboard_send_key_make(PS2_PREFIX_E0);
                }
                ps2_keyboard_send_key_break(mapping.scancode);
                ps2_keyboard_typematic_stop(prev_keycode);
            }
        }
    }

    // Handle regular key presses
    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t keycode = report->keys[i];
        bool was_pressed = false;

        if (keycode == 0) continue;

        for (int j = 0; j < KEYBOARD_REPORT_KEYS; j++) {
            if (previous_report.keys[j] == keycode) {
                was_pressed = true;
                break;
            }
        }

        if (!was_pressed) {
            ps2_mapping_t mapping = qmk_to_ps2_scancode(keycode);
            if (mapping.scancode != 0) {
                if (mapping.special_type == PS2_KEY_PRINTSCREEN) {
                    ps2_keyboard_send_printscreen_make();
                    continue;
                }

                if (mapping.special_type == PS2_KEY_PAUSE) {
                    ps2_keyboard_send_pause();
                    continue;
                }

                uprintf("[PS2] Key pressed: keycode=0x%04X, scancode=0x%02X%s\n",
                        keycode, mapping.scancode,
                        mapping.needs_e0_prefix ? ", E0 prefix" : "");
                if (mapping.needs_e0_prefix) {
                    ps2_keyboard_send_key_make(PS2_PREFIX_E0);
                }
                ps2_keyboard_send_key_make(mapping.scancode);
                ps2_keyboard_typematic_arm(keycode, mapping.scancode);
            }
        }
    }

    previous_report = *report;
}

static void ps2_send_nkro(report_nkro_t *report) {
    // Not implemented
}

static void ps2_send_mouse(report_mouse_t *report) {
    // Not implemented
}

// Handle media/consumer keys - FIXED VERSION
static void ps2_send_extra(report_extra_t *report) {
    uint16_t current_media_key = 0;

    if (report->report_id == REPORT_ID_CONSUMER) {
        current_media_key = report->usage;
        uprintf("[PS2] Extra key report: usage=0x%04X\n", current_media_key);
    }

    if (current_media_key != previous_media_key) {
        // 1. Handle Release (Break)
        if (previous_media_key != 0) {
            // USE CONSUMER MAPPING for consumer control codes
            ps2_mapping_t mapping = consumer_to_ps2_scancode(previous_media_key);
            // Ensure space for: E0 (prefix) + F0 (break) + Scancode = 3 bytes
            if (mapping.scancode != 0 && ps2_buffer_has_space(3)) {
                uprintf("[PS2] Media key RELEASE: usage=0x%04X, scancode=0x%02X%s\n",
                        previous_media_key, mapping.scancode,
                        mapping.needs_e0_prefix ? ", E0 prefix" : "");
                if (mapping.needs_e0_prefix) {
                    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
                }
                ps2_keyboard_send_key_break(mapping.scancode);
            } else if (mapping.scancode == 0) {
                uprintf("[PS2] WARNING: Previous consumer code 0x%04X has no PS/2 mapping!\n", previous_media_key);
            }
        }

        // 2. Handle Press (Make)
        if (current_media_key != 0) {
            // USE CONSUMER MAPPING for consumer control codes
            ps2_mapping_t mapping = consumer_to_ps2_scancode(current_media_key);
            // Ensure space for: E0 (prefix) + Scancode = 2 bytes
            if (mapping.scancode != 0 && ps2_buffer_has_space(2)) {
                uprintf("[PS2] Media key PRESS: usage=0x%04X, scancode=0x%02X%s\n",
                        current_media_key, mapping.scancode,
                        mapping.needs_e0_prefix ? ", E0 prefix" : "");
                if (mapping.needs_e0_prefix) {
                    ps2_keyboard_send_raw_byte(PS2_PREFIX_E0);
                }
                ps2_keyboard_send_key_make(mapping.scancode);

                // NOTE: We do NOT call ps2_keyboard_typematic_arm() here
                // because media keys should not repeat in PS/2.
            } else if (mapping.scancode == 0) {
                uprintf("[PS2] WARNING: Current consumer code 0x%04X has no PS/2 mapping!\n", current_media_key);
            }
        }
        previous_media_key = current_media_key;
    }
}

// Create the driver struct
host_driver_t ps2_keyboard_host_driver = {
    .keyboard_leds = ps2_keyboard_leds,
    .send_keyboard = ps2_send_keyboard,
    .send_nkro = ps2_send_nkro,
    .send_mouse = ps2_send_mouse,
    .send_extra = ps2_send_extra,
};

// Keep ps2_handle_command available for future host-to-device implementation
// Mark as used to avoid compiler warnings
void __attribute__((used)) ps2_device_process_host_command(uint8_t cmd) {
    ps2_handle_command(cmd);
}
