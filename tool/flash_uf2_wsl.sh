#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

UF2_PATH="${UF2_PATH:-$ROOT_DIR/build/firmware.uf2}"
BOOTLOADER_URL="${BOOTLOADER_URL:-http://10.63.27.1/api/rebootToUf2}"
MOUNT_LABEL="${MOUNT_LABEL:-RPI-RP2}"
WINDOWS_DRIVE="${WINDOWS_DRIVE:-}"
TIMEOUT_SECONDS="${TIMEOUT_SECONDS:-60}"
BUILD_FIRST="${BUILD_FIRST:-0}"
SKIP_REBOOT="${SKIP_REBOOT:-0}"

fail() {
  printf '[flash] ERROR: %s\n' "$*" >&2
  exit 1
}

command -v powershell.exe >/dev/null 2>&1 || fail "powershell.exe not found. Enable WSL interop first."
command -v cmd.exe >/dev/null 2>&1 || fail "cmd.exe not found. Enable WSL interop first."
command -v wslpath >/dev/null 2>&1 || fail "wslpath not found"

if [ "$BUILD_FIRST" = "1" ]; then
  printf '[flash] building firmware\n'
  cmake --build "$ROOT_DIR/build" -j
fi

[ -f "$UF2_PATH" ] || fail "UF2 not found: $UF2_PATH"

PS_SCRIPT="$(wslpath -w "$SCRIPT_DIR/flash_uf2_windows.ps1")"
UF2_WIN="$(wslpath -w "$UF2_PATH")"

PS_ARGS=(
  -NoProfile
  -ExecutionPolicy Bypass
  -File "$PS_SCRIPT"
  -Uf2Path "$UF2_WIN"
  -BootloaderUrl "$BOOTLOADER_URL"
  -MountLabel "$MOUNT_LABEL"
  -TimeoutSeconds "$TIMEOUT_SECONDS"
)

if [ -n "$WINDOWS_DRIVE" ]; then
  PS_ARGS+=(-WindowsDrive "$WINDOWS_DRIVE")
fi

if [ "$SKIP_REBOOT" = "1" ]; then
  PS_ARGS+=(-SkipReboot)
fi

powershell.exe "${PS_ARGS[@]}" && exit 0

printf '[flash] direct powershell.exe failed; retrying through cmd.exe launcher\n' >&2
exec cmd.exe /C powershell.exe "${PS_ARGS[@]}"
