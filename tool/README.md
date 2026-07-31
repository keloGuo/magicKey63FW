# Tools

This directory contains development and device utility scripts.

## Firmware And Web Tools

- `build_firmware.sh`: Configures and builds firmware release/debug artifacts,
  records Pico SDK version information, runs `arm-none-eabi-size`, and copies
  UF2/ELF/map outputs under `build/release/<BuildType>/`.
- `check_ci.sh`: Runs the local CI-style checks: whitespace, generated web page,
  old IP hard-coding, VID/PID release notes, CMake configure, firmware build,
  and firmware size output.
- `build_single_html.py`: Builds the web settings UI source files into one
  compact `web/webServer/out/index.html` file. Re-run it after changing
  `web/webServer/index.html`, `web/webServer/index.css`, or
  `web/webServer/main.js`.
- `pack_upload_page.py`: Converts `web/webUpdte/index.html` into generated C
  array source/header files for the firmware embedded upload page. This packed
  page is the recovery UI served from `/update`. CMake runs this automatically
  when the upload page changes.
- `upload_html.py`: Uploads the generated settings page to the device LittleFS
  through the HTTP update API. The device URL can be supplied with
  `--device-url` or the `DEVICE_URL` environment variable.
- `flash_uf2_wsl.sh`: Reboots the RP2040 into UF2 mode and copies
  `build/firmware.uf2` to the Windows `RPI-RP2` drive from WSL2.

Common commands:

```sh
tool/check_ci.sh
tool/build_firmware.sh --release --incremental -j 8
python3 tool/build_single_html.py
git diff --exit-code -- web/webServer/out/index.html
python3 tool/upload_html.py web/webServer/out/index.html
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

## Codex Status Sync

- `codex_app_server_hook.py`: Reads Codex App Server state and usage data, then
  posts status updates to the keyboard.
- `codex_status.sh`: Manually posts one status value to the device.
- `device_discover.sh`: Finds the keyboard HTTP URL for Codex status sync.
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

- `DEVICE_URL`: Device HTTP base URL. If unset, Codex status tools discover the
  keyboard and cache the result in `~/.ucodex/device_url`.
- `DEVICE_URL_CANDIDATES`: Space-separated URLs scanned by
  `device_discover.sh`. Default includes `10.63.27.1`, `10.63.27.2`, and
  `192.168.3.1`.
- `BOOTLOADER_URL`: Device bootloader API URL used by flashing scripts. Default
  is `http://10.63.27.1/api/rebootToUf2`.
- `WINDOWS_DRIVE`: Windows drive letter for the mounted `RPI-RP2` volume, for
  example `E:\`.
- `TIMEOUT_SECONDS`: Flash script wait timeout.
- `UF2_PATH`: Override the firmware UF2 path used by `flash_uf2_wsl.sh`.
- `BUILD_TYPE`: Firmware build type used by `build_firmware.sh`.
- `JOBS`: Parallel build jobs used by `build_firmware.sh`.
- `SIZE_TOOL`: Size tool used by `build_firmware.sh`. Default is
  `arm-none-eabi-size`.
- `FORBIDDEN_IP_PATTERN`: Extended regexp used by `check_ci.sh` to detect old
  hard-coded device IPs. Default is `192\.168\.3\.1`.
- `STRICT_RELEASE`: Set to `1` to make `check_ci.sh` fail on release blockers
  such as temporary VID/PID notes.
- `CODEX_BIN`: Codex executable used by `ucodex`.

Run scripts from the repository root unless a script documents otherwise.

If the keyboard network page is set to another IP preset or a custom IP, pass
the matching URL explicitly:

```sh
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

## Web Update Recovery

The main settings page is uploaded to LittleFS as `index.html`. The smaller
`web/webUpdte/index.html` page is embedded in the firmware and is served from
`/update`, so it remains available even when the LittleFS settings page needs to
be replaced.

Recovery workflow:

```sh
python3 tool/build_single_html.py
DEVICE_URL=http://<device-ip>:80 python3 tool/upload_html.py web/webServer/out/index.html
```

If the browser cannot load the main page, open `http://<device-ip>/update` and
use the packed update page to restore the LittleFS page.

## License

Development and device utility scripts in this directory are licensed under
GPL-2.0-only. See `tool/LICENSE`.
