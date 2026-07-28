#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
RUN_BUILD=1
STRICT_RELEASE="${STRICT_RELEASE:-0}"
FORBIDDEN_IP_PATTERN="${FORBIDDEN_IP_PATTERN:-192\\.168\\.3\\.1}"

usage() {
  cat <<'EOF'
Usage: tool/check_ci.sh [options]

Options:
  --skip-build        Run static checks only.
  --strict-release    Treat release blockers, such as temporary VID/PID notes, as errors.
  --build-dir DIR     Override build directory. Default: build.
  --build-type TYPE   CMake build type. Default: Release.
  -j, --jobs N        Parallel build jobs. Default: nproc.
  -h, --help          Show this help.

Environment:
  PICO_SDK_PATH       Pico SDK path. If unset, ../pico-sdk is used when found.
  FORBIDDEN_IP_PATTERN
                      Extended regexp for old IPs that must not be hard-coded.
                      Default: 192\.168\.3\.1
  STRICT_RELEASE      Same as --strict-release when set to 1.
EOF
}

log() {
  printf '[check] %s\n' "$*"
}

fail() {
  printf '[check] ERROR: %s\n' "$*" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --skip-build)
      RUN_BUILD=0
      shift
      ;;
    --strict-release)
      STRICT_RELEASE=1
      shift
      ;;
    --build-dir)
      [ "$#" -ge 2 ] || fail "missing value for --build-dir"
      BUILD_DIR="$2"
      shift 2
      ;;
    --build-type)
      [ "$#" -ge 2 ] || fail "missing value for --build-type"
      BUILD_TYPE="$2"
      shift 2
      ;;
    -j|--jobs)
      [ "$#" -ge 2 ] || fail "missing value for $1"
      JOBS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      fail "unknown option: $1"
      ;;
  esac
done

cd "$ROOT_DIR"

if [ -z "${PICO_SDK_PATH:-}" ] && [ -d "$ROOT_DIR/../pico-sdk" ]; then
  export PICO_SDK_PATH="$(cd "$ROOT_DIR/../pico-sdk" && pwd)"
fi

log "checking tracked file whitespace"
git diff --check

log "checking generated web page"
WEB_TMP="$(mktemp)"
trap 'rm -f "$WEB_TMP"' EXIT
python3 tool/build_single_html.py -o "$WEB_TMP" >/dev/null
cmp -s "$WEB_TMP" web/webServer/out/index.html || {
  printf '[check] web/webServer/out/index.html is stale\n' >&2
  printf '[check] run: python3 tool/build_single_html.py\n' >&2
  exit 1
}

log "checking old IP hard-coding"
if git grep -n -E "$FORBIDDEN_IP_PATTERN" -- .; then
  fail "found forbidden old IP pattern: $FORBIDDEN_IP_PATTERN"
fi

log "checking temporary VID/PID release notes"
VID_PID_PATTERN='development builds|register a pid\.codes PID|TinyUSB demo VID/PID|临时.*(VID|PID)'
if VID_PID_HITS="$(git grep -n -E "$VID_PID_PATTERN" -- firmware/usb 2>/dev/null)"; then
  if [ "$STRICT_RELEASE" = "1" ]; then
    printf '%s\n' "$VID_PID_HITS" >&2
    fail "temporary VID/PID release note still exists"
  fi
  printf '[check] warning: temporary VID/PID release note still exists\n' >&2
  printf '%s\n' "$VID_PID_HITS" >&2
fi

if [ "$RUN_BUILD" = "1" ]; then
  log "configuring firmware"
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

  log "building firmware"
  cmake --build "$BUILD_DIR" --target firmware -j "$JOBS"

  ELF="$BUILD_DIR/firmware.elf"
  [ -f "$ELF" ] || fail "missing firmware ELF: $ELF"

  SIZE_TOOL="${SIZE_TOOL:-arm-none-eabi-size}"
  command -v "$SIZE_TOOL" >/dev/null 2>&1 || fail "$SIZE_TOOL not found"
  log "firmware size"
  "$SIZE_TOOL" "$ELF"
else
  log "skipping firmware configure/build"
fi

log "ok"
