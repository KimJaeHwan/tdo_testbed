#!/usr/bin/env python3
"""
generate_registry_from_manifest.py — 런타임 케이스 등록 C 파일 생성기

cases_manifest.json을 읽어 바이너리별 런타임 등록 파일(dfbench_runtime_*.c)을
자동으로 생성한다. 이 파일들은 각 실행 파일이 "--list" / "--run DFBxxx" 명령을
처리할 때 케이스 목록을 찾기 위해 컴파일에 포함된다.

데이터 흐름:
  manifests/cases_manifest.json
        │  (단일 진실 원본 — source of truth)
        ▼
  generate_registry_from_manifest.py
        │
        ├─▶ src/dfbench_runtime_win_core.c
        ├─▶ src/dfbench_runtime_cpp.c
        ├─▶ src/dfbench_runtime_cpp_exceptions.c
        └─▶ src/dfbench_runtime_posix.c

주의: 생성된 C 파일을 직접 수정하지 말 것.
      manifest를 수정한 뒤 이 스크립트를 재실행하면 덮어씌워진다.
      (현재 gitignore에서 src/dfbench_runtime_*.c를 제외하는 이유도 이 때문이다.)

사용법:
  python tools/generate_registry_from_manifest.py

생성되는 C 파일 구조 (예: dfbench_runtime_win_core.c):
  // 각 케이스 함수 전방 선언
  DFB_EXTERN_C void case_DFB001_direct_value(void);
  DFB_EXTERN_C void case_DFB002_arithmetic_value(void);
  ...

  // 케이스 테이블: {ID 문자열, 이름 문자열, 함수 포인터}
  static const dfb_case_entry_t g_cases[] = {
      {"DFB001", "direct_value", case_DFB001_direct_value},
      {"DFB002", "arithmetic_value", case_DFB002_arithmetic_value},
      ...
  };

  // 런타임이 케이스 목록을 조회할 때 호출하는 함수
  const dfb_case_entry_t *dfb_get_cases(size_t *count) { ... }
"""

import json
from pathlib import Path
from collections import defaultdict

# 프로젝트 루트: 이 스크립트의 상위 디렉토리
ROOT          = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"

# 생성 파일 상단에 붙는 경고 주석 및 공통 include
HEADER = """\
/* AUTO-GENERATED — do not edit manually.
 * Regenerate with: python tools/generate_registry_from_manifest.py
 */
#include <stddef.h>
#include "dfbench_cases.h"

"""


def generate_runtime(binary_name: str, cases: list, runtime_file: str) -> None:
    """
    단일 바이너리에 대한 런타임 등록 C 파일을 생성한다.

    Args:
        binary_name:  생성 대상 바이너리 이름 (예: "dfbench_win_core")
        cases:        해당 바이너리에 속한 케이스 딕셔너리 목록
        runtime_file: 출력 파일의 프로젝트 루트 상대 경로
    """
    lines = [HEADER]

    # ── 1. 케이스 함수 전방 선언 ─────────────────────────────────────
    # 각 케이스 함수는 별도 소스 파일(cases_*.c)에 정의되어 있다.
    # 여기서는 링커가 심볼을 해결할 수 있도록 extern 선언만 한다.
    lines.append("/* case function declarations */\n")
    for case in cases:
        fn = case["function"]
        lines.append(f"DFB_EXTERN_C void {fn}(void);\n")

    # ── 2. 케이스 테이블 (g_cases[]) ─────────────────────────────────
    # dfb_case_entry_t = {id, name, fn_ptr} 구조체 배열.
    # 런타임이 "--list" 또는 "--run DFBxxx" 명령을 처리할 때 이 배열을 순회한다.
    lines.append("\nstatic const dfb_case_entry_t g_cases[] = {\n")
    for case in cases:
        cid  = case["id"]       # 예: "DFB001"
        name = case["name"]     # 예: "direct_value"
        fn   = case["function"] # 예: case_DFB001_direct_value
        lines.append(f'    {{"{cid}", "{name}", {fn}}},\n')
    lines.append("};\n\n")

    # ── 3. dfb_get_cases() — 케이스 목록 조회 함수 ───────────────────
    # main.c의 런타임 루프가 이 함수를 호출해 케이스 목록과 개수를 얻는다.
    lines.append("const dfb_case_entry_t *dfb_get_cases(size_t *count) {\n")
    lines.append("    if (count) {\n")
    lines.append("        *count = sizeof(g_cases) / sizeof(g_cases[0]);\n")
    lines.append("    }\n")
    lines.append("    return g_cases;\n")
    lines.append("}\n")

    # ── 파일 쓰기 ─────────────────────────────────────────────────────
    out_path = ROOT / runtime_file
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("".join(lines), encoding="utf-8")
    print(f"[generated] {runtime_file}  ({len(cases)} cases)")


def main():
    # ── manifest 로드 ─────────────────────────────────────────────────
    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    # binaries 섹션을 이름 기준 딕셔너리로 변환
    binary_meta = {b["name"]: b for b in manifest["binaries"]}

    # 케이스를 바이너리 이름별로 묶는다
    cases_by_binary = defaultdict(list)
    for case in manifest["cases"]:
        cases_by_binary[case["binary"]].append(case)

    # ── 바이너리별 런타임 파일 생성 ───────────────────────────────────
    for binary_name, meta in binary_meta.items():
        cases = cases_by_binary.get(binary_name, [])
        generate_runtime(binary_name, cases, meta["runtime_file"])

    print("[done] registry generation complete.")


if __name__ == "__main__":
    main()
