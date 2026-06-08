#!/usr/bin/env python3
"""Generate per-executable expected JSON files from cases_manifest.json."""

import json
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"

def main():
    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    binary_meta = {b["name"]: b for b in manifest["binaries"]}
    cases_by_binary = defaultdict(list)
    for case in manifest["cases"]:
        cases_by_binary[case["binary"]].append(case)

    for binary_name, meta in binary_meta.items():
        cases = cases_by_binary.get(binary_name, [])

        output = {
            "schema_version": 1,
            "program": binary_name,
            "generated_from": "manifests/cases_manifest.json",
            "cases": []
        }

        for case in cases:
            output["cases"].append({
                "id":               case["id"],
                "binary":           binary_name,
                "function":         case["function"],
                "anchor":           case["anchor"],
                "expected_sources": case["expected_sources"],
                "forbidden_sources": case["forbidden_sources"],
                "expected_features": case["expected_features"],
                "allowed_warnings": case["allowed_warnings"],
            })

        out_path = ROOT / meta["expected_file"]
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(
            json.dumps(output, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8"
        )
        print(f"[generated] {meta['expected_file']}  ({len(cases)} cases)")

    print("[done] expected JSON generation complete.")

if __name__ == "__main__":
    main()
