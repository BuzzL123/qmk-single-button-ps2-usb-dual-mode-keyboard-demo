// keyboards/bjl/ps2demo/ps2_scancodes.h
// PS/2 Scan Code Set 2 - Complete mapping table with lookup
#ifndef PS2_SCANCODES_H
#define PS2_SCANCODES_H

#include <stdint.h>
#include "quantum.h"  // For QMK keycodes

// Forward declare the mapping type (defined in ps2_keyboard.h)
typedef enum {
    PS2_KEY_NORMAL,
    PS2_KEY_PRINTSCREEN,
    PS2_KEY_PAUSE
} ps2_special_key_type_t;

typedef struct {
    uint8_t scancode;
    bool needs_e0_prefix;
    ps2_special_key_type_t special_type;
} ps2_mapping_t;

// PS/2 Scan Code Set 2 - Standard Keys
// Make codes only (break = 0xF0 + make code)
#define PS2_A             0x1C
#define PS2_B             0x32
#define PS2_C             0x21
#define PS2_D             0x23
#define PS2_E             0x24
#define PS2_F             0x2B
#define PS2_G             0x34
#define PS2_H             0x33
#define PS2_I             0x43
#define PS2_J             0x3B
#define PS2_K             0x42
#define PS2_L             0x4B
#define PS2_M             0x3A
#define PS2_N             0x31
#define PS2_O             0x44
#define PS2_P             0x4D
#define PS2_Q             0x15
#define PS2_R             0x2D
#define PS2_S             0x1B
#define PS2_T             0x2C
#define PS2_U             0x3C
#define PS2_V             0x2A
#define PS2_W             0x1D
#define PS2_X             0x22
#define PS2_Y             0x35
#define PS2_Z             0x1A

// Numbers
#define PS2_1             0x16
#define PS2_2             0x1E
#define PS2_3             0x26
#define PS2_4             0x25
#define PS2_5             0x2E
#define PS2_6             0x36
#define PS2_7             0x3D
#define PS2_8             0x3E
#define PS2_9             0x46
#define PS2_0             0x45

// Function keys
#define PS2_F1            0x05
#define PS2_F2            0x06
#define PS2_F3            0x04
#define PS2_F4            0x0C
#define PS2_F5            0x03
#define PS2_F6            0x0B
#define PS2_F7            0x83
#define PS2_F8            0x0A
#define PS2_F9            0x01
#define PS2_F10           0x09
#define PS2_F11           0x78
#define PS2_F12           0x07
#define PS2_F13           0x08
#define PS2_F14           0x10
#define PS2_F15           0x18
#define PS2_F16           0x20
#define PS2_F17           0x28
#define PS2_F18           0x30
#define PS2_F19           0x38
#define PS2_F20           0x40
#define PS2_F21           0x48
#define PS2_F22           0x50
#define PS2_F23           0x57
#define PS2_F24           0x5F

// Special characters
#define PS2_GRAVE         0x0E
#define PS2_MINUS         0x4E
#define PS2_EQUAL         0x55
#define PS2_BACKSLASH     0x5D
#define PS2_LBRACKET      0x54
#define PS2_RBRACKET      0x5B
#define PS2_SEMICOLON     0x4C
#define PS2_QUOTE         0x52
#define PS2_COMMA         0x41
#define PS2_DOT           0x49
#define PS2_SLASH         0x4A

// Control keys
#define PS2_ESC           0x76
#define PS2_BACKSPACE     0x66
#define PS2_TAB           0x0D
#define PS2_CAPS          0x58
#define PS2_LSHIFT        0x12
#define PS2_LCTRL         0x14
#define PS2_LALT          0x11
#define PS2_SPACE         0x29
#define PS2_RALT          0x11
#define PS2_RCTRL         0x14
#define PS2_RSHIFT        0x59
#define PS2_ENTER         0x5A
#define PS2_MENU          0x2F

// Navigation cluster
#define PS2_INSERT        0x70
#define PS2_HOME          0x6C
#define PS2_PGUP          0x7D
#define PS2_DELETE        0x71
#define PS2_END           0x69
#define PS2_PGDN          0x7A

// Arrow keys
#define PS2_UP            0x75
#define PS2_DOWN          0x72
#define PS2_LEFT          0x6B
#define PS2_RIGHT         0x74

