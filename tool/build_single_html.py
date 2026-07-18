#!/usr/bin/env python3
import argparse
import re
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[1]
WEB_DIR = ROOT_DIR / "web" / "webServer"


def strip_js_comments(text):
    out = []
    i = 0
    state = "code"
    quote = ""
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "code":
            if ch in ("'", '"', "`"):
                quote = ch
                state = "string"
                out.append(ch)
                i += 1
            elif ch == "/" and nxt == "/":
                i += 2
                while i < len(text) and text[i] not in "\r\n":
                    i += 1
            elif ch == "/" and nxt == "*":
                i += 2
                while i + 1 < len(text) and not (text[i] == "*" and text[i + 1] == "/"):
                    i += 1
                i += 2
            else:
                out.append(ch)
                i += 1
        else:
            out.append(ch)
            if ch == "\\":
                if i + 1 < len(text):
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
            elif ch == quote:
                state = "code"
                i += 1
            else:
                i += 1
    return "".join(out)


def js_to_single_line(text):
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    out = []
    for index, line in enumerate(lines):
        out.append(line)
        if index == len(lines) - 1:
            continue

        next_line = lines[index + 1]
        if (
            line.endswith(";")
            or line.endswith("{")
            or line.endswith(",")
            or line.endswith(":")
            or re.match(r"^[A-Za-z_$][A-Za-z0-9_$]*\s*:", line)
            or line.endswith(("(", "[", ".", "+", "-", "*", "/", "&&", "||", "?"))
            or line in ("else", "try", "do")
            or line.startswith("else ")
            or next_line == "{"
            or (line.endswith("}") and next_line.startswith(("else", "catch", "finally", "while")))
        ):
            out.append(" ")
        else:
            out.append("; ")
    return "".join(out)


def minify_text(text):
    return " ".join(line.strip() for line in text.splitlines() if line.strip())


def inline_assets(html_path):
    root = html_path.parent
    html = html_path.read_text(encoding="utf-8")
    css = (root / "index.css").read_text(encoding="utf-8")
    js = (root / "main.js").read_text(encoding="utf-8")

    css = re.sub(r"/\*.*?\*/", "", css, flags=re.S)
    js = js_to_single_line(strip_js_comments(js))

    html = re.sub(
        r"<!--\s*inject:css\s*-->.*?<!--\s*endinject\s*-->",
        "<style>" + css + "</style>",
        html,
        flags=re.S,
    )
    html = re.sub(
        r"<!--\s*inject:js\s*-->.*?<!--\s*endinject\s*-->",
        "<script>" + js + "</script>",
        html,
        flags=re.S,
    )
    html = re.sub(r"<!--.*?-->", "", html, flags=re.S)
    return minify_text(html)


def main():
    parser = argparse.ArgumentParser(description="Inline web assets and write a single-line HTML file.")
    parser.add_argument("-i", "--input", default=str(WEB_DIR / "index.html"), help="source HTML path")
    parser.add_argument("-o", "--output", default=str(WEB_DIR / "out" / "index.html"), help="output HTML path")
    args = parser.parse_args()

    html_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(inline_assets(html_path), encoding="utf-8")
    print(f"wrote {output_path}")


if __name__ == "__main__":
    main()
