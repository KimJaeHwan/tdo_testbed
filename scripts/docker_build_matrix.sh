#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/dist/docker-linux}"
PLATFORMS="${DFB_DOCKER_PLATFORMS:-linux/386,linux/amd64,linux/arm/v7,linux/arm64}"
SMOKE="${DFB_RUN_SMOKE:-1}"

mkdir -p "$OUT_DIR"

echo "[*] Build output: $OUT_DIR"
echo "[*] Platforms   : $PLATFORMS"
echo "[*] Smoke tests : $SMOKE"

docker buildx inspect --bootstrap >/dev/null

IFS=',' read -r -a PLATFORM_LIST <<< "$PLATFORMS"

for platform in "${PLATFORM_LIST[@]}"; do
    platform="$(printf '%s' "$platform" | tr -d '[:space:]')"
    if [[ -z "$platform" ]]; then
        continue
    fi

    platform_dir="${platform//\//_}"
    platform_dir="${platform_dir//:/_}"
    platform_out="$OUT_DIR/$platform_dir"

    mkdir -p "$platform_out"
    echo "[*] Building $platform -> $platform_out"

    docker buildx build \
        --file "$ROOT/docker/linux/Dockerfile" \
        --platform "$platform" \
        --build-arg "DFB_RUN_SMOKE=$SMOKE" \
        --output "type=local,dest=$platform_out" \
        "$ROOT"
done

echo "[+] Docker multi-arch build complete."
echo "    Artifacts: $OUT_DIR"
