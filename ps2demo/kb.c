// keyboards/bjl/ps2demo/kb.c - FIXED VERSION with proper USB driver restoration
#include "kb.h"
#include "ps2_keyboard.h"
#include "print.h"
#include "host.h"

// Mode state
static bool usb_mode = true;
static bool last_mode = true;

// Store original USB driver to restore later
static host_driver_t *original_usb_driver = NULL;

bool is_usb_mode(void) {
    return usb_mode;
}

bool is_ps2_mode(void) {
    return !usb_mode;
}

void keyboard_pre_init_kb(void) {
    setPinInputHigh(MODE_SWITCH_PIN);
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    static uint32_t mode_change_time = 0;
    bool current_mode = readPin(MODE_SWITCH_PIN);

    // Check for mode mismatch (current pin vs last known mode)
    // This handles both runtime switching AND initial boot detection
    if (current_mode != last_mode) {
        if (mode_change_time == 0) {
            mode_change_time = timer_read32();
        } else if (timer_elapsed32(mode_change_time) > 50) {
            // Mode stable for 50ms - switch!
            last_mode = current_mode;
            usb_mode = current_mode;
            mode_change_time = 0;

            uprintf("================================\n");
            uprintf("Mode switch: %s\n", usb_mode ? "USB" : "PS/2");
            uprintf("================================\n");

            if (!usb_mode) {
                // ===== Switching TO PS/2 =====

                // CRITICAL: Capture the driver here, where we know it is valid
                if (original_usb_driver == NULL) {
                    original_usb_driver = host_get_driver();
                }

                // Clear USB keyboard state while USB driver is still active
                clear_keyboard();
                wait_ms(20);

                // Now switch to PS/2
                ps2_keyboard_init(PS2_KEYBOARD_CLOCK_PIN, PS2_KEYBOARD_DATA_PIN);
                host_set_driver(&ps2_keyboard_host_driver);
                uprintf("[PS2] PS/2 driver activated\n");

            } else {
                // ===== Switching TO USB =====
                // Disable PS/2 typematic first
                ps2_keyboard_typematic_disable();

                // Restore the original USB driver
                if (original_usb_driver != NULL) {
                    host_set_driver(original_usb_driver);
                    uprintf("[USB] USB driver restored\n");
                } else {
                    uprintf("[USB] ERROR: original_usb_driver is NULL!\n");
                }

                wait_ms(20);

                // Clear keyboard state in USB mode
                clear_keyboard();
                send_keyboard_report();

                wait_ms(20);
            }
        }
    } else {
        mode_change_time = 0;
    }

    // Run PS/2 task only in PS/2 mode
    if (!usb_mode) {
        ps2_keyboard_task();
    }

    housekeeping_task_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // In PS/2 mode, ensure the correct driver is set BEFORE processing
    if (!usb_mode) {
        if (host_get_driver() != &ps2_keyboard_host_driver) {
            uprintf("[ERROR] Wrong driver in PS/2 mode! Fixing...\n");
            host_set_driver(&ps2_keyboard_host_driver);
        }
    }

    if (!process_record_user(keycode, record)) {
        return false;
    }

    if (record->event.pressed) {
        uprintf("[DEBUG] Key pressed: keycode=0x%04X (%s mode)\n",
                keycode, usb_mode ? "USB" : "PS/2");
    } else {
        uprintf("[DEBUG] Key released: keycode=0x%04X (%s mode)\n",
                keycode, usb_mode ? "USB" : "PS/2");
    }

    return true;
}

bool led_update_kb(led_t led_state) {
    // In PS/2 mode, block LED updates from USB
    // This prevents USB from sending any HID reports
    if (!usb_mode) {
        return false;  // Don't process LED updates in PS/2 mode
    }
    return led_update_user(led_state);
}

void matrix_init_kb(void) {
    matrix_init_user();
}

void matrix_scan_kb(void) {
    matrix_scan_user();
}
