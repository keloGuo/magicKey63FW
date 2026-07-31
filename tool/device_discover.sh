#!/usr/bin/env bash
set -euo pipefail

STATE_DIR="${UCODEX_STATE_DIR:-$HOME/.ucodex}"
CACHE_FILE="${DEVICE_URL_CACHE:-$STATE_DIR/device_url}"
TIMEOUT="${DEVICE_DISCOVER_TIMEOUT:-0.8}"
CANDIDATES="${DEVICE_URL_CANDIDATES:-http://10.63.27.1:80 http://10.63.27.2:80 http://192.168.3.1:80}"

mkdir -p "$STATE_DIR"

normalize_url() {
  local url="$1"
  if [[ "$url" != http://* && "$url" != https://* ]]; then
    url="http://$url"
  fi
  printf '%s\n' "${url%/}"
}

probe_device() {
  local url
  url="$(normalize_url "$1")"
  local body
  body="$(curl -fsS --max-time "$TIMEOUT" -X POST "$url/api/versionInfo" 2>/dev/null || true)"
  case "$body" in
    *'"firmware"'*'"configVersion"'*'"usbBcdDevice"'*)
      printf '%s\n' "$url"
      return 0
      ;;
  esac
  return 1
}

if [[ -n "${DEVICE_URL:-}" ]]; then
  normalize_url "$DEVICE_URL"
  exit 0
fi

if [[ -f "$CACHE_FILE" ]]; then
  cached="$(tr -d '[:space:]' < "$CACHE_FILE")"
  if [[ -n "$cached" ]] && probe_device "$cached" >/tmp/magickey63_device_url.$$; then
    cat /tmp/magickey63_device_url.$$
    rm -f /tmp/magickey63_device_url.$$
    exit 0
  fi
  rm -f /tmp/magickey63_device_url.$$
fi

for candidate in $CANDIDATES; do
  if found="$(probe_device "$candidate")"; then
    printf '%s\n' "$found" > "$CACHE_FILE"
    printf '%s\n' "$found"
    exit 0
  fi
done

printf '%s\n' "http://10.63.27.1:80"
exit 0
