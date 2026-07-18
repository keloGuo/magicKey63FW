#!/usr/bin/env python3
import argparse
import base64
import hashlib
import json
import os
import socket
import sys
import time
import urllib.request
import shlex
import subprocess
from urllib.parse import urlparse
from pathlib import Path


APP_SERVER_STATES = {"notLoaded", "idle", "active", "systemError"}
VALID_STATES = {"idle", "working", "success", "error", "waiting"} | APP_SERVER_STATES

STATE_ALIASES = {
    "idle": "idle",
    "notloaded": "notLoaded",
    "ready": "idle",
    "stopped": "idle",
    "working": "working",
    "running": "working",
    "busy": "working",
    "active": "active",
    "in_progress": "working",
    "success": "success",
    "succeeded": "success",
    "complete": "success",
    "completed": "success",
    "done": "success",
    "error": "error",
    "systemerror": "systemError",
    "failed": "error",
    "failure": "error",
    "exception": "error",
    "waiting": "waiting",
    "blocked": "waiting",
    "paused": "waiting",
    "approval": "waiting",
    "approval_requested": "waiting",
    "awaiting_approval": "waiting",
    "needs_input": "waiting",
    "user_input_requested": "waiting",
}

SUCCESS_WORDS = {
    "success", "succeeded", "complete", "completed", "task_complete",
    "turn_complete", "final", "final_answer", "done",
}
ERROR_WORDS = {
    "error", "failed", "failure", "exception", "aborted", "cancelled",
    "canceled", "turn_failed", "tool_error",
}
WAITING_WORDS = {
    "waiting", "blocked", "paused", "approval", "approval_requested",
    "request_user_input", "user_input_requested", "needs_input",
}
WORKING_WORDS = {
    "working", "running", "started", "start", "in_progress", "resumed",
    "user_message", "agent_message", "tool_call", "function_call",
    "patch_apply_begin", "command_started", "response_item",
}


def iter_strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for item in value.values():
            yield from iter_strings(item)
    elif isinstance(value, list):
        for item in value:
            yield from iter_strings(item)


def normalize_token(value):
    return value.strip().lower().replace("-", "_").replace(" ", "_")


def parse_payload(raw):
    raw = raw.strip()
    if not raw:
        return None
    if normalize_token(raw) in VALID_STATES:
        return normalize_token(raw)
    try:
        return json.loads(raw)
    except json.JSONDecodeError:
        return raw


def hook_state(payload):
    if isinstance(payload, str):
        token = normalize_token(payload)
        if token in STATE_ALIASES:
            return STATE_ALIASES[token]
        tokens = [token]
    else:
        tokens = [normalize_token(s) for s in iter_strings(payload)]

    token_set = set(tokens)
    for token in tokens:
        if token in STATE_ALIASES:
            return STATE_ALIASES[token]
    if token_set & ERROR_WORDS:
        return "error"
    if token_set & WAITING_WORDS:
        return "waiting"
    if token_set & SUCCESS_WORDS:
        return "success"
    if token_set & WORKING_WORDS:
        return "working"
    return "working"


def fetch_app_status(status_url, timeout):
    req = urllib.request.Request(status_url, method="GET")
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        body = resp.read().decode("utf-8", errors="replace")
    return parse_payload(body)


