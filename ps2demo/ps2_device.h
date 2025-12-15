// ps2_device.h
#ifndef PS2_DEVICE_H
#define PS2_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

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

// Scan Code Set 2 for letter 'A'
#define PS2_SCANCODE_A_MAKE        0x1C
#define PS2_SCANCODE_A_BREAK_PREFIX 0xF0
#define PS2_SCANCODE_A_BREAK       0x1C

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

// Function declarations
void ps2_device_init(uint8_t clk_pin, uint8_t data_pin);
void ps2_device_task(void);
bool ps2_device_send_key(uint8_t scancode);
bool ps2_device_send_key_make(uint8_t scancode);
bool ps2_device_send_key_break(uint8_t scancode);
ps2_led_state_t ps2_device_get_leds(void);
bool ps2_device_is_enabled(void);

#endif // PS2_DEVICE_H
