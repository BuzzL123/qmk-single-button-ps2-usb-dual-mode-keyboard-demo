// keyboards/your_keyboard/keyboard.h
#pragma once

#include "quantum.h"

//// Pin definitions (matching config.h)
// #define PS2_CLOCK_PIN  GP16
// #define PS2_DATA_PIN   GP17
// #define MODE_SWITCH_PIN GP14

// PS/2 Device functions
#include "ps2_keyboard.h"

// Layout macro - defines the physical layout
// This is a single key, so it's very simple
#define KEYMAP( \
    k00 \
) { \
    { k00 } \
}

// For keyboards with direct pin matrix (no diode matrix scanning)
// QMK will handle this automatically with DIRECT_PINS in config.h

// Optional: Add any keyboard-specific functions here
void keyboard_pre_init_kb(void);
void keyboard_post_init_kb(void);
void housekeeping_task_kb(void);
bool process_record_kb(uint16_t keycode, keyrecord_t *record);

// Mode detection
bool is_usb_mode(void);
bool is_ps2_mode(void);
