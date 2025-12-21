// ps2_device.h
#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "ps2_scancodes.h"
#include "host_driver.h"    // For host_driver_t

extern host_driver_t ps2_keyboard_host_driver;  // Declare the PS/2 driver
extern ps2_special_key_type_t ps2_key_type; // Declare modifier mappings
extern ps2_mapping_t qmk_to_ps2_scancode(uint16_t keycode); // Declare mapping function

// PS/2 Commands from host
#define PS2_CMD_SET_LEDS           0xED
#define PS2_CMD_ECHO               0xEE
#define PS2_CMD_SET_SCANCODE_SET   0xF0
#define PS2_CMD_IDENTIFY           0xF2
#define PS2_CMD_SET_TYPEMATIC      0xF3
#define PS2_CMD_ENABLE             0xF4
#define PS2_CMD_DISABLE            0xF5
#define PS2_CMD_SET_DEFAULTS       0xF6
#define PS2_CMD_RESEND             0xFE
#define PS2_CMD_RESET              0xFF

// PS/2 Responses
#define PS2_ACK                    0xFA
#define PS2_RESEND                 0xFE
#define PS2_BAT_SUCCESS            0xAA
#define PS2_BAT_FAIL               0xFC
#define PS2_ECHO_RESPONSE          0xEE

/* NOT IN USE!
// Scancode Sets (imlemented only Set 2 for now)
typedef enum {
    PS2_SCANCODE_SET_1 = 1,
    PS2_SCANCODE_SET_2 = 2,
    PS2_SCANCODE_SET_3 = 3
} ps2_scancode_set_t;

uint8_t ps2_keyboard_get_scancode_set(void);
void ps2_keyboard_set_scancode_set(ps2_scancode_set_t set);
*/

// PS/2 State Machine
typedef enum {
    PS2_STATE_IDLE,
    PS2_STATE_SENDING,
    PS2_STATE_RECEIVING,
    PS2_STATE_WAIT_RESPONSE
} ps2_state_t;

typedef struct {
    uint8_t scroll_lock : 1;
    uint8_t num_lock    : 1;
    uint8_t caps_lock   : 1;
    uint8_t reserved    : 5;
} ps2_led_state_t;

// PS/2 Keyboard Device functions (all renamed)
void ps2_keyboard_init(uint8_t clk_pin, uint8_t data_pin);
void ps2_keyboard_task(void);
bool ps2_keyboard_send_key_make(uint8_t scancode);
bool ps2_keyboard_send_key_break(uint8_t scancode);
ps2_led_state_t ps2_keyboard_get_leds(void);
bool ps2_keyboard_is_enabled(void);

// Typematic functions (renamed)
void ps2_keyboard_typematic_task(void);
void ps2_keyboard_typematic_arm(uint16_t keycode, uint8_t scancode);
void ps2_keyboard_typematic_stop(uint16_t keycode);
void ps2_keyboard_typematic_disable(void);
#endif // PS2_DEVICE_H
