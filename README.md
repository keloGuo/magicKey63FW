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

## Flash

When the device is connected to the Windows host through WSL2, use:

```sh
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

Adjust `WINDOWS_DRIVE` if the `RPI-RP2` bootloader volume uses a different
drive letter.

## License

This project is licensed under GPL-3.0. See `LICENSE`.
