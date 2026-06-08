#!/usr/bin/env python3
"""Validate cases_manifest.json for schema correctness and consistency."""

import json
import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"

VALID_BINARY_NAMES = {
    "dfbench_win_core",
    "dfbench_posix_runtime",
    "dfbench_cpp",
    "dfbench_cpp_exceptions",
}

VALID_PLATFORMS = {"windows", "posix"}
VALID_KINDS = {"core_c", "runtime_bridge", "cpp", "cpp_exceptions"}
ID_PATTERN = re.compile(r"^DFB\d{3}$")
FUNCTION_PATTERN = re.compile(r"^case_DFB\d{3}_\S+$")

ERRORS = []

def error(msg):
    ERRORS.append(msg)
    print(f"[ERROR] {msg}", file=sys.stderr)

def main():
    if not MANIFEST_PATH.exists():
        error(f"Manifest not found: {MANIFEST_PATH}")
        return 1

    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    # schema_version
    if manifest.get("schema_version") != 1:
        error("schema_version must be 1")

    # binaries
    declared_binaries = {}
    for b in manifest.get("binaries", []):
        name = b.get("name", "")
        if name not in VALID_BINARY_NAMES:
            error(f"Unknown binary name: {name}")
        if b.get("platform") not in VALID_PLATFORMS:
            error(f"Binary {name}: invalid platform '{b.get('platform')}'")
        if b.get("kind") not in VALID_KINDS:
            error(f"Binary {name}: invalid kind '{b.get('kind')}'")
        for field in ("runtime_file", "expected_file"):
            if not b.get(field):
                error(f"Binary {name}: missing field '{field}'")
        declared_binaries[name] = b

    # cases
    seen_ids = {}
    for case in manifest.get("cases", []):
        cid = case.get("id", "")

        if not ID_PATTERN.match(cid):
            error(f"Invalid case ID format: '{cid}'")

        if cid in seen_ids:
            error(f"Duplicate case ID: {cid}")
        seen_ids[cid] = True

        binary = case.get("binary", "")
        if binary not in declared_binaries:
            error(f"{cid}: binary '{binary}' not in binaries list")

        src = case.get("source_file", "")
        if not src:
            error(f"{cid}: missing source_file")
        elif not (ROOT / src).exists():
            print(f"[WARN] {cid}: source_file not found yet: {src}")

        fn = case.get("function", "")
        if not FUNCTION_PATTERN.match(fn):
            error(f"{cid}: invalid function name '{fn}'")

        anchor = case.get("anchor", {})
        if not anchor.get("callee"):
            error(f"{cid}: anchor missing callee")
        if not isinstance(anchor.get("arg_index"), int):
            error(f"{cid}: anchor.arg_index must be int")

        for field in ("expected_sources", "forbidden_sources", "expected_features", "allowed_warnings"):
            if not isinstance(case.get(field), list):
                error(f"{cid}: '{field}' must be a list")

    if ERRORS:
        print(f"\n{len(ERRORS)} error(s) found.", file=sys.stderr)
        return 1

    print(f"[OK] Manifest valid. {len(seen_ids)} cases across {len(declared_binaries)} binaries.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
