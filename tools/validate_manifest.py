#!/usr/bin/env python3
"""
validate_manifest.py — cases_manifest.json 스키마 검증기

케이스를 추가하거나 수정한 후 실행하여 manifests/cases_manifest.json이
올바른 형식인지 확인합니다.

검사 항목:
  - schema_version 값이 1인지
  - 각 binary 항목의 name / platform / kind / 필수 필드 유효성
  - 각 case의 ID 형식 (DFB + 숫자 3자리), 중복 여부
  - binary 이름이 binaries 목록에 선언된 것인지
  - source_file 경로 존재 여부 (없으면 WARN, 에러 아님)
  - function 이름 형식 (case_DFB{숫자}_{설명})
  - anchor 필드 (callee 문자열, arg_index 정수) 존재 여부
  - expected_sources / forbidden_sources 등 리스트 타입 여부

사용법:
  python tools/validate_manifest.py

출력:
  오류가 없으면 → [OK] Manifest valid. N cases across M binaries.
  오류가 있으면 → [ERROR] ... 메시지 출력 후 exit code 1
"""

import json
import sys
import re
from pathlib import Path

# 프로젝트 루트: 이 스크립트의 상위 디렉토리
ROOT = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "manifests" / "cases_manifest.json"

# cases_manifest.json에 선언 가능한 바이너리 이름 집합
VALID_BINARY_NAMES = {
    "dfbench_win_core",
    "dfbench_posix_runtime",
    "dfbench_cpp",
    "dfbench_cpp_exceptions",
}

VALID_PLATFORMS = {"windows", "posix"}
VALID_KINDS     = {"core_c", "runtime_bridge", "cpp", "cpp_exceptions"}

# case ID 형식: DFB 뒤에 정확히 3자리 숫자 (예: DFB001, DFB200)
ID_PATTERN       = re.compile(r"^DFB\d{3}$")
# function 이름 형식: case_DFB{숫자}_{임의 이름} (예: case_DFB001_direct_value)
FUNCTION_PATTERN = re.compile(r"^case_DFB\d{3}_\S+$")

ERRORS = []

def error(msg):
    """오류를 ERRORS 목록에 기록하고 stderr에 출력한다."""
    ERRORS.append(msg)
    print(f"[ERROR] {msg}", file=sys.stderr)


def main():
    # ── manifest 파일 존재 확인 ────────────────────────────────────────
    if not MANIFEST_PATH.exists():
        error(f"Manifest not found: {MANIFEST_PATH}")
        return 1

    with open(MANIFEST_PATH, encoding="utf-8") as f:
        manifest = json.load(f)

    # ── schema_version 확인 ───────────────────────────────────────────
    # 현재 지원 버전은 1뿐이다. 향후 스키마 변경 시 이 값을 올린다.
    if manifest.get("schema_version") != 1:
        error("schema_version must be 1")

    # ── binaries 섹션 검증 ────────────────────────────────────────────
    # 각 바이너리 항목에 유효한 name, platform, kind, 파일 경로가 있는지 확인한다.
    declared_binaries = {}
    for b in manifest.get("binaries", []):
        name = b.get("name", "")

        if name not in VALID_BINARY_NAMES:
            error(f"Unknown binary name: {name}")
        if b.get("platform") not in VALID_PLATFORMS:
            error(f"Binary {name}: invalid platform '{b.get('platform')}'")
        if b.get("kind") not in VALID_KINDS:
            error(f"Binary {name}: invalid kind '{b.get('kind')}'")

        # runtime_file: 자동 생성되는 C 런타임 등록 파일 경로
        # expected_file: 자동 생성되는 정답 JSON 파일 경로
        for field in ("runtime_file", "expected_file"):
            if not b.get(field):
                error(f"Binary {name}: missing field '{field}'")

        declared_binaries[name] = b

    # ── cases 섹션 검증 ───────────────────────────────────────────────
    seen_ids = {}
    for case in manifest.get("cases", []):
        cid = case.get("id", "")

        # ID 형식 검사: DFB + 3자리 숫자
        if not ID_PATTERN.match(cid):
            error(f"Invalid case ID format: '{cid}'")

        # ID 중복 검사
        if cid in seen_ids:
            error(f"Duplicate case ID: {cid}")
        seen_ids[cid] = True

        # binary 이름이 binaries 섹션에 선언되어 있는지 확인
        binary = case.get("binary", "")
        if binary not in declared_binaries:
            error(f"{cid}: binary '{binary}' not in binaries list")

        # source_file 경로 확인 (아직 파일이 없을 수 있으므로 WARN만)
        src = case.get("source_file", "")
        if not src:
            error(f"{cid}: missing source_file")
        elif not (ROOT / src).exists():
            print(f"[WARN] {cid}: source_file not found yet: {src}")

        # function 이름 형식 검사
        fn = case.get("function", "")
        if not FUNCTION_PATTERN.match(fn):
            error(f"{cid}: invalid function name '{fn}'")

        # anchor: backward slice의 시작점 정의
        #   callee    — 추적을 시작할 함수 호출 이름 (예: dfb_sink_int)
        #   arg_index — 그 함수의 몇 번째 인자에서 시작할지 (0-based)
        anchor = case.get("anchor", {})
        if not anchor.get("callee"):
            error(f"{cid}: anchor missing callee")
        if not isinstance(anchor.get("arg_index"), int):
            error(f"{cid}: anchor.arg_index must be int")

        # 필수 리스트 필드 타입 확인
        for field in ("expected_sources", "forbidden_sources",
                      "expected_features", "allowed_warnings"):
            if not isinstance(case.get(field), list):
                error(f"{cid}: '{field}' must be a list")

    # ── 결과 출력 ─────────────────────────────────────────────────────
    if ERRORS:
        print(f"\n{len(ERRORS)} error(s) found.", file=sys.stderr)
        return 1

    print(f"[OK] Manifest valid. "
          f"{len(seen_ids)} cases across {len(declared_binaries)} binaries.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
