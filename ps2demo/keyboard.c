// keyboards/bjl/ps2demo/keyboard.c
#include "keyboard.h"
#include "ps2_device.h"
#include "print.h"  // For debug output

// Mode state
static bool usb_mode = true;  // true = USB, false = PS/2

// Typematic repeat state (for PS/2 mode)
static struct {
    uint8_t scancode;       // Current held key scancode
    bool active;            // Is a key being held?
    uint32_t press_time;    // When key was pressed
    uint32_t last_repeat;   // When last repeat was sent
    uint16_t repeat_delay;  // Delay before repeat starts (ms)
    uint16_t repeat_rate;   // Time between repeats (ms)
} typematic = {
    .scancode = 0,
    .active = false,
    .press_time = 0,
    .last_repeat = 0,
    .repeat_delay = 500,    // 500ms delay before repeat (standard)
    .repeat_rate = 33       // ~30 repeats per second (standard)
};

// Helper functions for mode detection
bool is_usb_mode(void) {
    return usb_mode;
}

bool is_ps2_mode(void) {
    return !usb_mode;
}

// Early initialization - before USB or anything else
void keyboard_pre_init_kb(void) {
    // Initialize mode switch pin early
    setPinInputHigh(MODE_SWITCH_PIN);

    keyboard_pre_init_user();
}

// Post initialization - after USB is ready
void keyboard_post_init_kb(void) {
    // Check initial mode state - read multiple times to ensure stability
    usb_mode = readPin(MODE_SWITCH_PIN);
    wait_ms(10);  // Small delay for pin to settle
    usb_mode = readPin(MODE_SWITCH_PIN);

    // Debug output
    uprintf("================================\n");
    uprintf("Mode switch pin (GP14) state: %d\n", usb_mode);
    uprintf("Mode: %s\n", usb_mode ? "USB" : "PS/2");
    uprintf("================================\n");

    if (!usb_mode) {
        // Initialize PS/2 mode if switch is in PS/2 position
        uprintf("[PS2] Initializing PS/2 device...\n");
        ps2_device_init(PS2_CLOCK_PIN, PS2_DATA_PIN);
        uprintf("[PS2] PS/2 device initialized (CLK=GP16, DATA=GP17)\n");
        uprintf("[PS2] Typematic: delay=%dms, rate=%dms\n", typematic.repeat_delay, typematic.repeat_rate);
    } else {
        uprintf("[USB] USB mode selected\n");
    }

    keyboard_post_init_user();
}

// Main task loop - runs continuously
void housekeeping_task_kb(void) {
    // Check if mode switch has changed
    bool new_mode = readPin(MODE_SWITCH_PIN);

    if (new_mode != usb_mode) {
        usb_mode = new_mode;
        uprintf("\n!!! MODE CHANGE DETECTED !!!\n");
        uprintf("Mode switched to: %s\n", usb_mode ? "USB" : "PS/2");

        if (!usb_mode) {
            // Switched to PS/2 mode - initialize PS/2
            uprintf("[PS2] Initializing PS/2 on mode switch...\n");
            ps2_device_init(PS2_CLOCK_PIN, PS2_DATA_PIN);
            uprintf("[PS2] PS/2 reinitialized\n");
        } else {
            uprintf("[USB] Switched to USB mode\n");
            // Clear typematic state when switching to USB
            typematic.active = false;
        }
        uprintf("!!! MODE CHANGE COMPLETE !!!\n\n");
    }

    // Run PS/2 task if in PS/2 mode
    if (!usb_mode) {
        ps2_device_task();

        // Handle typematic repeat
        if (typematic.active) {
            uint32_t now = timer_read32();

            // Check if we should start repeating
            if ((now - typematic.press_time) >= typematic.repeat_delay) {
                // Check if it's time for another repeat
                if ((now - typematic.last_repeat) >= typematic.repeat_rate) {
                    // Send make code again
                    if (ps2_device_send_key_make(typematic.scancode)) {
                        typematic.last_repeat = now;
                        uprintf("[PS2] Typematic repeat: 0x%02X\n", typematic.scancode);
                    }
                }
            }
        }
    }

    housekeeping_task_user();
}

// Process all key presses
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // Debug output
    uprintf("[KEY] keycode=%d, pressed=%d, mode=%s\n",
            keycode, record->event.pressed, usb_mode ? "USB" : "PS/2");

    // Let user keymap process first
    if (!process_record_user(keycode, record)) {
        return false;
    }

    // If in PS/2 mode, send via PS/2 protocol
    if (!usb_mode) {
        uprintf("[PS2] PS/2 mode active - processing key\n");

        // Map QMK keycodes to PS/2 scan codes
        switch (keycode) {
            case KC_A:
                if (record->event.pressed) {
                    // Key pressed - send make code
                    uprintf("[PS2] Sending PS/2 make code for A (0x1C)...\n");
                    bool result = ps2_device_send_key_make(PS2_SCANCODE_A_MAKE);
                    uprintf("[PS2] Make code queued: %s\n", result ? "success" : "FAILED (buffer full)");

                    // Start typematic repeat
                    typematic.scancode = PS2_SCANCODE_A_MAKE;
                    typematic.active = true;
                    typematic.press_time = timer_read32();
                    typematic.last_repeat = timer_read32();
                    uprintf("[PS2] Typematic repeat armed\n");
                } else {
                    // Key released - send break code
                    uprintf("[PS2] Sending PS/2 break code for A (0xF0 0x1C)...\n");
                    bool result = ps2_device_send_key_break(PS2_SCANCODE_A_BREAK);
                    uprintf("[PS2] Break code queued: %s\n", result ? "success" : "FAILED (buffer full)");

                    // Stop typematic repeat
                    typematic.active = false;
                    uprintf("[PS2] Typematic repeat stopped\n");
                }
                return false;  // Don't process through USB stack

            default:
                uprintf("[PS2] Unknown keycode in PS/2 mode - blocking\n");
                return false;  // Block other keys in PS/2 mode
        }
    }

    // USB mode - process normally
    uprintf("[USB] USB mode - processing normally\n");
    return true;
}

// LED update callback (USB mode only)
bool led_update_kb(led_t led_state) {
    return led_update_user(led_state);
}

// Matrix scanning override (optional)
void matrix_init_kb(void) {
    matrix_init_user();
}

void matrix_scan_kb(void) {
    matrix_scan_user();
}

// Allow user to configure typematic settings
void ps2_set_typematic_rate(uint16_t delay_ms, uint16_t rate_ms) {
    typematic.repeat_delay = delay_ms;
    typematic.repeat_rate = rate_ms;
    uprintf("[PS2] Typematic updated: delay=%dms, rate=%dms\n", delay_ms, rate_ms);
}
