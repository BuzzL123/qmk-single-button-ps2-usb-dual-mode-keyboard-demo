""" PS/2 Protocol Decoder for Testing (Fixed and Enhanced)
=============================================
Use this script on a second Raspberry Pi Pico to monitor and decode
PS/2 signals from your keyboard device.

Key improvements:
- Better clock edge detection
- Shows raw hex bytes for debugging
- Proper extended key sequence handling
- Works with device-to-host communication

Wiring:
  - Connect GP16 (Clock) to your keyboard's PS/2 Clock line
  - Connect GP17 (Data) to your keyboard's PS/2 Data line
  - Connect GND to common ground

Author: Betzalel J. Lewis (Enhanced)
License: GPL-3.0
"""

from machine import Pin
import time

# Pin Configuration
PS2_CLOCK_PIN = 16
PS2_DATA_PIN = 17

# Initialize pins with pull-ups
clk_pin = Pin(PS2_CLOCK_PIN, Pin.IN, Pin.PULL_UP)
data_pin = Pin(PS2_DATA_PIN, Pin.IN, Pin.PULL_UP)

print("=" * 60)
print("PS/2 Keyboard Decoder (Enhanced for Device Testing)")
print("=" * 60)
print("Monitoring PS/2 signals on GP16 (CLK) and GP17 (DATA)")
print("Press Ctrl+C to stop")
print()

# State tracking
expecting_break = False
expecting_extended = False
byte_sequence = []  # Track multi-byte sequences

def read_ps2_byte_improved():
    """
    Read one byte from PS/2 using clock-driven sampling.
    This works for device-to-host communication.
    
    Returns:
        tuple: (byte_value, is_valid)
    """
    
    # Wait for clock to go LOW (start of bit transmission)
    # Increased timeout for device-to-host
    timeout = 0
    while clk_pin.value() == 1:
        time.sleep_us(5)
        timeout += 1
        if timeout > 200000:  # 1 second timeout
            return None, False
    
    bits = []
    
    # Read start bit on falling edge
    start_bit = data_pin.value()
    
    # Wait for clock to go high
    while clk_pin.value() == 0:
        time.sleep_us(5)
    
    # Read 8 data bits on clock falling edges
    for bit_num in range(8):
        # Wait for clock low
        while clk_pin.value() == 1:
            time.sleep_us(5)
        
        bit_value = data_pin.value()
        bits.append(bit_value)
        
        # Wait for clock high
        while clk_pin.value() == 0:
            time.sleep_us(5)
    
    # Read parity bit
    while clk_pin.value() == 1:
        time.sleep_us(5)
    parity_bit = data_pin.value()
    while clk_pin.value() == 0:
        time.sleep_us(5)
    
    # Read stop bit
    while clk_pin.value() == 1:
        time.sleep_us(5)
    stop_bit = data_pin.value()
    
    # Reconstruct byte (LSB first)
    byte_value = 0
    for i in range(8):
        if bits[i]:
            byte_value |= (1 << i)
    
    # Validate frame
    valid = (start_bit == 0) and (stop_bit == 1)
    
    return byte_value, valid

# PS/2 Scan Code Set 2 - Comprehensive lookup table
SCAN_CODES = {
    # Letters
    0x1C: 'A', 0x32: 'B', 0x21: 'C', 0x23: 'D', 0x24: 'E',
    0x2B: 'F', 0x34: 'G', 0x33: 'H', 0x43: 'I', 0x3B: 'J',
    0x42: 'K', 0x4B: 'L', 0x3A: 'M', 0x31: 'N', 0x44: 'O',
    0x4D: 'P', 0x15: 'Q', 0x2D: 'R', 0x1B: 'S', 0x2C: 'T',
    0x3C: 'U', 0x2A: 'V', 0x1D: 'W', 0x22: 'X', 0x35: 'Y',
    0x1A: 'Z',
    
    # Numbers
    0x45: '0', 0x16: '1', 0x1E: '2', 0x26: '3', 0x25: '4',
    0x2E: '5', 0x36: '6', 0x3D: '7', 0x3E: '8', 0x46: '9',
    
    # Function keys
    0x05: 'F1', 0x06: 'F2', 0x04: 'F3', 0x0C: 'F4', 0x03: 'F5',
    0x0B: 'F6', 0x83: 'F7', 0x0A: 'F8', 0x01: 'F9', 0x09: 'F10',
    0x78: 'F11', 0x07: 'F12',
    
    # Control keys
    0x5A: 'ENTER', 0x76: 'ESC', 0x66: 'BACKSPACE', 0x0D: 'TAB',
    0x29: 'SPACE', 0x0E: 'GRAVE', 0x4E: 'MINUS', 0x55: 'EQUAL',
    0x54: 'LBRACKET', 0x5B: 'RBRACKET', 0x5D: 'BSLASH',
    0x4C: 'SEMICOLON', 0x52: 'QUOTE', 0x41: 'COMMA',
    0x49: 'DOT', 0x4A: 'SLASH', 0x58: 'CAPSLOCK',
    
    # Modifiers
    0x12: 'LSHIFT', 0x59: 'RSHIFT', 0x14: 'LCTRL', 0x11: 'LALT',
    
    # Keypad
    0x70: 'KP_0', 0x69: 'KP_1', 0x72: 'KP_2', 0x7A: 'KP_3',
    0x6B: 'KP_4', 0x73: 'KP_5', 0x74: 'KP_6', 0x6C: 'KP_7',
    0x75: 'KP_8', 0x7D: 'KP_9', 0x7C: 'KP_ASTERISK',
    0x7B: 'KP_MINUS', 0x79: 'KP_PLUS', 0x71: 'KP_DOT',
    0x77: 'NUMLOCK', 0x7E: 'SCROLLLOCK',
}

