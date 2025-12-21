// keyboards/your_keyboard/config.h
#pragma once

// RP2040 Bootloader Configuration
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET // Activates the double-tap behavior
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U // Timeout window in ms in which the double tap can occur.
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP25 // Specify an optional status led by GPIO number which blinks when entering the bootloader

#define NO_USB_STARTUP_CHECK  true  // Don't wait for USB connection

// PS/2 Keyboard Pin definitions
#define PS2_KEYBOARD_CLOCK_PIN  GP16
#define PS2_KEYBOARD_DATA_PIN   GP17

// PS/2 Mouse Pin definitions (future)
#define PS2_MOUSE_CLOCK_PIN     GP18
#define PS2_MOUSE_DATA_PIN      GP19

// Mode switch pin (to toggle between USB and PS/2)
#define MODE_SWITCH_PIN GP14  // High = USB, Low = PS/2

// Debounce reduces chatter (can also be set in info.json)
#define DEBOUNCE 5

// Note: USB IDs, matrix configuration, and processor info
// are now defined in info.json instead of here

