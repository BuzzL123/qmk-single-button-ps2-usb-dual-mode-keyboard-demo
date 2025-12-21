# MCU name
MCU = RP2040
BOOTLOADER = rp2040

# Build Options
#   Do not enable features in both rules.mk and info.json
#   Features are already defined in info.json
#   Only define build-specific settings here

# Custom source files for PS/2 device implementation
SRC += ps2_keyboard.c \
       ps2_mouse.c \
       kb.c

# Compiler optimization
OPT_DEFS += -O2

# If you need to override any info.json features, do it here
# But it's better to keep everything in info.json for consistency
