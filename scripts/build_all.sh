#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

build_variant() {
    local preset="$1"
    echo "[*] Configuring $preset..."
    cmake --preset "$preset" -S "$ROOT"
    echo "[*] Building $preset..."
    cmake --build --preset "$preset"
}

# Windows PE cross-build (mingw-w64)
build_variant "win-cross-debug"

# Linux native POSIX build
build_variant "linux-native-debug"

echo "[+] All builds complete."
echo "    PE  artifacts : build/win-cross-debug/"
echo "    ELF artifacts : build/linux-native-debug/"
