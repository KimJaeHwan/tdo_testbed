#!/usr/bin/env python3
"""Smoke test: verify all found dfbench executables run without crashing."""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

EXPECTED_NAMES = [
    "dfbench_win_core",
    "dfbench_posix_runtime",
    "dfbench_cpp",
    "dfbench_cpp_exceptions",
]

def find_executables():
    candidates = []
    for name in EXPECTED_NAMES:
        candidates.extend(ROOT.glob(f"build/**/{name}"))
        candidates.extend(ROOT.glob(f"build/**/{name}.exe"))
    return [p for p in candidates if p.is_file()]

def run_check(exe):
    out = subprocess.check_output([str(exe), "--list"], text=True)
    assert "DFB" in out, f"--list output missing DFB: {exe}"

    out = subprocess.check_output([str(exe), "--run-all"], text=True)
    assert "OK" in out, f"--run-all did not print OK: {exe}"

    print(f"[OK] {exe.name}")

def main():
    exes = find_executables()
    if not exes:
        print("No dfbench executables found under build/", file=sys.stderr)
        return 1

    failures = 0
    for exe in exes:
        try:
            run_check(exe)
        except Exception as e:
            print(f"[FAIL] {exe}: {e}", file=sys.stderr)
            failures += 1

    if failures:
        print(f"\n{failures} executable(s) failed smoke test.", file=sys.stderr)
        return 1

    print(f"\nAll {len(exes)} executable(s) passed smoke test.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
