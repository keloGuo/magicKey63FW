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

Release and debug builds are wrapped by `tool/build_firmware.sh`:

```sh
tool/build_firmware.sh --release --incremental -j 8
tool/build_firmware.sh --release --clean -j 8
tool/build_firmware.sh --debug --incremental -j 8
```

The script supports clean and incremental builds, passes
`CMAKE_BUILD_TYPE=Release` or `Debug`, runs the build in parallel, records the
Pico SDK git commit, runs `arm-none-eabi-size`, and copies the UF2, ELF, and map
file into `build/release/<BuildType>/`.

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

## USB Device Identity

RP2040 does not provide a built-in unique chip ID. MagicKey63 uses the external
QSPI flash unique ID through Pico SDK `pico_unique_id`.

- USB string descriptor serial: the full 8-byte flash unique ID encoded as 16
  uppercase hexadecimal characters.
- RNDIS MAC address: generated from the same flash unique ID before USB
  initialization. The first byte is fixed to `0x02`, which makes the address a
  locally administered unicast MAC address. The remaining five bytes are mixed
  from the flash unique ID bytes.
- TinyUSB RNDIS reports and the Mongoose TCP/IP interface use the same generated
  MAC address.
- Device IP address is configurable. When multiple keyboards are connected to
  the same host at the same time, configure each keyboard to use a different IP
  preset or custom IP to avoid HTTP/RNDIS address conflicts.

After changing VID, PID, serial, MAC, or IP settings, verify enumeration on the
target host OS. Windows, macOS, and Linux may cache USB and network interface
state differently.

## Versioning

- Firmware version string: set by `MAGIC63_FIRMWARE_VERSION` in
  `firmware/CMakeLists.txt`. The default development value is `0.1.0-dev`.
- USB `bcdDevice`: defined as `MAGIC63_USB_BCD_DEVICE` in
  `firmware/magic63_version.h`. It uses packed BCD `0xMMmm`, where `MM` is the
  public major version and `mm` is the public minor version. Development builds
  before the first public release use `0x0100`.
- Persistent configuration schema: defined as `MAGIC63_CONFIG_VERSION` in
  `firmware/magic63_version.h`.
- Settings web page version: defined by `MAGIC63_SETTINGS_PAGE_VERSION` in
  `web/webServer/main.js`.
- Packed upload page version: defined by `MAGIC63_UPLOAD_PAGE_VERSION` in
  `web/webUpdte/index.html` and embedded into firmware by
  `tool/pack_upload_page.py`.

The `/api/versionInfo` endpoint exposes the firmware version, configuration
schema version, USB `bcdDevice`, and packed upload page version. The settings
web page combines that response with its own page version.

When the main persistent configuration structure changes, increment
`MAGIC63_CONFIG_VERSION`. The current migration policy is conservative: if the
stored structure size or schema version does not match, firmware restores the
main configuration and keymap to defaults. Network configuration is stored
separately as `netConfigData` and is validated by its own magic value.

## License

Firmware source code in this directory is licensed under GPL-2.0-only. See
`firmware/LICENSE`.

Third party libraries under `firmware/lib/` and other imported files keep their
own license notices.
