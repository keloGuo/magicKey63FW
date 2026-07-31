#!/usr/bin/env bash
set -euo pipefail

SCRIPT_PATH="${BASH_SOURCE[0]}"
while [[ -L "$SCRIPT_PATH" ]]; do
  SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"
  SCRIPT_PATH="$(readlink "$SCRIPT_PATH")"
  [[ "$SCRIPT_PATH" != /* ]] && SCRIPT_PATH="$SCRIPT_DIR/$SCRIPT_PATH"
done
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"

STATE="${1:-}"
DEVICE_DISCOVER_SCRIPT="${UCODEX_DEVICE_DISCOVER_SCRIPT:-$SCRIPT_DIR/device_discover.sh}"
if [[ ! -x "$DEVICE_DISCOVER_SCRIPT" && -x "/usr/local/lib/ucodex/device_discover.sh" ]]; then
  DEVICE_DISCOVER_SCRIPT="/usr/local/lib/ucodex/device_discover.sh"
fi
if [[ ! -x "$DEVICE_DISCOVER_SCRIPT" && -x "$HOME/.local/lib/ucodex/device_discover.sh" ]]; then
  DEVICE_DISCOVER_SCRIPT="$HOME/.local/lib/ucodex/device_discover.sh"
fi

if [[ -z "${DEVICE_URL:-}" && -x "$DEVICE_DISCOVER_SCRIPT" ]]; then
  DEVICE_URL="$("$DEVICE_DISCOVER_SCRIPT")"
else
  DEVICE_URL="${DEVICE_URL:-http://10.63.27.1:80}"
fi

case "$STATE" in
  notLoaded|idle|active|systemError|working|success|error|waiting) ;;
  *)
    echo "usage: $0 notLoaded|idle|active|systemError|working|success|error|waiting" >&2
    exit 2
    ;;
esac

curl -sS -X POST \
  -H 'Content-Type: application/json' \
  --data "{\"state\":\"$STATE\"}" \
  "$DEVICE_URL/api/codexStatus"
echo