class WebSocketJsonRpc:
    def __init__(self, url, timeout):
        self.url = url
        self.timeout = timeout
        self.sock = None
        self.next_id = 1

    def connect(self):
        parsed = urlparse(self.url)
        if parsed.scheme != "ws":
            raise ValueError("only ws:// app-server URLs are supported")

        host = parsed.hostname or "127.0.0.1"
        port = parsed.port or 80
        path = parsed.path or "/"
        if parsed.query:
            path += "?" + parsed.query

        sock = socket.create_connection((host, port), timeout=self.timeout)
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        req = (
            f"GET {path} HTTP/1.1\r\n"
            f"Host: {host}:{port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n"
        )
        sock.sendall(req.encode("ascii"))
        response = b""
        while b"\r\n\r\n" not in response:
            chunk = sock.recv(4096)
            if not chunk:
                raise RuntimeError("websocket handshake failed")
            response += chunk
        if b" 101 " not in response.split(b"\r\n", 1)[0]:
            raise RuntimeError(response.decode("utf-8", errors="replace"))
        expected = base64.b64encode(hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode("ascii")).digest())
        if expected not in response:
            raise RuntimeError("websocket accept key mismatch")
        self.sock = sock

    def close(self):
        if self.sock is not None:
            try:
                self.sock.close()
            finally:
                self.sock = None

    def send_json(self, value):
        payload = json.dumps(value, separators=(",", ":")).encode("utf-8")
        header = bytearray([0x81])
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < 65536:
            header.extend([0x80 | 126, (length >> 8) & 0xff, length & 0xff])
        else:
            header.append(0x80 | 127)
            header.extend(length.to_bytes(8, "big"))
        mask = os.urandom(4)
        masked = bytes(payload[i] ^ mask[i % 4] for i in range(length))
        self.sock.sendall(bytes(header) + mask + masked)

    def recv_json(self, timeout=None):
        old_timeout = self.sock.gettimeout()
        if timeout is not None:
            self.sock.settimeout(timeout)
        try:
            while True:
                first = self._read_exact(2)
                opcode = first[0] & 0x0f
                length = first[1] & 0x7f
                masked = (first[1] & 0x80) != 0
                if length == 126:
                    length = int.from_bytes(self._read_exact(2), "big")
                elif length == 127:
                    length = int.from_bytes(self._read_exact(8), "big")
                mask = self._read_exact(4) if masked else b""
                payload = self._read_exact(length)
                if masked:
                    payload = bytes(payload[i] ^ mask[i % 4] for i in range(length))
                if opcode == 8:
                    raise EOFError("websocket closed")
                if opcode == 9:
                    self._send_pong(payload)
                    continue
                if opcode == 1:
                    return json.loads(payload.decode("utf-8"))
        finally:
            if timeout is not None:
                self.sock.settimeout(old_timeout)

    def _read_exact(self, size):
        data = b""
        while len(data) < size:
            chunk = self.sock.recv(size - len(data))
            if not chunk:
                raise EOFError("websocket closed")
            data += chunk
        return data

    def _send_pong(self, payload):
        self.sock.sendall(bytes([0x8a, len(payload)]) + payload)

    def request(self, method, params=None):
        request_id = self.next_id
        self.next_id += 1
        msg = {"id": request_id, "method": method, "params": params if params is not None else {}}
        self.send_json(msg)
        while True:
            response = self.recv_json()
            if response.get("id") == request_id:
                if "error" in response:
                    raise RuntimeError(response["error"])
                return response.get("result")


def initialize_app_server(client):
    client.request(
        "initialize",
        {
            "clientInfo": {
                "name": "magickey_codex_status",
                "title": "MagicKey Codex Status",
                "version": "0.1.0",
            },
            "capabilities": {"experimentalApi": True},
        },
    )
    client.send_json({"method": "initialized", "params": {}})


def map_thread_status(status):
    if isinstance(status, str):
        if status in APP_SERVER_STATES:
            return status
        return hook_state(status)
    if not isinstance(status, dict):
        return "notLoaded"
    status_type = str(status.get("type", "notLoaded"))
    active_flags = status.get("activeFlags", [])
    if status_type == "active" and isinstance(active_flags, list):
        flag_tokens = {normalize_token(str(flag)) for flag in active_flags}
        if "waitingonapproval" in flag_tokens or "waiting_on_approval" in flag_tokens:
            return "waiting"
    if status_type in APP_SERVER_STATES:
        return status_type
    return "notLoaded"


def aggregate_states(states):
    if "systemError" in states:
        return "systemError"
    if "waiting" in states:
        return "waiting"
    if "active" in states:
        return "active"
    if "idle" in states:
        return "idle"
    return "notLoaded"


