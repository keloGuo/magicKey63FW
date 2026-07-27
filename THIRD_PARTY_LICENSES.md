# Third Party Licenses

This document summarizes third party software used by MagicKey63. It is a
release checklist and redistribution notice companion for source packages,
firmware binaries, and tool packages.

This summary does not replace the original license notices. If a third party
file contains its own copyright header, SPDX identifier, or license text, that
notice applies to that file and must be preserved.

## Summary

| Component | Location | License | Notes |
| --- | --- | --- | --- |
| Mongoose | `firmware/rndis/mongoose.c`, `firmware/rndis/mongoose.h` | GPL-2.0-only or commercial | Bundled in firmware source. The GPL-2.0-only option is used for this GPL-2.0-only firmware project. A commercial Mongoose license is needed for non-GPL proprietary redistribution. |
| LVGL | `firmware/lib/lvgl/` | MIT | Bundled in firmware source. Original license text is kept at `firmware/lib/lvgl/LICENCE.txt`. |
| littlefs | `firmware/lib/littlefs/` | BSD-3-Clause | Bundled in firmware source. Source files keep SPDX headers. |
| cJSON | `firmware/lib/cJson/` | MIT | Bundled in firmware source. Source and header files keep the MIT license header. |
| TinyUSB | Pico SDK dependency, usually `${PICO_SDK_PATH}/lib/tinyusb/` | MIT | Not vendored in this repository. Used through the Raspberry Pi Pico SDK at build time. |
| Raspberry Pi Pico SDK | External build dependency, `${PICO_SDK_PATH}` | BSD-3-Clause | Not vendored in this repository. Firmware builds link Pico SDK libraries and headers. |

## Bundled Components

### Mongoose

Files:

- `firmware/rndis/mongoose.c`
- `firmware/rndis/mongoose.h`

License:

- `GPL-2.0-only or commercial`

The bundled Mongoose header states that Mongoose is dual licensed. This project
uses the GPL-2.0-only option, matching the firmware license. Keep the original
Mongoose copyright and license header in redistributed source.

If MagicKey63 firmware is redistributed outside the GPL-2.0-only terms, review
Mongoose's commercial licensing requirements first.

### LVGL

Directory:

- `firmware/lib/lvgl/`

License:

- `MIT`

Original license file:

- `firmware/lib/lvgl/LICENCE.txt`

Keep LVGL's license file and any file-level notices when redistributing source.

### littlefs

Directory:

- `firmware/lib/littlefs/`

License:

- `BSD-3-Clause`

The littlefs source files include SPDX license headers such as:

```text
SPDX-License-Identifier: BSD-3-Clause
```

Keep these headers when redistributing source.

### cJSON

Directory:

- `firmware/lib/cJson/`

License:

- `MIT`

The cJSON source and header files include the MIT license text. Keep those
headers when redistributing source.

## External Build Dependencies

### TinyUSB

Typical local SDK path:

- `${PICO_SDK_PATH}/lib/tinyusb/`

License:

- `MIT`

TinyUSB is used through the Pico SDK build. It is not vendored in this
repository. When distributing binary firmware, include TinyUSB's MIT license
notice in the release license bundle.

### Raspberry Pi Pico SDK

Typical local SDK path:

- `${PICO_SDK_PATH}`

License:

- `BSD-3-Clause`

The Pico SDK is an external build dependency and is not vendored in this
repository. When distributing binary firmware, include the Pico SDK BSD-3-Clause
license notice in the release license bundle.

The Pico SDK also contains optional third party submodules, such as BTstack,
cyw43-driver, lwIP, mbedTLS, and TinyUSB. MagicKey63 should include notices for
the Pico SDK components actually linked into a release build.

## Redistribution Checklist

For source releases:

- [ ] Include this `THIRD_PARTY_LICENSES.md` file.
- [ ] Keep bundled third party source directories with their original file
      headers and license notices.
- [ ] Keep `firmware/lib/lvgl/LICENCE.txt`.
- [ ] Keep cJSON and littlefs source headers intact.
- [ ] Keep Mongoose copyright and license headers intact.

For firmware binary releases:

- [ ] Include this `THIRD_PARTY_LICENSES.md` file.
- [ ] Include the repository license summary in `LICENSE`.
- [ ] Include Mongoose GPL-2.0-only/commercial notice.
- [ ] Include LVGL MIT notice.
- [ ] Include littlefs BSD-3-Clause notice.
- [ ] Include cJSON MIT notice.
- [ ] Include TinyUSB MIT notice.
- [ ] Include Pico SDK BSD-3-Clause notice.
- [ ] Record the Pico SDK version or commit used for the build.

For tool package releases:

- [ ] Include `tool/LICENSE`.
- [ ] Include this third party license summary if the package contains firmware,
      generated web assets, or bundled third party source.
