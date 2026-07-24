#!/usr/bin/env bash
set -euo pipefail

STATE="${1:-}"
DEVICE_URL="${DEVICE_URL:-http://10.63.27.1:80}"

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
