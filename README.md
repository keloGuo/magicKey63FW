# MagicKey63

<p align="right">
  <a href="#english">English</a> |
  <a href="#中文简介">中文</a>
</p>

<!--
Project image placeholder:
Add a product photo or render here before publishing on GitHub/Crowd Supply.
Suggested path: docs/images/magickey63-hero.jpg

![MagicKey63 product photo or render](docs/images/magickey63-hero.jpg)
-->

## English

[中文](#中文简介)

MagicKey63 is an open-source RP2040 interactive hardware platform built into a compact mechanical keyboard. It works as a daily 63-key keyboard, but it is also designed for engineers, makers, and firmware hackers who want to modify and extend their desktop tools.

The project includes RP2040 firmware, USB HID/RNDIS, a browser-based web configuration UI, LittleFS storage, an LVGL onboard screen UI, WS2812 lighting, macro recording/playback, switch bounce diagnostics, hardware files, and mechanical design files.

### Features

- Compact 63-key layout with arrow keys and practical editing keys.
- RP2040 firmware with matrix scanning, USB HID, USB networking, and device-side HTTP APIs.
- Self-hosted web configuration page for keymap editing, macro management, resource upload, and version display.
- The web settings page can be updated independently without reflashing the full firmware.
- LVGL onboard UI with status, layer, lighting, network, and bootloader pages.
- Hidden dual clickable encoders for layer switching, volume, menu control, or custom workflows.
- Device-side macro recording, storage, and playback without host-side macro software.
- PIO/DMA-based key scanning and WS2812 driving to reduce real-time CPU load.
- Switch bounce diagnostics for validating switches, debounce behavior, and scan timing.
- Firmware, web UI, hardware files, and mechanical files are open-sourced under separate licenses.

### Build

Build from the repository root. A local Raspberry Pi Pico SDK checkout is required:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build
cmake --build build
```

The generated UF2 is available at:

```text
build/firmware.uf2
```

For release and debug builds, use the project script:

```sh
tool/build_firmware.sh --release --incremental -j 8
tool/build_firmware.sh --release --clean -j 8
tool/build_firmware.sh --debug --incremental -j 8
```

Local checks:

```sh
tool/check_ci.sh
```

### Flash

When the device is connected to a Windows host through WSL2, use:

```sh
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

If the device IP has been changed, provide the matching bootloader URL:

```sh
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

You can also enter UF2 mode manually and copy `build/firmware.uf2` to the `RPI-RP2` volume.

### Web Configuration

Default USB network address:

```text
http://10.63.27.1:80
```

The device can switch to alternate IP presets or a custom IP. Tool scripts accept `DEVICE_URL` and `BOOTLOADER_URL`:

```sh
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

After editing `web/webServer/index.html`, `index.css`, or `main.js`, regenerate the tracked single-file page:

```sh
python3 tool/build_single_html.py
git diff --exit-code -- web/webServer/out/index.html
```

### License Notes

Different parts of this repository use different licenses. See `LICENSE` and `THIRD_PARTY_LICENSES.md`.

- Firmware, web UI, and development tools: GPL-2.0-only.
- Hardware electronics design files: CERN-OHL-S-2.0.
- Case and mechanical 3D files: CC-BY-SA-4.0.
- Mongoose is licensed as `GPL-2.0-only or commercial`. This firmware project uses the GPL-2.0-only option. Closed-source or non-GPL commercial redistribution requires separate Mongoose commercial licensing.
- LVGL, littlefs, cJSON, TinyUSB, and Pico SDK components keep their own license notices.

### VID/PID Status

The USB VID/PID has been updated to the Raspberry Pi provided assignment: `0x2E8A:0x113A`. The assignment source is documented in `firmware/usb/usb_descriptors.c`, and the firmware no longer uses TinyUSB demo IDs or a temporary development PID.

Before release, run:

```sh
tool/check_ci.sh --strict-release
```

## 中文简介

[English](#english)

MagicKey63 是一个以机械键盘为载体的开源 RP2040 交互硬件平台。它可以作为一把日常使用的紧凑 63 键键盘，也可以作为工程师、创客和固件爱好者继续修改、扩展和实验的桌面设备。

项目包含 RP2040 固件、USB HID/RNDIS、浏览器 Web 配置界面、LittleFS 存储、LVGL 屏幕 UI、WS2812 灯效、宏录制/播放、机械轴抖动检测，以及硬件和结构资料。

### 功能特性

- 紧凑 63 键布局，保留方向键和常用编辑效率。
- RP2040 固件，包含键盘矩阵扫描、USB HID、USB 网络和设备端 HTTP API。
- 自托管 Web 配置页面，可在浏览器里修改键位、管理宏、上传资源和查看版本信息。
- Web 设置页可以独立更新，不必每次修改网页都重新刷写完整固件。
- LVGL 屏幕 UI，可显示状态页、层页、灯光页、网络页、Boot 页等设备端界面。
- 隐藏式双可按压编码器，可用于层切换、音量、菜单或自定义功能。
- 硬件宏录制、存储和播放，不依赖电脑端宏软件。
- PIO/DMA 键盘扫描和 WS2812 驱动，降低主循环实时负担。
- 机械轴抖动检测功能，适合验证轴体、消抖策略和扫描行为。
- 固件、Web UI、硬件资料和结构文件按不同许可证分区开源。

### 快速构建

从仓库根目录构建。需要先准备 Raspberry Pi Pico SDK，并设置 `PICO_SDK_PATH`：

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build
cmake --build build
```

生成的 UF2 文件位于：

```text
build/firmware.uf2
```

发布/调试构建建议使用项目脚本：

```sh
tool/build_firmware.sh --release --incremental -j 8
tool/build_firmware.sh --release --clean -j 8
tool/build_firmware.sh --debug --incremental -j 8
```

本地检查：

```sh
tool/check_ci.sh
```

### 烧录

在 Windows + WSL2 环境中，可以让设备进入 UF2 bootloader 并复制固件：

```sh
WINDOWS_DRIVE='E:\' TIMEOUT_SECONDS=120 ./tool/flash_uf2_wsl.sh
```

如果设备 IP 已切换，需要指定 bootloader URL：

```sh
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

也可以手动让设备进入 UF2 模式，然后将 `build/firmware.uf2` 复制到 `RPI-RP2` 盘符。

### Web 配置

默认 USB 网络地址：

```text
http://10.63.27.1:80
```

设备可在屏幕/网页中切换到其它 IP 预设或自定义 IP。工具脚本支持通过 `DEVICE_URL` / `BOOTLOADER_URL` 指定当前设备地址：

```sh
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
BOOTLOADER_URL=http://172.23.63.1/api/rebootToUf2 ./tool/flash_uf2_wsl.sh
```

修改 `web/webServer/index.html`、`index.css` 或 `main.js` 后，需要重新生成被跟踪的单文件页面：

```sh
python3 tool/build_single_html.py
git diff --exit-code -- web/webServer/out/index.html
```

### License 注意事项

本仓库不同部分使用不同许可证。详见 `LICENSE` 和 `THIRD_PARTY_LICENSES.md`。

- 固件、Web UI、开发工具：GPL-2.0-only。
- 硬件电子设计文件：CERN-OHL-S-2.0。
- 外壳和结构 3D 文件：CC-BY-SA-4.0。
- Mongoose 使用 `GPL-2.0-only or commercial` 许可选项。本项目固件按 GPL-2.0-only 发布；如果未来闭源或非 GPL 商业分发，需要单独处理 Mongoose 商业授权。
- LVGL、littlefs、cJSON、TinyUSB 和 Pico SDK 组件保留各自原始 license notice。

### VID/PID 状态

当前 USB VID/PID 已更新为 Raspberry Pi 提供的 VID/PID：`0x2E8A:0x113A`。`firmware/usb/usb_descriptors.c` 中记录了分配来源，固件不再使用 TinyUSB demo ID 或临时开发 PID。

发布前建议运行：

```sh
tool/check_ci.sh --strict-release
```

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

## Release Artifacts

`tool/build_firmware.sh` records the Pico SDK path and git commit, runs
`arm-none-eabi-size`, and copies release artifacts to:

```text
build/release/<BuildType>/firmware.uf2
build/release/<BuildType>/firmware.elf
build/release/<BuildType>/firmware.map
build/release/<BuildType>/firmware.size.txt
build/release/<BuildType>/build-info.txt
```
