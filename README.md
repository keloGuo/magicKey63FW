# MagicKey63

MagicKey63 is an RP2040 based USB keyboard firmware project. The firmware
includes keyboard matrix scanning, USB HID/RNDIS, an embedded web settings
interface, LittleFS storage, LVGL screen UI, WS2812 lighting, macro storage and
macro playback.

## Repository Layout

- `firmware/`: RP2040 Pico SDK firmware source code and firmware-side third
  party libraries.
- `web/`: Web settings UI source files and generated single-file output.
- `tool/`: Development, upload, flashing, packaging, and Codex status sync
  scripts.
- `case_3d/`: Keyboard enclosure and mechanical structure 3D files.
- `hardware/`: Schematics and hardware manufacturing source/export files.
- `tool/dist/`: Generated tool package artifacts.
- `build/`: Local CMake build output.

## Build

The root `CMakeLists.txt` is kept as the build entry point. Set
`PICO_SDK_PATH` to a local Pico SDK checkout, then build from the repository
root:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build
cmake --build build
```

The generated UF2 is available at:

```text
build/firmware.uf2
```

For release builds, use the project build script:

```sh
tool/build_firmware.sh --release --incremental -j 8
tool/build_firmware.sh --release --clean -j 8
tool/build_firmware.sh --debug --incremental -j 8
```

The script records the Pico SDK path and git commit, runs
`arm-none-eabi-size`, and copies release artifacts to:

```text
build/release/<BuildType>/firmware.uf2
build/release/<BuildType>/firmware.elf
build/release/<BuildType>/firmware.map
build/release/<BuildType>/firmware.size.txt
build/release/<BuildType>/build-info.txt
```

## Flash

When the device is connected to the Windows host through WSL2, use:

```sh
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

Adjust `WINDOWS_DRIVE` if the `RPI-RP2` bootloader volume uses a different
drive letter.

## Device Network

The default USB network address is:

```text
http://10.63.27.1:80
```

The device UI can switch to alternate IP presets or a custom IP. When the
device IP is changed, pass the matching URL to tools through `DEVICE_URL` or
`BOOTLOADER_URL`, for example:

```sh
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

## License

This repository uses separate licenses for different parts of the project. See
`LICENSE` for the repository-level summary.

- Firmware, web UI, and development tools: GPL-2.0-only.
- Hardware electronics design files: CERN-OHL-S-2.0.
- Case and mechanical 3D files: CC-BY-SA-4.0.

Third party libraries keep their own license notices. This includes Mongoose
(`GPL-2.0-only or commercial`), LVGL (`MIT`), littlefs (`BSD-3-Clause`), cJSON
(`MIT`), TinyUSB (`MIT`), and Pico SDK components.

See `THIRD_PARTY_LICENSES.md` for the redistribution checklist and third party
license summary.
