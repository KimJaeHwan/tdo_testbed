#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

for build_dir in "$ROOT"/build/*/; do
    for exe_name in dfbench_win_core dfbench_posix_runtime dfbench_cpp dfbench_cpp_exceptions; do
        exe="$build_dir$exe_name"
        if [[ -x "$exe" ]]; then
            echo "[*] $exe --list"
            "$exe" --list
            echo "[*] $exe --run-all"
            "$exe" --run-all
        fi
    done
done
