#!/usr/bin/env python3
"""Generate C source/header files for the embedded upload page."""

from __future__ import annotations

import argparse
from pathlib import Path


def c_bytes(data: bytes) -> str:
    lines = []
    for offset in range(0, len(data), 16):
        chunk = data[offset : offset + 16]
        lines.append(",".join(f"0x{byte:02x}" for byte in chunk) + ",")
    return "\n".join(lines)


def write_if_changed(path: Path, text: str) -> None:
    if path.exists() and path.read_text() == text:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert the upload HTML page to C array source files."
    )
    parser.add_argument("--input", required=True, type=Path, help="Input HTML file")
    parser.add_argument("--output-c", required=True, type=Path, help="Output C file")
    parser.add_argument("--output-h", required=True, type=Path, help="Output header file")
    parser.add_argument(
        "--symbol",
        default="magic63_update_page_html",
        help="C array symbol name",
    )
    args = parser.parse_args()

    data = args.input.read_bytes() + b"\0"
    guard = f"{args.symbol.upper()}_H"
    size_macro = f"{args.symbol.upper()}_SIZE"

    header = f"""#ifndef {guard}
#define {guard}

#include <stddef.h>

#define {size_macro} {len(data)}u

extern const unsigned char {args.symbol}[{size_macro}];

#endif
"""

    source = f"""#include \"{args.output_h.name}\"

const unsigned char {args.symbol}[{size_macro}] =
{{
{c_bytes(data)}
}};
"""

    write_if_changed(args.output_h, header)
    write_if_changed(args.output_c, source)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
