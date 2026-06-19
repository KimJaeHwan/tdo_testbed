#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/dist/docker-windows-pe}"
TARGETS="${DFB_PE_TARGETS:-x86,x64}"
DOCKER_PLATFORM="${DFB_PE_DOCKER_PLATFORM:-linux/amd64}"

mkdir -p "$OUT_DIR"

echo "[*] Build output    : $OUT_DIR"
echo "[*] PE targets      : $TARGETS"
echo "[*] Docker platform : $DOCKER_PLATFORM"

docker buildx inspect --bootstrap >/dev/null

docker buildx build \
    --file "$ROOT/docker/windows-pe/Dockerfile" \
    --platform "$DOCKER_PLATFORM" \
    --build-arg "DFB_PE_TARGETS=$TARGETS" \
    --output "type=local,dest=$OUT_DIR" \
    "$ROOT"

echo "[+] Docker Windows PE build complete."
echo "    Artifacts: $OUT_DIR"