// Keypad
#define PS2_NUMLOCK       0x77
#define PS2_KP_SLASH      0x4A
#define PS2_KP_ASTERISK   0x7C
#define PS2_KP_MINUS      0x7B
#define PS2_KP_PLUS       0x79
#define PS2_KP_ENTER      0x5A
#define PS2_KP_DOT        0x71
#define PS2_KP_0          0x70
#define PS2_KP_1          0x69
#define PS2_KP_2          0x72
#define PS2_KP_3          0x7A
#define PS2_KP_4          0x6B
#define PS2_KP_5          0x73
#define PS2_KP_6          0x74
#define PS2_KP_7          0x6C
#define PS2_KP_8          0x75
#define PS2_KP_9          0x7D

// Lock keys
#define PS2_SCROLL        0x7E
#define PS2_PSCREEN       0x7C
#define PS2_PAUSE         0x77

// Windows/GUI keys
#define PS2_LGUI          0x1F
#define PS2_RGUI          0x27

// Multimedia keys
#define PS2_MUTE          0x23
#define PS2_VOLUMEDOWN    0x21
#define PS2_VOLUMEUP      0x32
#define PS2_MEDIA_NEXT    0x4D
#define PS2_MEDIA_PREV    0x15
#define PS2_MEDIA_STOP    0x3B
#define PS2_MEDIA_PLAY    0x34
#define PS2_MEDIA_SELECT  0x50

// Browser keys
#define PS2_WWW_SEARCH    0x10
#define PS2_WWW_HOME      0x3A
#define PS2_WWW_BACK      0x38
#define PS2_WWW_FORWARD   0x30
#define PS2_WWW_STOP      0x28
#define PS2_WWW_REFRESH   0x20
#define PS2_WWW_FAVORITES 0x18

// Application keys
#define PS2_APP_MAIL      0x48
#define PS2_APP_CALC      0x2B
#define PS2_APP_MYCOMP    0x40

// ACPI Power keys
#define PS2_POWER         0x37
#define PS2_SLEEP         0x3F
#define PS2_WAKE          0x5E

// International keys
#define PS2_INTL1         0x51
#define PS2_INTL2         0x13
#define PS2_INTL3         0x6A
#define PS2_INTL4         0x64
#define PS2_INTL5         0x67
#define PS2_INTL6         0x13
#define PS2_LANG1         0xF2
#define PS2_LANG2         0xF1
#define PS2_LANG3         0x63
#define PS2_LANG4         0x64
#define PS2_LANG5         0x67

// Special prefix codes
#define PS2_PREFIX_E0   0xE0
#define PS2_PREFIX_E1   0xE1
#define PS2_PREFIX_F0   0xF0

// =============================================================================
// LOOKUP TABLES
// =============================================================================

