#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
UF2_PATH="${UF2_PATH:-$ROOT_DIR/build/firmware.uf2}"
BOOTLOADER_URL="${BOOTLOADER_URL:-http://192.168.3.1/api/rebootToUf2}"
MOUNT_LABEL="${MOUNT_LABEL:-RPI-RP2}"
WINDOWS_DRIVE="${WINDOWS_DRIVE:-}"
TIMEOUT_SECONDS="${TIMEOUT_SECONDS:-60}"
BUILD_FIRST="${BUILD_FIRST:-0}"
SKIP_REBOOT="${SKIP_REBOOT:-0}"

log() {
  printf '[flash] %s\n' "$*"
}

fail() {
  printf '[flash] ERROR: %s\n' "$*" >&2
  exit 1
}

find_uf2_mount_with_powershell() {
  command -v powershell.exe >/dev/null 2>&1 || return 1

  powershell.exe -NoProfile -Command "\
    \$v = Get-Volume -FileSystemLabel '${MOUNT_LABEL}' -ErrorAction SilentlyContinue | Select-Object -First 1; \
    if (\$v -and \$v.DriveLetter) { Write-Output (\$v.DriveLetter + ':\\') }" \
    2>/dev/null | tr -d '\r' | sed -n '1p'
}

copy_uf2_to_windows_drive() {
  local windows_drive="$1"
  local windows_uf2
  windows_uf2="$(wslpath -w "$UF2_PATH")"
  powershell.exe -NoProfile -Command "\
    \$ErrorActionPreference = 'Stop'; \
    Copy-Item -LiteralPath '${windows_uf2}' -Destination '${windows_drive}' -Force; \
    Start-Sleep -Milliseconds 500" >/dev/null || \
    cmd.exe /C copy /Y "$windows_uf2" "$windows_drive" >/dev/null
}

windows_drive_exists() {
  local windows_drive="$1"
  local info_file="${windows_drive}INFO_UF2.TXT"
  powershell.exe -NoProfile -Command "\
    \$ErrorActionPreference = 'Stop'; \
    if (Test-Path -LiteralPath '${info_file}') { exit 0 } else { exit 1 }" >/dev/null 2>&1 || \
    cmd.exe /C "if exist ${info_file} exit /b 0 else exit /b 1" >/dev/null 2>&1
}

if [ "$BUILD_FIRST" = "1" ]; then
  log "building firmware"
  cmake --build "$ROOT_DIR/build" -j
fi

[ -f "$UF2_PATH" ] || fail "UF2 not found: $UF2_PATH"

if [ "$SKIP_REBOOT" != "1" ]; then
  log "requesting UF2 bootloader: $BOOTLOADER_URL"
  curl -fsS -m 2 -X POST "$BOOTLOADER_URL" >/dev/null || \
    log "bootloader request failed; waiting for an already-mounted ${MOUNT_LABEL} drive"
else
  log "skip HTTP reboot request"
fi

log "waiting for ${MOUNT_LABEL} drive"
deadline=$((SECONDS + TIMEOUT_SECONDS))
while [ "$SECONDS" -lt "$deadline" ]; do
  if [ -n "$WINDOWS_DRIVE" ]; then
    if windows_drive_exists "$WINDOWS_DRIVE"; then
      log "copying $(basename "$UF2_PATH") to Windows drive $WINDOWS_DRIVE"
      if copy_uf2_to_windows_drive "$WINDOWS_DRIVE"; then
        log "done"
        exit 0
      fi
      log "copy failed; waiting for ${WINDOWS_DRIVE} to become ready"
    fi
  fi

  if windows_drive="$(find_uf2_mount_with_powershell)" && [ -n "$windows_drive" ]; then
    log "copying $(basename "$UF2_PATH") to Windows drive $windows_drive"
    copy_uf2_to_windows_drive "$windows_drive"
    log "done"
    exit 0
  fi

  sleep 1
done

fail "timed out waiting for ${MOUNT_LABEL}. Put the device in BOOTSEL mode or increase TIMEOUT_SECONDS."
