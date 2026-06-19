# DataFlowBench 사용 및 개발 가이드

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [저장소 구조](#2-저장소-구조)
3. [빌드 방법](#3-빌드-방법)
4. [실행 파일 사용법](#4-실행-파일-사용법)
5. [정답지(Expected JSON) 읽는 법](#5-정답지expected-json-읽는-법)
6. [Manifest 읽는 법](#6-manifest-읽는-법)
7. [케이스 목록 및 카테고리](#7-케이스-목록-및-카테고리)
8. [새 케이스 추가 방법](#8-새-케이스-추가-방법)
9. [생성 파이프라인](#9-생성-파이프라인)
10. [플랫폼별 주의사항](#10-플랫폼별-주의사항)
11. [PASS / FAIL / WARN 판정 기준](#11-pass--fail--warn-판정-기준)

---

## 1. 프로젝트 개요

DataFlowBench는 **"정답이 알려진 테스트 바이너리"** 를 생성해, Ghidra High PCode 기반 Backward Slice 엔진이 올바르게 동작하는지 검증하는 테스트베드다.

```
[DataFlowBench]                 [외부 프로젝트 — 이 저장소 범위 외]
  케이스 구현 (.c/.cpp)   →  바이너리 빌드  →  Ghidra 분석
  정답 메타데이터                            →  결과 비교 (PASS/FAIL/WARN)
  (expected JSON)
```

### 핵심 원칙

- `manifests/cases_manifest.json` 이 유일한 정답 원본이다.
- 런타임 레지스트리 C 파일과 expected JSON은 이 매니페스트로부터 자동 생성된다.
- **생성된 파일은 절대 수동 편집하지 않는다.**

---

## 2. 저장소 구조

```
dataflow-bench/
├── README.md
├── CMakeLists.txt              빌드 시스템
├── CMakePresets.json           빌드 프리셋 (win-debug, linux-native-debug 등)
│
├── include/
│   ├── dfbench.h               매크로 정의 (DFB_CASE, DFB_SOURCE, DFB_SINK 등)
│   ├── dfbench_cases.h         케이스 엔트리 구조체 및 dfb_get_cases() 선언
│   └── dfbench_sources_sinks.h source/sink 함수 선언
│
├── src/
│   ├── main.c                  CLI 진입점 (--list / --run-all / --run)
│   ├── dfbench_sources_sinks.c source/sink 함수 구현
│   │
│   ├── cases_basic.c           DFB001-007
│   ├── cases_control_flow.c    DFB010-016
│   ├── cases_stack.c           DFB020-023
│   ├── cases_global.c          DFB024-027
│   ├── cases_heap.c            DFB030-033
│   ├── cases_struct.c          DFB040-042, DFB046
│   ├── cases_array.c           DFB043-045
│   ├── cases_interproc.c       DFB050-060, DFB066
│   ├── cases_indirect.c        DFB070-075
│   ├── cases_recursion.c       DFB061-065
│   ├── cases_runtime_bridge.c  DFB091 (TLS), DFB090/DFB092 (POSIX only)
│   ├── cases_abi.c             DFB100-102, DFB151-152
│   ├── cases_exceptional.c     DFB110
│   ├── cases_memory_api.c      DFB120-123
│   ├── cases_import.c          DFB130-131
│   │
│   └── [generated]
│       ├── dfbench_runtime_win_core.c      ← generate_registry_from_manifest.py 생성
│       ├── dfbench_runtime_posix.c
│       ├── dfbench_runtime_cpp.c
│       └── dfbench_runtime_cpp_exceptions.c
│
├── cpp/
│   ├── cases_cpp.cpp           DFB080-081
│   └── cases_cpp_exceptions.cpp DFB111
│
├── importlib/
│   ├── dfbench_importlib.h     공유 라이브러리 헤더 (DLL export/import 매크로)
│   └── dfbench_importlib.c     dfb_import_identity, dfb_import_write_out, dfb_get_opaque_fn, dfb_external_no_summary 구현
│
├── manifests/
│   └── cases_manifest.json     ★ 유일한 정답 원본 — 모든 케이스 정의
│
├── expected/                   [generated] generate_expected_from_manifest.py 생성
│   ├── dfbench_win_core.expected.json
│   ├── dfbench_posix_runtime.expected.json
│   ├── dfbench_cpp.expected.json
│   └── dfbench_cpp_exceptions.expected.json
│
├── tools/
│   ├── validate_manifest.py              매니페스트 스키마 검증
│   ├── generate_registry_from_manifest.py 런타임 C 파일 생성
│   └── generate_expected_from_manifest.py expected JSON 생성
│
├── toolchains/
│   ├── mingw-w64-x64.cmake     Linux→Windows PE 크로스컴파일 툴체인
│   └── linux-native.cmake      Linux 네이티브 빌드 툴체인
│
├── scripts/
│   ├── build_all.ps1 / .sh     전체 빌드 스크립트
│   └── run_all.ps1 / .sh       전체 실행 스크립트
│
└── tests/
    └── smoke_test.py           빌드된 실행파일 기본 동작 검증
```

---

## 3. 빌드 방법

### 사전 준비

| 환경 | 필요 도구 |
|---|---|
| Windows | CMake 3.20+, Ninja, MinGW-w64 또는 MSVC, Python 3.8+ |
| Linux (전체) | CMake 3.20+, Ninja, gcc, g++, mingw-w64, Python 3.8+ |

### Windows — 네이티브 빌드

```powershell
# Debug (분석에 권장 — noinline 강제)
cmake --preset win-debug
cmake --build --preset win-debug

# Release (noinline 유지, 최적화 적용)
cmake --preset win-release
cmake --build --preset win-release
```

산출물:
```
build/win-debug/
├── dfbench_win_core.exe        Windows 핵심 케이스 75개
├── dfbench_cpp.exe             C++ 케이스 2개
├── dfbench_cpp_exceptions.exe  C++ 예외 케이스 1개
└── libdfbench_importlib.dll    공유 라이브러리
```

### Linux — 전체 케이스 빌드

```bash
# Windows PE 크로스컴파일 (mingw-w64)
cmake --preset win-cross-debug
cmake --build --preset win-cross-debug

# Linux 네이티브 (POSIX 스레드 케이스용)
cmake --preset linux-native-debug
cmake --build --preset linux-native-debug
```

산출물:
```
build/win-cross-debug/      ← Ghidra에서 PE로 분석
    dfbench_win_core.exe
    dfbench_cpp.exe
    dfbench_cpp_exceptions.exe

build/linux-native-debug/   ← Ghidra에서 ELF로 분석
    dfbench_posix_runtime   ← DFB090, DFB092 (POSIX 전용)
```

---

## 4. 실행 파일 사용법

각 실행파일은 동일한 CLI 인터페이스를 제공한다.

```bash
# 포함된 케이스 목록 출력
./dfbench_win_core.exe --list

# 모든 케이스 실행 (크래시 없으면 OK 출력)
./dfbench_win_core.exe --run-all

# 특정 케이스 하나만 실행
./dfbench_win_core.exe --run DFB040
```

> **주의**: 실행 결과는 런타임 sanity check 용도다.
> 실제 데이터 흐름 분석은 Ghidra에서 외부 BackwardSlicer가 수행한다.

---

## 5. 정답지(Expected JSON) 읽는 법

`expected/dfbench_win_core.expected.json` 예시:

```json
{
  "schema_version": 1,
  "program": "dfbench_win_core",
  "generated_from": "manifests/cases_manifest.json",
  "cases": [
    {
      "id": "DFB040",
      "binary": "dfbench_win_core",
      "function": "case_DFB040_struct_field_precise",
      "anchor": {
        "callee": "dfb_sink_int",
        "arg_index": 0
      },
      "expected_sources": ["dfb_source_B.ret"],
      "forbidden_sources": ["dfb_source_A.ret"],
      "expected_features": ["struct", "field_sensitivity"],
      "allowed_warnings": []
    }
  ]
}
```

| 필드 | 의미 |
|---|---|
| `anchor.callee` | Backward Slice를 시작하는 sink 함수 이름 |
| `anchor.arg_index` | slice를 시작할 인자 번호 (0-indexed) |
| `expected_sources` | slice 결과에 **반드시 포함**되어야 할 source 목록 |
| `forbidden_sources` | slice 결과에 **절대 포함되면 안** 되는 source 목록 (precision 검증) |
| `expected_features` | 분석기가 처리해야 하는 기능 태그 |
| `allowed_warnings` | 허용되는 WARN 태그 (간접호출 미해석 등) |

### Ghidra 사용 예

1. `dfbench_win_core.exe`를 Ghidra에 import
2. `case_DFB040_struct_field_precise` 함수로 이동
3. `dfb_sink_int` 호출의 0번째 인자에서 Backward Slice 시작
4. 결과에 `dfb_source_B` 포함 여부 → PASS / FAIL

---

## 6. Manifest 읽는 법

`manifests/cases_manifest.json`이 모든 정보의 원본이다.

```json
{
  "schema_version": 1,
  "binaries": [
    {
      "name": "dfbench_win_core",
      "platform": "windows",
      "kind": "core_c",
      "runtime_file": "src/dfbench_runtime_win_core.c",
      "expected_file": "expected/dfbench_win_core.expected.json"
    }
  ],
  "cases": [
    {
      "id": "DFB040",
      "name": "struct_field_precise",
      "binary": "dfbench_win_core",
      "source_file": "src/cases_struct.c",
      "function": "case_DFB040_struct_field_precise",
      "anchor": {"callee": "dfb_sink_int", "arg_index": 0},
      "expected_sources": ["dfb_source_B.ret"],
      "forbidden_sources": ["dfb_source_A.ret"],
      "expected_features": ["struct", "field_sensitivity"],
      "allowed_warnings": []
    }
  ]
}
```

매니페스트 수정 후에는 반드시 생성 파이프라인을 다시 실행한다. [9번 항목](#9-생성-파이프라인) 참조.

---

## 7. 케이스 목록 및 카테고리

### dfbench_win_core (75개)

| ID 범위 | 파일 | 테스트 주제 |
|---|---|---|
| DFB001–007 | cases_basic.c | 직접 값, 산술, 타입 캐스트, true-negative, kill, 멀티 소스, 서브레지스터 |
| DFB010–016 | cases_control_flow.c | Branch/Loop/Switch PHI, 루프 widening, 제어 의존성, 메모리 PHI |
| DFB020–023 | cases_stack.c | 스택 로컬, out-param, 이중 포인터 |
| DFB024–027 | cases_global.c | 전역 변수 흐름, 필드 정밀도, 읽기 전용 전역 |
| DFB030–033 | cases_heap.c | 힙 필드, realloc 후 보존, 힙 포인터 별칭 |
| DFB040–042, 046 | cases_struct.c | 구조체 필드, 포인터 산술, 유니온, 부분 덮어쓰기 |
| DFB043–045 | cases_array.c | 배열 상수/변수 인덱스, 중첩 집합체 |
| DFB050–060, 066 | cases_interproc.c | 인터프로시저: identity, 중첩 호출, 컨텍스트 민감성, sret, 함수 요약, swap |
| DFB061–065 | cases_recursion.c | 재귀: 비꼬리, 상호 재귀 SCC, 전역 사이드 이펙트, 간접 재귀, 트리 재귀 |
| DFB070–075 | cases_indirect.c | 함수 포인터, 콜백, FP 테이블, 간접 싱크, CALLIND 미해석, 외부 경계 |
| DFB091 | cases_runtime_bridge.c | TLS (Thread-Local Storage) |
| DFB100–102, 151–152 | cases_abi.c | varargs, tail call, 부호/비부호 경계, 선택적 인자 통과, callee 내부 소스 |
| DFB110 | cases_exceptional.c | setjmp/longjmp |
| DFB120–123 | cases_memory_api.c | memcpy, memmove, strcpy, memset+memcpy |
| DFB130–131 | cases_import.c | 공유 라이브러리 import (arg→ret, out-param) |

### dfbench_posix_runtime (2개) — Linux only

| ID | 테스트 주제 |
|---|---|
| DFB090 | pthread 스레드 공유 메모리 |
| DFB092 | pthread + 함수 포인터 테이블 디스패치 |

### dfbench_cpp (2개)

| ID | 테스트 주제 |
|---|---|
| DFB080 | C++ 가상 함수 호출 (vtable dispatch) |
| DFB081 | C++ 람다 캡처 |

### dfbench_cpp_exceptions (1개)

| ID | 테스트 주제 |
|---|---|
| DFB111 | C++ 예외 흐름 (throw/catch) |

---

## 8. 새 케이스 추가 방법

### 1단계 — 케이스 코드 작성

적절한 `src/cases_*.c` 파일에 케이스 함수를 추가한다.

```c
// src/cases_basic.c 예시
DFB_CASE void case_DFB004_my_new_case(void) {
    int a = dfb_source_A();
    int b = my_transform(a);
    dfb_sink_int(b);
}
```

규칙:
- 함수명: `case_DFBxxx_<설명>` 형식
- 반드시 `DFB_CASE` 매크로 적용
- source는 `dfb_source_A/B/C()` 사용
- sink는 `dfb_sink_int/long/ptr/buf()` 사용

### 2단계 — Manifest에 케이스 추가

`manifests/cases_manifest.json`의 `cases` 배열에 항목 추가:

```json
{
    "id": "DFB004",
    "name": "my_new_case",
    "binary": "dfbench_win_core",
    "source_file": "src/cases_basic.c",
    "function": "case_DFB004_my_new_case",
    "anchor": {"callee": "dfb_sink_int", "arg_index": 0},
    "expected_sources": ["dfb_source_A.ret"],
    "forbidden_sources": [],
    "expected_features": ["my_feature"],
    "allowed_warnings": [],
    "required_warnings": []
}
```

### 3단계 — 생성 파이프라인 실행

```bash
python tools/validate_manifest.py
python tools/generate_registry_from_manifest.py
python tools/generate_expected_from_manifest.py
```

### 4단계 — 빌드 및 확인

```bash
cmake --build --preset win-debug
./build/win-debug/dfbench_win_core.exe --run DFB004   # → OK DFB004
```

---

## 9. 생성 파이프라인

```
manifests/cases_manifest.json  (수동 편집 대상)
           │
           ├─→ validate_manifest.py
           │       스키마, 중복 ID, binary 소속, 함수명 형식 검증
           │
           ├─→ generate_registry_from_manifest.py
           │       src/dfbench_runtime_*.c  (빌드 대상, 수동 편집 금지)
           │
           └─→ generate_expected_from_manifest.py
                   expected/*.expected.json  (정답지, 수동 편집 금지)
```

### 수동 편집 대상 vs 생성 파일

| 구분 | 파일 |
|---|---|
| **수동 편집** | `manifests/cases_manifest.json`, `src/cases_*.c`, `cpp/cases_*.cpp`, `include/*.h`, `CMakeLists.txt`, `tools/*.py` |
| **생성 (편집 금지)** | `src/dfbench_runtime_*.c`, `expected/*.expected.json` |

---

## 10. 플랫폼별 주의사항

### 실행파일 - 플랫폼 소속

| 실행파일 | 빌드 환경 | 포함 케이스 |
|---|---|---|
| dfbench_win_core.exe | Windows / Linux 크로스 | DFB001-075, DFB091, DFB100-102, DFB130-131, DFB151-152 |
| dfbench_cpp.exe | Windows / Linux 크로스 | DFB080, DFB081 |
| dfbench_cpp_exceptions.exe | Windows / Linux 크로스 | DFB111 |
| dfbench_posix_runtime | Linux 네이티브만 | DFB090, DFB092 |

### 분석기 개발 시 플랫폼별 차이

| 케이스 | 주의 사항 |
|---|---|
| DFB091 (TLS) | Windows: `fs:` 세그먼트, Linux: `gs:` 세그먼트로 TLS 접근 패턴이 다름 |
| DFB100 (varargs) | Windows x64: shadow space, Linux x64: 레지스터 저장 영역 |
| DFB053 (sret) | Windows: 8바이트 초과 시 hidden sret, Linux: 16바이트 초과 시 |
| DFB111 (C++ 예외) | Windows: SEH, Linux GCC: SJLJ/Dwarf — unwinder 패턴 다름 |
| DFB130-131 (import) | LTO 빌드 시 import boundary 소멸 → 이 케이스의 의미 희석 |

---

## 11. PASS / FAIL / WARN 판정 기준

DataFlowBench 자체는 판정을 수행하지 않는다. 외부 비교 엔진이 아래 기준을 따른다.

### PASS
- `expected_sources`에 나열된 모든 source가 분석 결과에 존재
- `forbidden_sources`에 나열된 source가 분석 결과에 없음
- 분석기 크래시/타임아웃 없음

### FAIL
- `expected_sources` 중 하나라도 누락
- `forbidden_sources` 중 하나라도 발견 (precision 실패)
- anchor sink 함수 미발견
- 케이스 함수가 바이너리 심볼 테이블에 없음
- 분석기 크래시

### WARN

WARN은 두 가지 역할을 한다.

- **`allowed_warnings`** 에 나열된 태그: 발생하면 FAIL → PASS로 완화
- **`required_warnings`** 에 나열된 태그: 반드시 발생해야 PASS (누락 시 FAIL)

| 태그 | 의미 |
|---|---|
| `unresolved_indirect` | 간접 호출 타겟 미해석, 하지만 source taint가 sink까지 도달함 |
| `unresolved_virtual` | 가상 함수 타겟 미해석, 하지만 taint 보존됨 |
| `unresolved_varargs` | varargs 내부 미해석, 하지만 caller 인자 source 보존됨 |
| `unresolved_nonlocal_jump` | setjmp/longjmp 흐름 미해석, 전역 경유로 taint 보존됨 |
| `unresolved_thread_dispatch` | 스레드 디스패치 미해석, source taint 보존됨 |
| `variable_index_conservative` | 변수 인덱스 배열 보수적 merge, source taint 보존됨 |
| `unresolved_exception_flow` | 예외 흐름 미해석, 전역 경유로 taint 보존됨 |
| `widened_unknown` | 루프 반복 중 값이 widened_unknown으로 확장됨, 진입 taint 보존됨 |
| `recursive_summary_unavailable` | 재귀 함수 요약 미완성 (fixed-point 미수렴), 보수적으로 taint 보존됨 |
| `unresolved_indirect_call` | 함수 포인터 타겟 정적 미해석, taint propagation 불확실 (DFB074) |
| `unresolved_call_boundary` | 외부 라이브러리 경계에서 요약 없음, taint propagation 불확실 (DFB075) |

### SKIP
- 해당 플랫폼에서 빌드되지 않은 실행파일 (예: Windows에서 dfbench_posix_runtime)
- 명시적으로 제외된 빌드 구성
