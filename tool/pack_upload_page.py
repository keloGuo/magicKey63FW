#!/usr/bin/env python3
"""Generate C source/header files for the embedded upload page."""

from __future__ import annotations

import argparse
import re
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


def c_string(text: str) -> str:
    return (
        text.replace("\\", "\\\\")
        .replace("\"", "\\\"")
        .replace("\n", "\\n")
        .replace("\r", "\\r")
    )


def extract_upload_page_version(html: str) -> str:
    match = re.search(
        r"\bMAGIC63_UPLOAD_PAGE_VERSION\s*=\s*['\"]([^'\"]+)['\"]",
        html,
    )
    return match.group(1) if match else "unknown"


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

    html_text = args.input.read_text(encoding="utf-8")
    upload_page_version = extract_upload_page_version(html_text)
    data = html_text.encode("utf-8") + b"\0"
    guard = f"{args.symbol.upper()}_H"
    size_macro = f"{args.symbol.upper()}_SIZE"
    version_macro = f"{args.symbol.upper()}_VERSION"

    header = f"""#ifndef {guard}
#define {guard}

#include <stddef.h>

#define {size_macro} {len(data)}u
#define {version_macro} "{c_string(upload_page_version)}"

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