// Main lookup table for basic keycodes (0x00-0xFF)
static const ps2_mapping_t ps2_scancode_lookup[] = {
    // Letters (0x04-0x1D)
    [KC_A] = {PS2_A, false, PS2_KEY_NORMAL},
    [KC_B] = {PS2_B, false, PS2_KEY_NORMAL},
    [KC_C] = {PS2_C, false, PS2_KEY_NORMAL},
    [KC_D] = {PS2_D, false, PS2_KEY_NORMAL},
    [KC_E] = {PS2_E, false, PS2_KEY_NORMAL},
    [KC_F] = {PS2_F, false, PS2_KEY_NORMAL},
    [KC_G] = {PS2_G, false, PS2_KEY_NORMAL},
    [KC_H] = {PS2_H, false, PS2_KEY_NORMAL},
    [KC_I] = {PS2_I, false, PS2_KEY_NORMAL},
    [KC_J] = {PS2_J, false, PS2_KEY_NORMAL},
    [KC_K] = {PS2_K, false, PS2_KEY_NORMAL},
    [KC_L] = {PS2_L, false, PS2_KEY_NORMAL},
    [KC_M] = {PS2_M, false, PS2_KEY_NORMAL},
    [KC_N] = {PS2_N, false, PS2_KEY_NORMAL},
    [KC_O] = {PS2_O, false, PS2_KEY_NORMAL},
    [KC_P] = {PS2_P, false, PS2_KEY_NORMAL},
    [KC_Q] = {PS2_Q, false, PS2_KEY_NORMAL},
    [KC_R] = {PS2_R, false, PS2_KEY_NORMAL},
    [KC_S] = {PS2_S, false, PS2_KEY_NORMAL},
    [KC_T] = {PS2_T, false, PS2_KEY_NORMAL},
    [KC_U] = {PS2_U, false, PS2_KEY_NORMAL},
    [KC_V] = {PS2_V, false, PS2_KEY_NORMAL},
    [KC_W] = {PS2_W, false, PS2_KEY_NORMAL},
    [KC_X] = {PS2_X, false, PS2_KEY_NORMAL},
    [KC_Y] = {PS2_Y, false, PS2_KEY_NORMAL},
    [KC_Z] = {PS2_Z, false, PS2_KEY_NORMAL},

    // Numbers (0x1E-0x27)
    [KC_1] = {PS2_1, false, PS2_KEY_NORMAL},
    [KC_2] = {PS2_2, false, PS2_KEY_NORMAL},
    [KC_3] = {PS2_3, false, PS2_KEY_NORMAL},
    [KC_4] = {PS2_4, false, PS2_KEY_NORMAL},
    [KC_5] = {PS2_5, false, PS2_KEY_NORMAL},
    [KC_6] = {PS2_6, false, PS2_KEY_NORMAL},
    [KC_7] = {PS2_7, false, PS2_KEY_NORMAL},
    [KC_8] = {PS2_8, false, PS2_KEY_NORMAL},
    [KC_9] = {PS2_9, false, PS2_KEY_NORMAL},
    [KC_0] = {PS2_0, false, PS2_KEY_NORMAL},

    // Special characters (0x28-0x38)
    [KC_ENTER]     = {PS2_ENTER, false, PS2_KEY_NORMAL},
    [KC_ESCAPE]    = {PS2_ESC, false, PS2_KEY_NORMAL},
    [KC_BSPC]      = {PS2_BACKSPACE, false, PS2_KEY_NORMAL},
    [KC_TAB]       = {PS2_TAB, false, PS2_KEY_NORMAL},
    [KC_SPACE]     = {PS2_SPACE, false, PS2_KEY_NORMAL},
    [KC_MINUS]     = {PS2_MINUS, false, PS2_KEY_NORMAL},
    [KC_EQUAL]     = {PS2_EQUAL, false, PS2_KEY_NORMAL},
    [KC_LBRC]      = {PS2_LBRACKET, false, PS2_KEY_NORMAL},
    [KC_RBRC]      = {PS2_RBRACKET, false, PS2_KEY_NORMAL},
    [KC_BSLS]      = {PS2_BACKSLASH, false, PS2_KEY_NORMAL},
    [KC_SCLN]      = {PS2_SEMICOLON, false, PS2_KEY_NORMAL},
    [KC_QUOTE]     = {PS2_QUOTE, false, PS2_KEY_NORMAL},
    [KC_GRAVE]     = {PS2_GRAVE, false, PS2_KEY_NORMAL},
    [KC_COMMA]     = {PS2_COMMA, false, PS2_KEY_NORMAL},
    [KC_DOT]       = {PS2_DOT, false, PS2_KEY_NORMAL},
    [KC_SLASH]     = {PS2_SLASH, false, PS2_KEY_NORMAL},
    [KC_CAPS]      = {PS2_CAPS, false, PS2_KEY_NORMAL},

    // Function keys (0x3A-0x45)
    [KC_F1]  = {PS2_F1, false, PS2_KEY_NORMAL},
    [KC_F2]  = {PS2_F2, false, PS2_KEY_NORMAL},
    [KC_F3]  = {PS2_F3, false, PS2_KEY_NORMAL},
    [KC_F4]  = {PS2_F4, false, PS2_KEY_NORMAL},
    [KC_F5]  = {PS2_F5, false, PS2_KEY_NORMAL},
    [KC_F6]  = {PS2_F6, false, PS2_KEY_NORMAL},
    [KC_F7]  = {PS2_F7, false, PS2_KEY_NORMAL},
    [KC_F8]  = {PS2_F8, false, PS2_KEY_NORMAL},
    [KC_F9]  = {PS2_F9, false, PS2_KEY_NORMAL},
    [KC_F10] = {PS2_F10, false, PS2_KEY_NORMAL},
    [KC_F11] = {PS2_F11, false, PS2_KEY_NORMAL},
    [KC_F12] = {PS2_F12, false, PS2_KEY_NORMAL},

    // Special keys (0x46-0x52)
    [KC_PSCR]       = {PS2_PSCREEN, false, PS2_KEY_PRINTSCREEN},
    [KC_SCRL]       = {PS2_SCROLL, false, PS2_KEY_NORMAL},
    [KC_PAUSE]      = {PS2_PAUSE, false, PS2_KEY_PAUSE},
    [KC_INSERT]     = {PS2_INSERT, true, PS2_KEY_NORMAL},
    [KC_HOME]       = {PS2_HOME, true, PS2_KEY_NORMAL},
    [KC_PGUP]       = {PS2_PGUP, true, PS2_KEY_NORMAL},
    [KC_DELETE]     = {PS2_DELETE, true, PS2_KEY_NORMAL},
    [KC_END]        = {PS2_END, true, PS2_KEY_NORMAL},
    [KC_PGDN]     = {PS2_PGDN, true, PS2_KEY_NORMAL},
    [KC_RIGHT]      = {PS2_RIGHT, true, PS2_KEY_NORMAL},
    [KC_LEFT]       = {PS2_LEFT, true, PS2_KEY_NORMAL},
    [KC_DOWN]       = {PS2_DOWN, true, PS2_KEY_NORMAL},
    [KC_UP]         = {PS2_UP, true, PS2_KEY_NORMAL},

    // Keypad (0x53-0x63)
    [KC_NUM]         = {PS2_NUMLOCK, false, PS2_KEY_NORMAL},
    [KC_KP_SLASH]    = {PS2_KP_SLASH, true, PS2_KEY_NORMAL},
    [KC_KP_ASTERISK] = {PS2_KP_ASTERISK, false, PS2_KEY_NORMAL},
    [KC_KP_MINUS]    = {PS2_KP_MINUS, false, PS2_KEY_NORMAL},
    [KC_KP_PLUS]     = {PS2_KP_PLUS, false, PS2_KEY_NORMAL},
    [KC_KP_ENTER]    = {PS2_KP_ENTER, true, PS2_KEY_NORMAL},
    [KC_KP_1]        = {PS2_KP_1, false, PS2_KEY_NORMAL},
    [KC_KP_2]        = {PS2_KP_2, false, PS2_KEY_NORMAL},
    [KC_KP_3]        = {PS2_KP_3, false, PS2_KEY_NORMAL},
    [KC_KP_4]        = {PS2_KP_4, false, PS2_KEY_NORMAL},
    [KC_KP_5]        = {PS2_KP_5, false, PS2_KEY_NORMAL},
    [KC_KP_6]        = {PS2_KP_6, false, PS2_KEY_NORMAL},
    [KC_KP_7]        = {PS2_KP_7, false, PS2_KEY_NORMAL},
    [KC_KP_8]        = {PS2_KP_8, false, PS2_KEY_NORMAL},
    [KC_KP_9]        = {PS2_KP_9, false, PS2_KEY_NORMAL},
    [KC_KP_0]        = {PS2_KP_0, false, PS2_KEY_NORMAL},
    [KC_KP_DOT]      = {PS2_KP_DOT, false, PS2_KEY_NORMAL},

    // Modifiers (0xE0-0xE7)
    [KC_LCTL]   = {PS2_LCTRL, false, PS2_KEY_NORMAL},
    [KC_LSFT]   = {PS2_LSHIFT, false, PS2_KEY_NORMAL},
    [KC_LALT]   = {PS2_LALT, false, PS2_KEY_NORMAL},
    [KC_LGUI]   = {PS2_LGUI, true, PS2_KEY_NORMAL},
    [KC_RCTL]   = {PS2_RCTRL, true, PS2_KEY_NORMAL},
    [KC_RSFT]   = {PS2_RSHIFT, false, PS2_KEY_NORMAL},
    [KC_RALT]   = {PS2_RALT, true, PS2_KEY_NORMAL},
    [KC_RGUI]   = {PS2_RGUI, true, PS2_KEY_NORMAL},

    // Application/Menu key (0x65)
    [KC_APPLICATION] = {PS2_MENU, true, PS2_KEY_NORMAL},

    // International keys (0x87-0x91)
    [KC_INT1] = {PS2_INTL1, false, PS2_KEY_NORMAL},
    [KC_INT2] = {PS2_INTL2, false, PS2_KEY_NORMAL},
    [KC_INT3] = {PS2_INTL3, false, PS2_KEY_NORMAL},
    [KC_INT4] = {PS2_INTL4, false, PS2_KEY_NORMAL},
    [KC_INT5] = {PS2_INTL5, false, PS2_KEY_NORMAL},
    [KC_INT6] = {PS2_INTL6, false, PS2_KEY_NORMAL},
    [KC_LNG1] = {PS2_LANG1, false, PS2_KEY_NORMAL},
    [KC_LNG2] = {PS2_LANG2, false, PS2_KEY_NORMAL},
    [KC_LNG3] = {PS2_LANG3, false, PS2_KEY_NORMAL},
    [KC_LNG4] = {PS2_LANG4, false, PS2_KEY_NORMAL},
    [KC_LNG5] = {PS2_LANG5, false, PS2_KEY_NORMAL},
};

