#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="${UCODEX_VERSION:-0.1.0}"
ARCH="${UCODEX_ARCH:-all}"
PACKAGE="ucodex"
BUILD_DIR="${TMPDIR:-/tmp}/ucodex-deb-build"
PKG_DIR="$BUILD_DIR/${PACKAGE}_${VERSION}_${ARCH}"
OUT_DIR="$ROOT_DIR/dist"
DEB_PATH="$OUT_DIR/${PACKAGE}_${VERSION}_${ARCH}.deb"

rm -rf "$PKG_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p \
  "$PKG_DIR/DEBIAN" \
  "$PKG_DIR/usr/local/bin" \
  "$PKG_DIR/usr/local/lib/ucodex" \
  "$PKG_DIR/usr/share/doc/ucodex"

install -m 0755 "$ROOT_DIR/tool/ucodex" "$PKG_DIR/usr/local/bin/ucodex"
install -m 0755 "$ROOT_DIR/tool/codex_app_server_hook.py" "$PKG_DIR/usr/local/lib/ucodex/codex_app_server_hook.py"
ln -s ../lib/ucodex/codex_app_server_hook.py "$PKG_DIR/usr/local/bin/ucodex-status"

cat > "$PKG_DIR/DEBIAN/control" <<EOF_CONTROL
Package: $PACKAGE
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Depends: bash, python3, procps, iproute2
Maintainer: kelo <kelo@localhost>
Description: Codex launcher with MagicKey63 device status synchronization
 ucodex starts a Codex App Server, launches a status watcher, and enters
 the Codex TUI through the App Server so MagicKey63 LEDs can reflect Codex
 working states.
EOF_CONTROL

cat > "$PKG_DIR/usr/share/doc/ucodex/README" <<'EOF_README'
ucodex
======

Commands:
  ucodex
  ucodex resume --last
  ucodex-status active --force
  ucodex-status waiting --force
  ucodex-status idle --force

Runtime files:
  ~/.ucodex/logs/
  ~/.ucodex/status_state

Environment:
  DEVICE_URL=http://10.63.27.1:80
  CODEX_STATUS_PORT=4222
  CODEX_BIN=codex
EOF_README

chmod 0644 "$PKG_DIR/DEBIAN/control" "$PKG_DIR/usr/share/doc/ucodex/README"
find "$PKG_DIR" -type d -exec chmod 0755 {} +
mkdir -p "$OUT_DIR"
dpkg-deb --build --root-owner-group "$PKG_DIR" "$DEB_PATH"
dpkg-deb --info "$DEB_PATH"
printf '%s\n' "$DEB_PATH"
