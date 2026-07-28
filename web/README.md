# Web

This directory contains the embedded web settings UI used by the device HTTP
server.

## Layout

- `webServer/`: Main settings UI source files.
  - `index.html`: HTML structure for the settings page.
  - `index.css`: Page styles.
  - `main.js`: Keyboard layout, key binding, macro editing, upload, and device
    API logic.
  - `out/index.html`: Generated single-file page uploaded to LittleFS on the
    device.
- `webUpdte/`: Minimal update/upload page kept separately from the main
  settings UI.

## Build Single HTML

The device serves a single compressed HTML file from LittleFS. After editing
`webServer/index.html`, `webServer/index.css`, or `webServer/main.js`, rebuild
the generated page from the repository root:

```sh
python3 tool/build_single_html.py
```

The output is:

```text
web/webServer/out/index.html
```

Do not edit `webServer/out/index.html` directly. It is generated output, but it
is intentionally tracked so a release can include and review the exact page that
will be uploaded to the device.

To check whether the tracked output matches the current source files, rebuild
the page and then inspect the diff:

```sh
python3 tool/build_single_html.py
git diff -- web/webServer/out/index.html
```

For CI or release checks, use `git diff --exit-code -- web/webServer/out/index.html`
after rebuilding.

## Upload To Device

After rebuilding the single HTML file, upload it to the device:

```sh
python3 tool/upload_html.py web/webServer/out/index.html
```

The default device URL is `http://10.63.27.1`. Override it with
`--device-url` or `DEVICE_URL` when the keyboard network page is set to another
IP preset or a custom IP.

Examples:

```sh
python3 tool/upload_html.py --device-url http://172.23.63.1:80
DEVICE_URL=http://172.23.63.1:80 python3 tool/upload_html.py
```

## LittleFS Page And Packed Update Page

The firmware has two web page paths:

- Main settings page: generated from `webServer/index.html`,
  `webServer/index.css`, and `webServer/main.js`, then uploaded to LittleFS as
  `/index.html`.
- Packed update page: generated from `webUpdte/index.html` and embedded into
  the firmware image as `/web_root/update/index.html`.

The normal configuration UI should come from LittleFS. The packed update page is
kept in firmware as a recovery path and can be opened with:

```text
http://<device-ip>/update
```

If the LittleFS page is missing, damaged, or a page upload fails before the main
UI is usable again, use the packed update page or rerun `tool/upload_html.py` to
upload a fresh `web/webServer/out/index.html`.

## Notes

- Keep source edits in `index.html`, `index.css`, and `main.js`.
- The embedded web page uses relative `/api/...` paths, so it follows the
  current device IP instead of a hard-coded address.
- The firmware-side HTTP API handlers are under `firmware/rndis/`.

## License

Web UI source files and generated embedded page output are licensed under
GPL-2.0-only. See `web/LICENSE`.
