# PS/2 USB Dual-Mode Keyboard

A QMK-based keyboard firmware that supports both USB and PS/2 protocols on an RP2040 microcontroller. Switch between modes with a hardware toggle!

![License](https://img.shields.io/badge/license-GPL--2.0-blue.svg)
![QMK](https://img.shields.io/badge/QMK-firmware-blue.svg)
![Platform](https://img.shields.io/badge/platform-RP2040-green.svg)

## Features

- 🔌 **Dual Protocol Support**: USB HID and PS/2 device modes
- 🔄 **Hardware Mode Switch**: Toggle between USB and PS/2 with a physical switch
- ⌨️ **Full PS/2 Implementation**: 
  - Make/Break scan codes (Set 2)
  - Typematic repeat (auto-repeat when key held)
  - Proper timing and idle state handling
- 🎮 **RP2040 Powered**: Built for Raspberry Pi Pico and compatible boards
- 📦 **QMK Framework**: Easy to extend and customize

## Hardware Requirements

- **Microcontroller**: RP2040 board (Raspberry Pi Pico, Waveshare RP2040-Zero, etc.)
- **PS/2 Connector**: 6-pin Mini-DIN female connector
- **Mode Switch**: SPST switch or jumper
- **Button**: Single momentary switch (demo uses 'A' key)
- **Level Shifter** (recommended): Bidirectional logic level converter or transistor-based shifter

### ⚠️ Important: Voltage Level Considerations

**The RP2040 is a 3.3V device, while PS/2 protocol typically operates at 5V.**

- **RP2040 GPIO**: 3.3V logic levels
- **PS/2 Standard**: 5V logic levels

**Options:**

1. **Direct Connection (3.3V PS/2)** - Works with many modern PS/2 hosts
   - RP2040 outputs 3.3V, which is often sufficient for PS/2 receivers
   - RP2040 inputs are 5V tolerant when configured as inputs with pull-ups
   - This is what the demo uses, and it works reliably in most cases
   - ⚠️ Some older PS/2 hosts may not reliably detect 3.3V logic high

2. **Bidirectional Level Shifter** (recommended for production use)
   - Use a 3.3V ↔ 5V bidirectional level shifter (e.g., TXS0102, BSS138-based)
   - Ensures proper voltage levels in both directions
   - Necessary for guaranteed compatibility with all PS/2 hosts
   - Protects RP2040 from potential 5V signals on misconfigured hosts

3. **Simple Transistor Level Shifter**
   - 2x N-channel MOSFETs (BSS138 or similar) for clock and data
   - 4x 10kΩ pull-up resistors
   - Cost-effective DIY solution

**For this demo project**, direct connection works fine with modern computers. For production keyboards intended for wide compatibility, use a proper level shifter.

### Pin Configuration

| Function | GPIO Pin | Description |
|----------|----------|-------------|
| Button | GP15 | Single key input (direct pin) |
| PS/2 Clock | GP16 | PS/2 clock line (bidirectional) |
| PS/2 Data | GP17 | PS/2 data line (bidirectional) |
| Mode Switch | GP14 | HIGH = USB mode, LOW = PS/2 mode |

### PS/2 Connector Pinout

```
  ___
 /   \
| 6 5 |    1: Data
| 4 3 |    2: Not connected
 \ 2 1/    3: Ground
  ---      4: VCC (+5V)
           5: Clock
           6: Not connected
```

**Wiring:**
- Pin 1 (Data) → GP17
- Pin 3 (GND) → GND
- Pin 4 (VCC) → 5V (VBUS)
- Pin 5 (Clock) → GP16

## Software Setup

### Prerequisites

1. [QMK Firmware](https://docs.qmk.fm/#/newbs_getting_started) installed
2. QMK CLI configured
3. ARM GCC toolchain

### Installation

1. **Clone this repository into your QMK keyboards directory:**

```bash
cd ~/qmk_firmware/keyboards
mkdir -p bjl  # Create the bjl directory first
cd bjl
git clone https://github.com/BuzzL123/qmk-single-button-ps2-usb-dual-mode-keyboard-demo.git ps2demo
```

Or if you want to keep your QMK directory clean, use **sparse checkout** to only get the keyboard files:

```bash
cd ~/qmk_firmware/keyboards
mkdir -p bjl/ps2demo  # Create both directories
cd bjl/ps2demo
git init
git remote add origin https://github.com/BuzzL123/qmk-single-button-ps2-usb-dual-mode-keyboard-demo.git
git config core.sparseCheckout true
echo "*.c" >> .git/info/sparse-checkout
echo "*.h" >> .git/info/sparse-checkout
echo "*.json" >> .git/info/sparse-checkout
echo "*.mk" >> .git/info/sparse-checkout
git pull origin main
```

This will only download the firmware files (`.c`, `.h`, `.json`, `.mk`) and skip documentation/extras.

2. **Compile the firmware:**

```bash
qmk compile -kb bjl/ps2demo -km default
```

3. **Flash to your RP2040:**

```bash
# Put your board into bootloader mode (double-tap reset or hold BOOTSEL while plugging in)
qmk flash -kb bjl/ps2demo -km default
```

Alternatively, manually copy the `.uf2` file to the RPI-RP2 drive that appears.

## Usage

### USB Mode

1. Set the mode switch to HIGH (or disconnect from GND)
2. Connect via USB
3. The keyboard will enumerate as a standard USB HID device
4. Press the button to send 'A' key

### PS/2 Mode

1. Set the mode switch to LOW (connect to GND)
2. Connect the PS/2 cable to your computer
3. Power the RP2040 via USB (for power only) or external 5V source
4. Press the button to send PS/2 scan codes:
   - Make code: `0x1C` (key down)
   - Break code: `0xF0 0x1C` (key up)
5. Hold the button for typematic repeat (auto-repeat after 500ms)

## Project Structure

```
bjl/ps2demo/
├── keymaps/
│   └── default/
│       └── keymap.c  # Keymap definition (single 'A' key)
├── config.h          # Pin definitions and configuration
├── info.json         # QMK keyboard metadata and USB IDs
├── keyboard.c        # Main keyboard logic and mode switching
├── keyboard.h        # Keyboard header and layout definitions
├── ps2_device.c      # PS/2 protocol implementation
├── ps2_device.h      # PS/2 protocol header
├── rules.mk          # Build configuration
├── ps2_decoder.py    # Testing tool for second Pico
├── README.md         # This file
├── QUICKSTART.md     # Quick start guide
└── LICENSE           # GPL-2.0 license
```

## PS/2 Protocol Implementation

This firmware implements PS/2 Scan Code Set 2 as a **device** (keyboard), including:

### Transmission Format

Each byte sent over PS/2 consists of 11 bits:
```
[Start] [D0] [D1] [D2] [D3] [D4] [D5] [D6] [D7] [Parity] [Stop]
   0     LSB                              MSB    Odd      1
```

### Timing Characteristics

- Clock frequency: ~12.5 kHz (40μs half-period)
- Inter-byte delay: 2ms
- Idle state: Both clock and data HIGH

### Key Features

- **Make Codes**: Sent when key is pressed
- **Break Codes**: Two-byte sequence (`0xF0` + scan code) sent when key is released
- **Typematic Repeat**: 
  - Delay: 500ms before repeat starts
  - Rate: ~30 repeats per second (33ms interval)

## Debugging

### Serial Debug Output

The firmware includes debug output via USB serial (CONSOLE feature enabled):

```bash
# Monitor serial output
qmk console
```

Example output:
```
================================
Mode: PS/2
================================
[PS2] Initializing PS/2 device...
[PS2] PS/2 device initialized (CLK=GP16, DATA=GP17)
[KEY] keycode=4, pressed=1, mode=PS/2
[PS2] Sending PS/2 make code for A (0x1C)...
[PS2] Typematic repeat armed
```

### Testing with Python

To verify PS/2 output, use the included `ps2_decoder.py` script on a second Raspberry Pi Pico:

1. Upload `ps2_decoder.py` to a second Pico using Thonny or mpremote
2. Wire GP16 (Clock) and GP17 (Data) to your keyboard's PS/2 lines
3. Run the script and press keys

The decoder will show:
```
✓ KEY DOWN: A
  Break prefix (0xF0)
✓ KEY UP:   A
```

See [QUICKSTART.md](QUICKSTART.md#for-testingdebugging) for detailed instructions.

## Customization

### Adding More Keys

Edit `keymaps/default/keymap.c` and `info.json` to add more keys:

```c
// keymap.c
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_1x3(
        KC_A, KC_B, KC_C
    )
};
```

Then add scan code mappings in `keyboard.c`:

```c
case KC_B:
    if (record->event.pressed) {
        ps2_device_send_key_make(0x32);  // B key scan code
    } else {
        ps2_device_send_key_break(0x32);
    }
    return false;
```

### Adjusting Typematic Rate

Modify the typematic settings in `keyboard.c`:

```c
typematic.repeat_delay = 250;  // Start repeat after 250ms
typematic.repeat_rate = 50;    // 50ms between repeats (20 Hz)
```

Or call at runtime:
```c
ps2_set_typematic_rate(250, 50);
```

## Technical Details

### Why PS/2 Device Mode?

Most PS/2 projects implement **host** mode (reading from a PS/2 keyboard). This project implements **device** mode, making the RP2040 *act as* a PS/2 keyboard. This is much rarer and more complex!

### Challenges Solved

1. **Timing Precision**: PS/2 requires precise timing (~40μs per half clock cycle)
2. **Idle State Management**: Lines must be properly released between transmissions
3. **Inter-byte Delays**: Receiver needs time to process each byte
4. **Typematic Repeat**: Firmware must handle key repeat timing

### Future Enhancements

- [ ] Host-to-device command handling (LED updates, scan code set switching)
- [ ] Full keyboard matrix support
- [ ] PS/2 host mode (for connecting PS/2 keyboards to USB)
- [ ] Bidirectional PS/2 communication
- [ ] Support for extended scan codes (multimedia keys)

## Troubleshooting

### Keyboard not recognized in PS/2 mode

- Check wiring (especially clock and data)
- Verify 5V power to PS/2 port
- Ensure mode switch is set to LOW (PS/2 mode)
- Check debug console for initialization messages
- **Voltage level issues**: If your PS/2 host doesn't recognize the keyboard, it may not accept 3.3V logic levels. Try using a bidirectional level shifter (3.3V ↔ 5V)

### Keys not registering

- Monitor serial debug output
- Verify button is connected to correct GPIO
- Check that firmware is in correct mode
- If in PS/2 mode, ensure the host computer recognizes the device first

### Typematic repeat not working

- Ensure you're holding the key for > 500ms
- Check debug output for "Typematic repeat" messages
- Verify `ps2_device_task()` is being called regularly

### Intermittent PS/2 behavior

- This is often a voltage level issue
- Add a bidirectional level shifter between RP2040 and PS/2 connector
- Ensure good connections (no loose wires)
- Check that pull-up resistors are properly configured

## License

This project is licensed under the GPL-2.0 License - see the LICENSE file for details.

This license matches QMK Firmware's GPL-2.0 license for maximum compatibility.

## Credits

- **Author**: Betzalel J. Lewis
- **Framework**: [QMK Firmware](https://qmk.fm/)
- **Hardware**: Raspberry Pi RP2040

## References

- [PS/2 Protocol Specification](https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf)
- [QMK Documentation](https://docs.qmk.fm/)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

**Note**: This is a demonstration project showing PS/2 device implementation. For production use, you would want to add full keyboard matrix support, host command handling, and more robust error handling.
