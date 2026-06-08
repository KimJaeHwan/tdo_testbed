#!/usr/bin/env python3
"""Generate per-executable runtime registry C files from cases_manifest.json."""

import json
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"

HEADER = """\
/* AUTO-GENERATED — do not edit manually.
 * Regenerate with: python tools/generate_registry_from_manifest.py
 */
#include <stddef.h>
#include "dfbench_cases.h"

"""

def generate_runtime(binary_name, cases, runtime_file):
    lines = [HEADER]

    lines.append("/* case function declarations */\n")
    for case in cases:
        fn = case["function"]
        lines.append(f"DFB_EXTERN_C void {fn}(void);\n")

    lines.append("\nstatic const dfb_case_entry_t g_cases[] = {\n")
    for case in cases:
        cid  = case["id"]
        name = case["name"]
        fn   = case["function"]
        lines.append(f'    {{"{cid}", "{name}", {fn}}},\n')
    lines.append("};\n\n")

    lines.append("const dfb_case_entry_t *dfb_get_cases(size_t *count) {\n")
    lines.append("    if (count) {\n")
    lines.append("        *count = sizeof(g_cases) / sizeof(g_cases[0]);\n")
    lines.append("    }\n")
    lines.append("    return g_cases;\n")
    lines.append("}\n")

    out_path = ROOT / runtime_file
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("".join(lines), encoding="utf-8")
    print(f"[generated] {runtime_file}  ({len(cases)} cases)")

def main():
    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    binary_meta = {b["name"]: b for b in manifest["binaries"]}
    cases_by_binary = defaultdict(list)
    for case in manifest["cases"]:
        cases_by_binary[case["binary"]].append(case)

    for binary_name, meta in binary_meta.items():
        cases = cases_by_binary.get(binary_name, [])
        generate_runtime(binary_name, cases, meta["runtime_file"])

    print("[done] registry generation complete.")

if __name__ == "__main__":
    main()