// Extended keys table (for keycodes > 0xFF)
static const struct {
    uint16_t qmk_keycode;
    ps2_mapping_t mapping;
} ps2_extended_keys[] = {
    // System keys
    {KC_SYSTEM_POWER, {PS2_POWER, true, PS2_KEY_NORMAL}},
    {KC_SYSTEM_SLEEP, {PS2_SLEEP, true, PS2_KEY_NORMAL}},
    {KC_SYSTEM_WAKE,  {PS2_WAKE, true, PS2_KEY_NORMAL}},

    // Media keys
    {KC_AUDIO_MUTE,        {PS2_MUTE, true, PS2_KEY_NORMAL}},
    {KC_AUDIO_VOL_UP,      {PS2_VOLUMEUP, true, PS2_KEY_NORMAL}},
    {KC_AUDIO_VOL_DOWN,    {PS2_VOLUMEDOWN, true, PS2_KEY_NORMAL}},
    {KC_MEDIA_NEXT_TRACK,  {PS2_MEDIA_NEXT, true, PS2_KEY_NORMAL}},
    {KC_MEDIA_PREV_TRACK,  {PS2_MEDIA_PREV, true, PS2_KEY_NORMAL}},
    {KC_MEDIA_STOP,        {PS2_MEDIA_STOP, true, PS2_KEY_NORMAL}},
    {KC_MEDIA_PLAY_PAUSE,  {PS2_MEDIA_PLAY, true, PS2_KEY_NORMAL}},
    {KC_MEDIA_SELECT,      {PS2_MEDIA_SELECT, true, PS2_KEY_NORMAL}},

    // Browser keys
    {KC_WWW_SEARCH,    {PS2_WWW_SEARCH, true, PS2_KEY_NORMAL}},
    {KC_WWW_HOME,      {PS2_WWW_HOME, true, PS2_KEY_NORMAL}},
    {KC_WWW_BACK,      {PS2_WWW_BACK, true, PS2_KEY_NORMAL}},
    {KC_WWW_FORWARD,   {PS2_WWW_FORWARD, true, PS2_KEY_NORMAL}},
    {KC_WWW_STOP,      {PS2_WWW_STOP, true, PS2_KEY_NORMAL}},
    {KC_WWW_REFRESH,   {PS2_WWW_REFRESH, true, PS2_KEY_NORMAL}},
    {KC_WWW_FAVORITES, {PS2_WWW_FAVORITES, true, PS2_KEY_NORMAL}},

    // Application keys
    {KC_MAIL,        {PS2_APP_MAIL, true, PS2_KEY_NORMAL}},
    {KC_CALCULATOR,  {PS2_APP_CALC, true, PS2_KEY_NORMAL}},
    {KC_MY_COMPUTER, {PS2_APP_MYCOMP, true, PS2_KEY_NORMAL}},

    // F13-F24
    {KC_F13, {PS2_F13, false, PS2_KEY_NORMAL}},
    {KC_F14, {PS2_F14, false, PS2_KEY_NORMAL}},
    {KC_F15, {PS2_F15, false, PS2_KEY_NORMAL}},
    {KC_F16, {PS2_F16, false, PS2_KEY_NORMAL}},
    {KC_F17, {PS2_F17, false, PS2_KEY_NORMAL}},
    {KC_F18, {PS2_F18, false, PS2_KEY_NORMAL}},
    {KC_F19, {PS2_F19, false, PS2_KEY_NORMAL}},
    {KC_F20, {PS2_F20, false, PS2_KEY_NORMAL}},
    {KC_F21, {PS2_F21, false, PS2_KEY_NORMAL}},
    {KC_F22, {PS2_F22, false, PS2_KEY_NORMAL}},
    {KC_F23, {PS2_F23, false, PS2_KEY_NORMAL}},
    {KC_F24, {PS2_F24, false, PS2_KEY_NORMAL}},
};

