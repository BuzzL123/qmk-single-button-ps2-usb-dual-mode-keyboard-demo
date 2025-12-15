#!/usr/bin/env python3
"""
PS/2 Protocol Decoder for Testing
==================================
Use this script on a second Raspberry Pi Pico to monitor and decode
PS/2 signals from your keyboard.

Wiring:
  - Connect GP16 (Clock) to your keyboard's PS/2 Clock line
  - Connect GP17 (Data) to your keyboard's PS/2 Data line
  - Connect GND to common ground

Usage:
  1. Upload this script to a second Pico using Thonny or similar
  2. Connect the PS/2 lines as described above
  3. Run the script and press keys on your PS/2 keyboard
  4. You should see make/break codes displayed

Author: Betzalel J. Lewis
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

print("=" * 50)
print("PS/2 Keyboard Decoder (with Auto-Resync)")
print("=" * 50)
print("Monitoring PS/2 signals on GP16 (CLK) and GP17 (DATA)")
print("Press Ctrl+C to stop")
print()

expecting_break = False

def wait_for_idle():
    """Wait until both clock and data are high (idle state)"""
    timeout = 0
    while timeout < 10000:
        if clk_pin.value() == 1 and data_pin.value() == 1:
            return True
        time.sleep_us(10)
        timeout += 1
    return False

def read_ps2_byte():
    """
    Read one byte from PS/2 keyboard with full validation.
    
    Returns:
        tuple: (byte_value, is_valid) where is_valid indicates if
               start and stop bits were correct
    """
    
    # Wait for clock to go low (start of transmission)
    timeout = 0
    while clk_pin.value() == 1:
        time.sleep_us(10)
        timeout += 1
        if timeout > 100000:  # 1 second timeout
            return None, False
    
    # Read start bit (should be 0)
    start_bit = data_pin.value()
    while clk_pin.value() == 0:
        time.sleep_us(10)
    
    # Read 8 data bits (LSB first)
    byte_value = 0
    for bit_num in range(8):
        while clk_pin.value() == 1:
            time.sleep_us(10)
        if data_pin.value():
            byte_value |= (1 << bit_num)
        while clk_pin.value() == 0:
            time.sleep_us(10)
    
    # Read parity bit (we check it but don't fail on it)
    while clk_pin.value() == 1:
        time.sleep_us(10)
    parity_bit = data_pin.value()
    while clk_pin.value() == 0:
        time.sleep_us(10)
    
    # Read stop bit (should be 1)
    while clk_pin.value() == 1:
        time.sleep_us(10)
    stop_bit = data_pin.value()
    while clk_pin.value() == 0:
        time.sleep_us(10)
    
    # Validate frame
    valid = (start_bit == 0) and (stop_bit == 1)
    
    return byte_value, valid

# PS/2 Scan Code Set 2 lookup table (partial)
SCAN_CODES = {
    0x1C: 'A', 0x32: 'B', 0x21: 'C', 0x23: 'D', 0x24: 'E',
    0x2B: 'F', 0x34: 'G', 0x33: 'H', 0x43: 'I', 0x3B: 'J',
    0x42: 'K', 0x4B: 'L', 0x3A: 'M', 0x31: 'N', 0x44: 'O',
    0x4D: 'P', 0x15: 'Q', 0x2D: 'R', 0x1B: 'S', 0x2C: 'T',
    0x3C: 'U', 0x2A: 'V', 0x1D: 'W', 0x22: 'X', 0x35: 'Y',
    0x1A: 'Z', 0x45: '0', 0x16: '1', 0x1E: '2', 0x26: '3',
    0x25: '4', 0x2E: '5', 0x36: '6', 0x3D: '7', 0x3E: '8',
    0x46: '9', 0x5A: 'ENTER', 0x76: 'ESC', 0x66: 'BACKSPACE',
    0x29: 'SPACE', 0x12: 'L-SHIFT', 0x59: 'R-SHIFT',
    0x14: 'L-CTRL', 0x11: 'L-ALT'
}

print("Waiting for keyboard input...")
print()

try:
    while True:
        scan_code, valid = read_ps2_byte()
        
        if valid:
            if scan_code == 0xF0:
                # Break code prefix
                expecting_break = True
                print("  Break prefix (0xF0)")
            else:
                # Look up the key name
                key_name = SCAN_CODES.get(scan_code, f"Unknown (0x{scan_code:02X})")
                
                if expecting_break:
                    print(f"✓ KEY UP:   {key_name}")
                    expecting_break = False
                else:
                    print(f"✓ KEY DOWN: {key_name}")
        else:
            # Invalid frame detected - resync
            print("⚠ Invalid frame detected, resyncing...")
            if wait_for_idle():
                print("  Resync successful")
            else:
                print("  Resync timeout!")
            time.sleep_ms(10)

except KeyboardInterrupt:
    print("\n" + "=" * 50)
    print("Decoder stopped")
    print("=" * 50)