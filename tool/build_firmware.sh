#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
GENERATOR="${GENERATOR:-}"
CLEAN=0

usage() {
  cat <<'EOF'
Usage: tool/build_firmware.sh [options]

Options:
  --clean             Remove build directory before configuring.
  --incremental       Reuse the existing build directory. This is the default.
  --release           Configure CMake with CMAKE_BUILD_TYPE=Release. Default.
  --debug             Configure CMake with CMAKE_BUILD_TYPE=Debug.
  --build-dir DIR     Override build directory. Default: build.
  --generator NAME    CMake generator for new build dirs. Default: Ninja when available.
  -j, --jobs N        Parallel build jobs. Default: nproc.
  -h, --help          Show this help.

Environment:
  PICO_SDK_PATH       Pico SDK path. If unset, ../pico-sdk is used when found.
  BUILD_TYPE          Release or Debug. Command line options take precedence.
  JOBS                Parallel build jobs. Command line options take precedence.
  GENERATOR           CMake generator for new build dirs. Command line options take precedence.
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --clean)
      CLEAN=1
      shift
      ;;
    --incremental)
      CLEAN=0
      shift
      ;;
    --release)
      BUILD_TYPE="Release"
      shift
      ;;
    --debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    --build-dir)
      [ "$#" -ge 2 ] || { echo "missing value for --build-dir" >&2; exit 2; }
      BUILD_DIR="$2"
      shift 2
      ;;
    --generator)
      [ "$#" -ge 2 ] || { echo "missing value for --generator" >&2; exit 2; }
      GENERATOR="$2"
      shift 2
      ;;
    -j|--jobs)
      [ "$#" -ge 2 ] || { echo "missing value for $1" >&2; exit 2; }
      JOBS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

case "$BUILD_TYPE" in
  Release|Debug|RelWithDebInfo|MinSizeRel) ;;
  *)
    echo "unsupported BUILD_TYPE: $BUILD_TYPE" >&2
    exit 2
    ;;
esac

if [ -z "${PICO_SDK_PATH:-}" ]; then
  if [ -d "$ROOT_DIR/../pico-sdk" ]; then
    export PICO_SDK_PATH="$(cd "$ROOT_DIR/../pico-sdk" && pwd)"
  else
    echo "PICO_SDK_PATH is not set and ../pico-sdk was not found" >&2
    exit 1
  fi
fi

if [ "$CLEAN" = "1" ]; then
  rm -rf "$BUILD_DIR"
fi

if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
  if [ -z "$GENERATOR" ] && command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
  fi
fi

CONFIGURE_ARGS=(-S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE")
if [ -n "$GENERATOR" ]; then
  CONFIGURE_ARGS+=(-G "$GENERATOR")
fi

cmake "${CONFIGURE_ARGS[@]}"
cmake --build "$BUILD_DIR" -j "$JOBS"

ELF="$BUILD_DIR/firmware.elf"
UF2="$BUILD_DIR/firmware.uf2"
MAP="$BUILD_DIR/firmware.elf.map"

[ -f "$ELF" ] || { echo "missing ELF: $ELF" >&2; exit 1; }
[ -f "$UF2" ] || { echo "missing UF2: $UF2" >&2; exit 1; }
[ -f "$MAP" ] || { echo "missing MAP: $MAP" >&2; exit 1; }

OUT_DIR="$BUILD_DIR/release/$BUILD_TYPE"
mkdir -p "$OUT_DIR"
cp -f "$UF2" "$OUT_DIR/firmware.uf2"
cp -f "$ELF" "$OUT_DIR/firmware.elf"
cp -f "$MAP" "$OUT_DIR/firmware.map"

SIZE_TOOL="${SIZE_TOOL:-arm-none-eabi-size}"
"$SIZE_TOOL" "$ELF" | tee "$OUT_DIR/firmware.size.txt"

SDK_COMMIT="unknown"
SDK_DESCRIBE="unknown"
if git -C "$PICO_SDK_PATH" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  SDK_COMMIT="$(git -C "$PICO_SDK_PATH" rev-parse HEAD 2>/dev/null || echo unknown)"
  SDK_DESCRIBE="$(git -C "$PICO_SDK_PATH" describe --tags --always --dirty 2>/dev/null || echo unknown)"
fi

{
  printf 'build_type=%s\n' "$BUILD_TYPE"
  printf 'jobs=%s\n' "$JOBS"
  printf 'generator=%s\n' "${GENERATOR:-existing}"
  printf 'build_dir=%s\n' "$BUILD_DIR"
  printf 'output_dir=%s\n' "$OUT_DIR"
  printf 'pico_sdk_path=%s\n' "$PICO_SDK_PATH"
  printf 'pico_sdk_commit=%s\n' "$SDK_COMMIT"
  printf 'pico_sdk_describe=%s\n' "$SDK_DESCRIBE"
  printf 'uf2=%s\n' "$OUT_DIR/firmware.uf2"
  printf 'elf=%s\n' "$OUT_DIR/firmware.elf"
  printf 'map=%s\n' "$OUT_DIR/firmware.map"
} > "$OUT_DIR/build-info.txt"

printf '\n[build] artifacts written to %s\n' "$OUT_DIR"
printf '[build] sdk %s %s\n' "$SDK_DESCRIBE" "$SDK_COMMIT"
