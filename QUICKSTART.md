# Quick Start Guide

Get your PS/2 USB keyboard up and running in under 10 minutes!

## What You Need

- Raspberry Pi Pico (or any RP2040 board)
- 6-pin Mini-DIN PS/2 female connector
- Single push button
- SPST switch or jumper wire
- Breadboard and jumper wires
- USB cable (micro-USB or USB-C depending on your board)
- **(Optional but recommended)** Bidirectional 3.3V ↔ 5V level shifter

### ⚠️ Voltage Level Warning

The RP2040 operates at **3.3V**, while PS/2 standard is **5V**.

**For this demo:**
- Direct connection (3.3V) works with most modern computers
- The RP2040 inputs are 5V tolerant when configured with pull-ups
- Most PS/2 hosts accept 3.3V as logic high (it's above the ~2.0V threshold)

**For production use:**
- Use a bidirectional level shifter (TXS0102, BSS138-based, etc.)
- Ensures compatibility with all PS/2 hosts
- Provides proper voltage protection

**If you have a level shifter**, wire it between the RP2040 and PS/2 connector:
```
RP2040 (3.3V) ←→ Level Shifter ←→ PS/2 Connector (5V)
```

## Step 1: Wire It Up

### Connections

```
RP2040 Pico          Component
═══════════════      ═════════════════════
GP15         ───────► Button (other side to GND)
GP16         ───────► PS/2 Pin 5 (Clock)
GP17         ───────► PS/2 Pin 1 (Data)
GP14         ───────► Mode Switch (other side to GND)
5V (VBUS)    ───────► PS/2 Pin 4 (VCC)
GND          ───────► PS/2 Pin 3 (GND)
```

### Mode Switch

- **USB Mode**: Leave GP14 unconnected (pulled HIGH internally)
- **PS/2 Mode**: Connect GP14 to GND

💡 **Tip**: Use a physical switch for easy mode switching!

## Step 2: Flash the Firmware

### Option A: Pre-built UF2 (Easiest)

1. Download the latest `.uf2` file from [Releases](../../releases)
2. Hold BOOTSEL button on Pico while plugging in USB
3. Drag and drop the `.uf2` file onto the RPI-RP2 drive
4. Done! The Pico will reboot automatically

### Option B: Build from Source

```bash
# Clone QMK if you haven't already
git clone https://github.com/qmk/qmk_firmware.git
cd qmk_firmware

# Install this keyboard (method 1: full clone)
cd keyboards
mkdir -p bjl  # Create the bjl directory first
cd bjl
git clone https://github.com/BuzzL123/qmk-single-button-ps2-usb-dual-mode-keyboard-demo.git ps2demo
cd ../..

# OR use sparse checkout to only get firmware files (method 2: clean)
cd keyboards
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
cd ../../..

# Compile
qmk compile -kb bjl/ps2demo -km default

# Flash (put Pico in bootloader mode first)
qmk flash -kb bjl/ps2demo -km default
```

## Step 3: Test USB Mode

1. Keep mode switch HIGH (or disconnected)
2. Plug in via USB
3. Open a text editor
4. Press the button → You should see 'A' appear!
5. Hold the button → 'AAAAAAAA...' (auto-repeat)

✅ **USB mode working!**

### 🔍 Debug Output (Optional)

You can monitor real-time debug output in both USB and PS/2 modes:

**Windows/Linux/macOS:**
```bash
# Open QMK MSYS terminal (Windows) or regular terminal (Mac/Linux)
qmk console
```

You'll see detailed output like:
```
================================
Mode: USB
================================
[USB] USB mode selected
[KEY] keycode=4, pressed=1, mode=USB
[USB] USB mode - processing normally
```

This works in **both USB and PS/2 modes** - the debug output always goes over USB even when the keyboard is in PS/2 mode!

## Step 4: Test PS/2 Mode

1. Connect mode switch to GND (PS/2 mode)
2. Connect PS/2 cable to computer
3. Power the Pico via USB (for power)
4. Your computer should detect a PS/2 keyboard
5. Press button → 'A' appears!

### 🔍 Monitor PS/2 Activity with QMK Console

Even in PS/2 mode, you can see what's happening via USB debug output:

```bash
# In QMK MSYS terminal (Windows) or regular terminal
qmk console
```

When you press the 'A' button in PS/2 mode, you'll see:
```
================================
Mode: PS/2
================================
[PS2] Initializing PS/2 device...
[PS2] PS/2 device initialized (CLK=GP16, DATA=GP17)
[KEY] keycode=4, pressed=1, mode=PS/2
[PS2] PS/2 mode active - processing key
[PS2] Sending PS/2 make code for A (0x1C)...
[PS2] Make code queued: success
[PS2] Typematic repeat armed
[PS2] Sending PS/2 break code for A (0xF0 0x1C)...
[PS2] Break code queued: success
[PS2] Typematic repeat stopped
```

This is incredibly useful for debugging! The keyboard sends data via PS/2 while simultaneously logging everything over USB.

### For Testing/Debugging

Use another Pico to monitor PS/2 signals:

1. **Get the decoder script**:
   ```bash
   # Download just the decoder (no need to clone everything)
   curl -O https://raw.githubusercontent.com/BuzzL123/qmk-single-button-ps2-usb-dual-mode-keyboard-demo/main/ps2_decoder.py
   
   # Or with wget
   wget https://raw.githubusercontent.com/BuzzL123/qmk-single-button-ps2-usb-dual-mode-keyboard-demo/main/ps2_decoder.py
   ```

2. **Upload the decoder script** to a second Pico:
   - Use Thonny, mpremote, or your preferred MicroPython tool
   - Upload `ps2_decoder.py`

2. **Wire the second Pico**:
   ```
   Decoder Pico     →     Your PS/2 Keyboard
   ════════════           ══════════════════
   GP16 (Clock)     →     PS/2 Clock line
   GP17 (Data)      →     PS/2 Data line
   GND              →     Common GND
   ```

3. **Run the decoder** and press keys

Expected output when pressing 'A':
```
✓ KEY DOWN: A     ← Key pressed (make code 0x1C)
  Break prefix (0xF0)
✓ KEY UP:   A     ← Key released (break code 0xF0 0x1C)
```

The decoder includes:
- ✅ Auto-resync on invalid frames
- ✅ Start/stop bit validation
- ✅ Scan code lookup table
- ✅ Human-readable key names

## Common Issues

### 🔴 Nothing happens in USB mode

- Check button wiring (GP15 to GND)
- Verify firmware flashed successfully (LED should blink on Pico)
- Try different USB cable

### 🔴 PS/2 not detected

- Verify PS/2 wiring (especially Clock and Data)
- Ensure 5V power connected
- Mode switch must be set to LOW (connected to GND)
- Some computers need PS/2 device connected during boot
- **Try a level shifter** if your computer doesn't detect 3.3V signals reliably

### 🔴 Keys repeat too fast/slow

Edit `keyboard.c` and change:
```c
typematic.repeat_delay = 500;  // Delay before repeat (ms)
typematic.repeat_rate = 33;    // Time between repeats (ms)
```

### 🔴 Getting random characters

- Check PS/2 clock and data aren't swapped
- Ensure proper pull-up resistors (built-in are usually fine)
- Add external 4.7kΩ pull-ups if needed

## Next Steps

- Add more keys to your keyboard
- Customize the keymap
- Add LED indicators
- Implement host command handling

See the full [README.md](README.md) for detailed documentation!

## Need Help?

- Check [Troubleshooting](README.md#troubleshooting) section
- Open an [Issue](../../issues)
- Read the [PS/2 Protocol](README.md#ps2-protocol-implementation) docs

---

Happy typing! ⌨️