def read_app_server_state(client, thread_id="", args=None):
    if thread_id:
        result = client.request("thread/read", {"threadId": thread_id})
        thread = result.get("thread", {}) if isinstance(result, dict) else {}
        if args is not None:
            log_debug(args, f"thread {thread_id} raw_status={thread.get('status')}")
        return map_thread_status(thread.get("status"))

    loaded = client.request("thread/loaded/list") or {}
    thread_ids = loaded.get("data", []) if isinstance(loaded, dict) else []
    if args is not None:
        log_debug(args, f"thread/loaded/list raw={loaded}")
        log_debug(args, f"loaded threads={thread_ids}")
    if not thread_ids:
        if args is not None:
            log_debug(args, "no loaded threads from this App Server; current state maps to notLoaded")
        return "notLoaded"

    states = []
    for thread_id in thread_ids:
        result = client.request("thread/read", {"threadId": thread_id})
        thread = result.get("thread", {}) if isinstance(result, dict) else {}
        if args is not None:
            log_debug(args, f"thread {thread_id} raw_status={thread.get('status')}")
        states.append(map_thread_status(thread.get("status")))
    return aggregate_states(states)


def parse_app_server_urls(values):
    urls = []
    for value in values:
        if not value:
            continue
        for item in value.split(","):
            item = item.strip()
            if item:
                urls.append(item)
    return urls


def discover_app_server_urls():
    urls = []
    try:
        output = subprocess.check_output(["ps", "-eo", "args="], text=True)
    except Exception:
        return urls

    for line in output.splitlines():
        if "codex" not in line or "app-server" not in line or "--listen" not in line:
            continue
        try:
            parts = shlex.split(line)
        except ValueError:
            parts = line.split()
        for index, part in enumerate(parts):
            url = ""
            if part == "--listen" and index + 1 < len(parts):
                url = parts[index + 1]
            elif part.startswith("--listen="):
                url = part.split("=", 1)[1]
            if url.startswith("ws://") and url not in urls:
                urls.append(url)
    return urls


def connect_app_server(url, timeout):
    client = WebSocketJsonRpc(url, timeout)
    client.connect()
    initialize_app_server(client)
    return client


def read_payload(args):
    if args.state:
        return parse_payload(args.state)

    if args.status_url:
        return fetch_app_status(args.status_url, args.timeout)

    env_payload = os.environ.get("CODEX_HOOK_PAYLOAD") or os.environ.get("APP_SERVER_HOOK_PAYLOAD")
    if env_payload:
        return parse_payload(env_payload)

    if not sys.stdin.isatty():
        return parse_payload(sys.stdin.read())

    return "working"


def read_last_state(path):
    try:
        return path.read_text(encoding="utf-8").strip()
    except OSError:
        return ""


def write_last_state(path, state):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(state, encoding="utf-8")


