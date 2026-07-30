# MagicKey63 Hardware

This directory contains the electronics design files for the current
MagicKey63 prototype revision.

## Available Files

- `ProPrj_magicKey.epro2` - Editable JLCEDA Pro project archive containing
  the schematic and PCB layout sources for the current prototype.
- `SCH_magicKeySCH.pdf` - Main keyboard schematic, including the RP2040,
  key matrix, RGB LEDs, power circuitry, encoders, and peripheral interfaces.
- `SCH_magicKeyLcd.pdf` - Display interconnect board schematic.
- `SCH_magicKeyUSB.pdf` - USB-C connector board schematic.
- `BOM_magicKey.xlsx` - Preliminary electronics BOM covering the main,
  display, and USB-C boards.

The `.epro2` archive can be imported into the free JLCEDA Pro editor.

## Manufacturing Status

The editable PCB sources, schematic PDFs, and preliminary electronics BOM
are available now. Verified Gerber and drill files, pick-and-place data,
assembly drawings, and the production-qualified BOM will be added after the
pilot revision is frozen.

Keyboard enclosure and mechanical 3D files are stored separately in
`case_3d/`.

## License

Hardware electronics design files in this directory are licensed under
CERN-OHL-S-2.0. See `hardware/LICENSE`.
