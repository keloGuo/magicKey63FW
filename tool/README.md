# Tools

This directory contains development and device utility scripts.

## Firmware And Web Tools

- `build_single_html.py`: Builds the web settings UI into one compact
  `web/webServer/out/index.html` file.
- `upload_html.py`: Uploads the generated settings page to the device LittleFS
  through the HTTP update API.
- `flash_uf2_wsl.sh`: Reboots the RP2040 into UF2 mode and copies
  `build/firmware.uf2` to the Windows `RPI-RP2` drive from WSL2.

Common commands:

```sh
python3 tool/build_single_html.py
python3 tool/upload_html.py web/webServer/out/index.html
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

## Codex Status Sync

- `codex_app_server_hook.py`: Reads Codex App Server state and usage data, then
  posts status updates to the keyboard.
- `codex_status.sh`: Manually posts one status value to the device.
- `ucodex`: Starts Codex with an App Server and a status watcher so keyboard
  LEDs and UI can reflect Codex working state.
- `build_ucodex_deb.sh`: Packages `ucodex` and the status hook as a Debian
  package under `dist/`.

Common commands:

```sh
./tool/codex_status.sh active
./tool/ucodex
./tool/build_ucodex_deb.sh
```

## Environment

Useful variables:

- `DEVICE_URL`: Device HTTP base URL. Default is `http://10.63.27.1:80`.
- `BOOTLOADER_URL`: Device bootloader API URL used by flashing scripts. Default
  is `http://10.63.27.1/api/rebootToUf2`.
- `WINDOWS_DRIVE`: Windows drive letter for the mounted `RPI-RP2` volume, for
  example `E:\`.
- `TIMEOUT_SECONDS`: Flash script wait timeout.
- `UF2_PATH`: Override the firmware UF2 path used by `flash_uf2_wsl.sh`.
- `CODEX_BIN`: Codex executable used by `ucodex`.

Run scripts from the repository root unless a script documents otherwise.

If the keyboard network page is set to another IP preset or a custom IP, pass
the matching URL explicitly:

```sh
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

## License

Development and device utility scripts in this directory are licensed under
GPL-2.0-only. See `tool/LICENSE`.
