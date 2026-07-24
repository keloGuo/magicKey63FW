# Firmware

This directory contains the RP2040 Pico SDK firmware source code for
MagicKey63.

## Build Entry

Build from the repository root, not from this directory:

```sh
cmake -S . -B build
cmake --build build
```

The root CMake file imports the Pico SDK and then adds this directory with
`add_subdirectory(firmware)`.

## Directory Layout

- `main.c`: Firmware entry point and system startup flow.
- `magic63_config.h`: Shared hardware and firmware configuration.
- `keyboardScan/`: PIO/DMA keyboard matrix scanning and bounce diagnostic
  support.
- `PIO/`: Standalone PIO programs used by firmware features.
- `usb/`: TinyUSB HID descriptors, reports, and USB device logic.
- `rndis/`: RNDIS network interface, HTTP server, and web API handlers.
- `dataSave/`: Persistent settings storage.
- `macroRecorder/`: Macro recording, storage, and playback support.
- `UI/`: LVGL screen pages, page switching, and UI data handling.
- `lvglPort/`: LVGL display port for the ST7735 screen.
- `st7735/`: ST7735 display driver.
- `ws2812/`: WS2812 LED driver and PIO program.
- `keymap/`: Key value and keymap related logic.
- `apm/`: APM counting.
- `boot/`: Reboot and UF2 bootloader entry helpers.
- `debug/`: Optional debug output and status logging helpers.
- `scratch/`: Shared scratch memory support.
- `lib/`: Firmware-side third party libraries, including LVGL, LittleFS, and
  cJSON.
- `zCmakeFile/`: Pico SDK import helper.

## Output

CMake generates the firmware executable and UF2 under `build/`. A compatibility
copy is kept at:

```text
build/firmware.uf2
```

This keeps existing flashing scripts working after the firmware source was moved
under `firmware/`.
