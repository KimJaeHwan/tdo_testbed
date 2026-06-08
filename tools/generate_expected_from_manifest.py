#!/usr/bin/env python3
"""
generate_expected_from_manifest.py — 정답 JSON 생성기

cases_manifest.json을 읽어 바이너리별 정답 파일(*.expected.json)을 생성한다.
생성된 파일은 Ghidra backward slicer 검증기가 읽어 결과와 비교하는 기준이 된다.

데이터 흐름:
  manifests/cases_manifest.json
        │  (단일 진실 원본 — source of truth)
        ▼
  generate_expected_from_manifest.py
        │
        ├─▶ expected/dfbench_win_core.expected.json
        ├─▶ expected/dfbench_cpp.expected.json
        ├─▶ expected/dfbench_cpp_exceptions.expected.json
        └─▶ expected/dfbench_posix_runtime.expected.json

주의: expected JSON 파일을 직접 수정하지 말 것.
      manifest를 수정한 뒤 이 스크립트를 재실행하면 덮어씌워진다.

사용법:
  python tools/generate_expected_from_manifest.py

출력 파일 형식 (예: dfbench_win_core.expected.json):
  {
    "schema_version": 1,
    "program": "dfbench_win_core",
    "generated_from": "manifests/cases_manifest.json",
    "cases": [
      {
        "id": "DFB001",
        "function": "case_DFB001_direct_value",
        "anchor": {"callee": "dfb_sink_int", "arg_index": 0},
        "expected_sources":  ["dfb_source_A.ret"],  ← 반드시 나와야 할 소스
        "forbidden_sources": [],                     ← 나오면 안 되는 소스
        "expected_features": ["direct_value"],       ← 케이스 특성 태그 (메타)
        "allowed_warnings":  []
      },
      ...
    ]
  }
"""

import json
from pathlib import Path
from collections import defaultdict

# 프로젝트 루트: 이 스크립트의 상위 디렉토리
ROOT          = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"


def main():
    # ── manifest 로드 ─────────────────────────────────────────────────
    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    # binaries 섹션을 이름 기준 딕셔너리로 변환
    #   예: {"dfbench_win_core": {"name": ..., "expected_file": ..., ...}, ...}
    binary_meta = {b["name"]: b for b in manifest["binaries"]}

    # 케이스를 바이너리 이름별로 묶는다
    cases_by_binary = defaultdict(list)
    for case in manifest["cases"]:
        cases_by_binary[case["binary"]].append(case)

    # ── 바이너리별 expected JSON 생성 ────────────────────────────────
    for binary_name, meta in binary_meta.items():
        cases = cases_by_binary.get(binary_name, [])

        # 출력 JSON 루트 객체 구성
        output = {
            "schema_version":  1,
            "program":         binary_name,
            "generated_from":  "manifests/cases_manifest.json",
            "cases":           []
        }

        for case in cases:
            # manifest의 각 케이스에서 검증기에 필요한 필드만 추출한다.
            # source_file / name 같은 개발용 메타 필드는 제외한다.
            output["cases"].append({
                "id":               case["id"],
                "binary":           binary_name,

                # 검증기가 바이너리에서 함수를 찾을 때 사용하는 심볼 이름
                "function":         case["function"],

                # backward slice 시작점:
                #   callee    — 추적을 시작할 함수 호출 (예: dfb_sink_int)
                #   arg_index — 그 함수의 몇 번째 인자에서 시작할지 (0-based)
                "anchor":           case["anchor"],

                # 슬라이서가 반드시 도달해야 하는 소스 목록
                # 형식: "<함수명>.ret" (반환값) 또는 "<함수명>.arg{N}" (인자)
                "expected_sources": case["expected_sources"],

                # 슬라이서 결과에 있으면 안 되는 소스 목록 (오염 탐지용)
                "forbidden_sources": case["forbidden_sources"],

                # 이 케이스가 어떤 패턴을 검증하는지 나타내는 태그
                # 슬라이서가 직접 읽지는 않으며, 결과 리포트 분류용 메타 정보다.
                "expected_features": case["expected_features"],

                # 슬라이서 구현에서 허용하는 경고 메시지 패턴 목록
                "allowed_warnings": case["allowed_warnings"],
            })

        # ── 파일 쓰기 ─────────────────────────────────────────────────
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
