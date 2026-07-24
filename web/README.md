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

## Upload To Device

After rebuilding the single HTML file, upload it to the device:

```sh
python3 tool/upload_html.py web/webServer/out/index.html
```

The default device URL is `http://10.63.27.1`. Override it with
`--device-url` or `DEVICE_URL` when the keyboard network page is set to another
IP preset or a custom IP.

## Notes

- Keep source edits in `index.html`, `index.css`, and `main.js`.
- The embedded web page uses relative `/api/...` paths, so it follows the
  current device IP instead of a hard-coded address.
- Treat `out/index.html` as generated output, but it is tracked so the current
  device page can be reviewed and uploaded without rebuilding.
- The firmware-side HTTP API handlers are under `firmware/rndis/`.

## License

Web UI source files and generated embedded page output are licensed under
GPL-2.0-only. See `web/LICENSE`.
