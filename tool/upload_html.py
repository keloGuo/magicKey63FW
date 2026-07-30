#!/usr/bin/env python3
import argparse
import json
import os
import time
import urllib.request
from pathlib import Path


CHUNK_SIZE = 1024
ROOT_DIR = Path(__file__).resolve().parents[1]
DEFAULT_HTML = ROOT_DIR / "web" / "webServer" / "out" / "index.html"


def post_json(device_url, api, payload, timeout):
    body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
    req = urllib.request.Request(
        device_url.rstrip("/") + "/api/" + api,
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.read().decode("utf-8", errors="replace")


def post_chunk(device_url, chunk, timeout):
    boundary = "----WebKitFormBoundaryMagicKeyUpload"
    body = (
        ("--" + boundary + "\r\n").encode("ascii")
        + b'Content-Disposition: form-data; name="file"; filename="index.html"\r\n'
        + b"Content-Type: application/octet-stream\r\n\r\n"
        + chunk
        + ("\r\n--" + boundary + "--\r\n").encode("ascii")
    )
    req = urllib.request.Request(
        device_url.rstrip("/") + "/api/updateP",
        data=body,
        headers={"Content-Type": "multipart/form-data; boundary=" + boundary},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.read().decode("utf-8", errors="replace")


def upload_html(device_url, path, timeout, delay):
    data = path.read_bytes()
    print(post_json(device_url, "updateStart", {"size": len(data), "type": 1}, timeout))
    for offset in range(0, len(data), CHUNK_SIZE):
        chunk = data[offset:offset + CHUNK_SIZE]
        print(f"[upload] offset={offset} len={len(chunk)}")
        print(post_chunk(device_url, chunk, timeout))
        payload = {
            "offset": offset,
            "cleckSum": sum(chunk),
            "len": len(chunk),
            "over": 1 if offset + len(chunk) >= len(data) else 0,
        }
        print(post_json(device_url, "webFileUpdatePpakgEnter", payload, timeout))
        time.sleep(delay)


def main():
    parser = argparse.ArgumentParser(description="Upload single-line MagicKey index.html to device littlefs.")
    parser.add_argument("html", nargs="?", default=str(DEFAULT_HTML))
    parser.add_argument("--device-url", default=os.environ.get("DEVICE_URL", "http://10.63.27.1:80"))
    parser.add_argument("--timeout", type=float, default=3.0)
    parser.add_argument("--delay", type=float, default=0.02)
    args = parser.parse_args()
    upload_html(args.device_url, Path(args.html), args.timeout, args.delay)


if __name__ == "__main__":
    main()