// CONSUMER CONTROL USAGE CODES TO PS/2 MAPPING
// These are the actual USB HID Consumer Control usage codes
// that come through in report_extra_t
static const struct {
    uint16_t usage_code;  // USB HID Consumer Control usage
    ps2_mapping_t mapping;
} ps2_consumer_mappings[] = {
    // Volume and Media Control (0x00E2, 0x00E9, 0x00EA, etc.)
    {0x00E2, {PS2_MUTE, true, PS2_KEY_NORMAL}},        // Mute
    {0x00E9, {PS2_VOLUMEUP, true, PS2_KEY_NORMAL}},    // Volume Up
    {0x00EA, {PS2_VOLUMEDOWN, true, PS2_KEY_NORMAL}},  // Volume Down
    {0x00B5, {PS2_MEDIA_NEXT, true, PS2_KEY_NORMAL}},  // Scan Next Track
    {0x00B6, {PS2_MEDIA_PREV, true, PS2_KEY_NORMAL}},  // Scan Previous Track
    {0x00B7, {PS2_MEDIA_STOP, true, PS2_KEY_NORMAL}},  // Stop
    {0x00CD, {PS2_MEDIA_PLAY, true, PS2_KEY_NORMAL}},  // Play/Pause
    {0x0183, {PS2_MEDIA_SELECT, true, PS2_KEY_NORMAL}},// Media Select

    // Browser Controls (0x0221-0x0227)
    {0x0221, {PS2_WWW_SEARCH, true, PS2_KEY_NORMAL}},    // WWW Search
    {0x0223, {PS2_WWW_HOME, true, PS2_KEY_NORMAL}},      // WWW Home
    {0x0224, {PS2_WWW_BACK, true, PS2_KEY_NORMAL}},      // WWW Back
    {0x0225, {PS2_WWW_FORWARD, true, PS2_KEY_NORMAL}},   // WWW Forward
    {0x0226, {PS2_WWW_STOP, true, PS2_KEY_NORMAL}},      // WWW Stop
    {0x0227, {PS2_WWW_REFRESH, true, PS2_KEY_NORMAL}},   // WWW Refresh
    {0x022A, {PS2_WWW_FAVORITES, true, PS2_KEY_NORMAL}}, // WWW Favorites

    // Application Launch (0x018A, 0x0192, 0x0194)
    {0x018A, {PS2_APP_MAIL, true, PS2_KEY_NORMAL}},      // Email Reader
    {0x0192, {PS2_APP_CALC, true, PS2_KEY_NORMAL}},      // Calculator
    {0x0194, {PS2_APP_MYCOMP, true, PS2_KEY_NORMAL}},    // My Computer
};

// Size definitions for lookup tables
#define PS2_SCANCODE_LOOKUP_SIZE (sizeof(ps2_scancode_lookup) / sizeof(ps2_scancode_lookup[0]))
#define PS2_EXTENDED_KEYS_SIZE (sizeof(ps2_extended_keys) / sizeof(ps2_extended_keys[0]))
#define PS2_CONSUMER_MAPPINGS_SIZE (sizeof(ps2_consumer_mappings) / sizeof(ps2_consumer_mappings[0]))

#endif // PS2_SCANCODES_H