# Extended scan codes (prefixed with 0xE0)
EXTENDED_SCAN_CODES = {
    # Navigation cluster
    0x70: 'INSERT', 0x6C: 'HOME', 0x7D: 'PGUP',
    0x71: 'DELETE', 0x69: 'END', 0x7A: 'PGDN',
    
    # Arrow keys
    0x75: 'UP', 0x72: 'DOWN', 0x6B: 'LEFT', 0x74: 'RIGHT',
    
    # Modifiers
    0x14: 'RCTRL', 0x11: 'RALT',
    0x1F: 'LGUI', 0x27: 'RGUI', 0x2F: 'APP',
    
    # Keypad
    0x5A: 'KP_ENTER', 0x4A: 'KP_SLASH',
    
    # ACPI
    0x37: 'POWER', 0x3F: 'SLEEP', 0x5E: 'WAKE',
    
    # *** MEDIA KEYS - FIXED MAPPINGS ***
    0x10: 'WWW_SEARCH',
    0x18: 'WWW_FAVORITES', 
    0x20: 'WWW_REFRESH',
    0x21: 'VOLUME_DOWN',      # Fixed!
    0x23: 'MUTE',             # Fixed!
    0x28: 'WWW_STOP',
    0x2B: 'CALCULATOR',
    0x30: 'WWW_FORWARD',
    0x32: 'VOLUME_UP',        # Fixed!
    0x34: 'PLAY_PAUSE',
    0x38: 'WWW_BACK',
    0x3A: 'WWW_HOME',
    0x3B: 'MEDIA_STOP',
    0x40: 'MY_COMPUTER',
    0x48: 'EMAIL',
    0x4D: 'NEXT_TRACK',
    0x15: 'PREV_TRACK',
    0x50: 'MEDIA_SELECT',
    
    # Print screen helper
    0x12: 'PRTSC_PART',
    0x7C: 'PRTSC',
}

def decode_scan_code(scan_code, is_extended):
    """Decode a scan code to a key name"""
    if is_extended:
        key_name = EXTENDED_SCAN_CODES.get(scan_code, None)
        if key_name:
            return f"{key_name}"
        else:
            return f"UNKNOWN_E0_{scan_code:02X}"
    else:
        key_name = SCAN_CODES.get(scan_code, None)
        if key_name:
            return key_name
        else:
            return f"UNKNOWN_{scan_code:02X}"

def format_byte_sequence(seq):
    """Format a sequence of bytes for display"""
    return " ".join(f"{b:02X}" for b in seq)

print("Waiting for keyboard input...")
print("Shows: RAW HEX | KEY NAME | MAKE/BREAK")
print()

try:
    while True:
        scan_code, valid = read_ps2_byte_improved()
        
        if scan_code is None:
            continue
            
        if not valid:
            print(f"⚠ INVALID FRAME: {scan_code:02X}")
            continue
        
        # Add to sequence tracker
        byte_sequence.append(scan_code)
        
        # Show raw byte in real-time
        if scan_code == 0xE0:
            print(f"    E0        <- Extended prefix")
            expecting_extended = True
            
        elif scan_code == 0xE1:
            print(f"    E1        <- Pause/Break prefix")
            byte_sequence = [0xE1]  # Start new sequence
            
        elif scan_code == 0xF0:
            print(f"    F0        <- Break prefix")
            expecting_break = True
            
        else:
            # Actual key scan code
            key_name = decode_scan_code(scan_code, expecting_extended)
            
            # Build the full sequence string for display
            seq_str = format_byte_sequence(byte_sequence)
            
            if expecting_break:
                print(f"✓   {seq_str:12s} = KEY UP:   {key_name}")
                expecting_break = False
                expecting_extended = False
                byte_sequence = []
            else:
                print(f"✓   {seq_str:12s} = KEY DOWN: {key_name}")
                expecting_extended = False
                byte_sequence = []
        
        # Reset sequence if it gets too long (error recovery)
        if len(byte_sequence) > 8:
            print("    [Sequence too long - resetting]")
            byte_sequence = []
            expecting_extended = False
            expecting_break = False

except KeyboardInterrupt:
    print("\n" + "=" * 60)
    print("Decoder stopped")
    print("=" * 60)