def notify_device(device_url, state, timeout):
    body = json.dumps({"state": state}, separators=(",", ":")).encode("utf-8")
    req = urllib.request.Request(
        device_url.rstrip("/") + "/api/codexStatus",
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.read().decode("utf-8", errors="replace")


def try_notify_device(args, state):
    try:
        return notify_device(args.device_url, state, args.timeout), None
    except Exception as exc:
        return "", exc


def log_debug(args, message):
    if args.verbose:
        print(message, flush=True)


def main():
    parser = argparse.ArgumentParser(
        description="Codex App Server hook/watcher: read Codex status and update keyboard WS2812 status."
    )
    parser.add_argument("state", nargs="?", help="state name or raw hook JSON")
    parser.add_argument(
        "--status-url",
        default=os.environ.get("CODEX_APP_STATUS_URL", ""),
        help="optional plain HTTP status endpoint returning JSON/text",
    )
    parser.add_argument(
        "--app-server",
        action="append",
        default=[],
        help="Codex App Server websocket URL, e.g. ws://127.0.0.1:4222",
    )
    parser.add_argument(
        "--thread-id",
        default=os.environ.get("CODEX_APP_THREAD_ID", ""),
        help="specific App Server thread id to read; by default reads all loaded threads",
    )
    parser.add_argument(
        "--discover",
        action="store_true",
        help="scan running processes for codex app-server --listen ws://... endpoints",
    )
    parser.add_argument(
        "--discover-interval",
        type=float,
        default=2.0,
        help="seconds between app-server discovery scans",
    )
    parser.add_argument("--watch", action="store_true", help="poll app-server/status-url continuously")
    parser.add_argument("--interval", type=float, default=0.5, help="watch poll interval seconds")
    parser.add_argument("--force", action="store_true", help="notify even when state did not change")
    parser.add_argument("--dry-run", action="store_true", help="print mapped state without notifying")
    parser.add_argument("--verbose", action="store_true", help="print app-server discovery and raw thread status")
    parser.add_argument(
        "--device-url",
        default=os.environ.get("DEVICE_URL", "http://192.168.3.1:80"),
        help="keyboard device base URL",
    )
    parser.add_argument(
        "--state-file",
        default=os.environ.get(
            "CODEX_STATUS_STATE_FILE",
            str(Path.home() / ".ucodex" / "status_state"),
        ),
        help="file used to suppress duplicate notifications",
    )
    parser.add_argument("--timeout", type=float, default=1.0, help="HTTP timeout seconds")
    args = parser.parse_args()

    state_file = Path(args.state_file)
    app_server_urls = parse_app_server_urls(
        args.app_server
        + [
            os.environ.get("CODEX_APP_SERVER_URL", ""),
            os.environ.get("CODEX_APP_SERVER_URLS", ""),
        ]
    )

    if args.watch and not args.status_url and not app_server_urls and not args.discover:
        print("error: --watch requires --app-server, --status-url, CODEX_APP_SERVER_URL, or CODEX_APP_STATUS_URL", file=sys.stderr)
        return 2

    app_clients = {}
    for url in app_server_urls:
        app_clients[url] = connect_app_server(url, args.timeout)

    last_discovery = 0.0

    while True:
        now = time.monotonic()
        if args.discover and now - last_discovery >= args.discover_interval:
            last_discovery = now
            discovered_urls = discover_app_server_urls()
            log_debug(args, f"discovered app-servers: {discovered_urls or []}")
            for url in discovered_urls:
                if url in app_clients:
                    continue
                try:
                    app_clients[url] = connect_app_server(url, args.timeout)
                    print(f"connected app-server {url}", flush=True)
                except Exception as exc:
                    print(f"failed app-server {url}: {exc}", file=sys.stderr, flush=True)

        if app_clients:
            states = []
            disconnected = []
            for url, client in app_clients.items():
                try:
                    state_from_server = read_app_server_state(client, args.thread_id, args)
                    states.append(state_from_server)
                    log_debug(args, f"app-server {url} state={state_from_server}")
                except Exception as exc:
                    print(f"lost app-server {url}: {exc}", file=sys.stderr, flush=True)
                    disconnected.append(url)
            for url in disconnected:
                try:
                    app_clients[url].close()
                finally:
                    del app_clients[url]
            state = aggregate_states(states)
        else:
            payload = read_payload(args)
            state = hook_state(payload)
        last_state = read_last_state(state_file)

        if args.force or state != last_state:
            if args.dry_run:
                print(state)
            else:
                response, error = try_notify_device(args, state)
                if error is None:
                    write_last_state(state_file, state)
                    print(f"{last_state or '-'} -> {state}: {response}", flush=True)
                else:
                    print(f"{last_state or '-'} -> {state}: notify failed: {error}", file=sys.stderr, flush=True)
        elif not args.watch:
            print(f"unchanged {state}")
        else:
            log_debug(args, f"unchanged {state}")

        if not args.watch:
            break

        time.sleep(args.interval)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
