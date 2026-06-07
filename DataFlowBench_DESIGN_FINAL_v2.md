# DataFlowBench 개발 설계서

## 0. 문서 목적

이 문서는 **DataFlowBench 테스트베드 프로그램**을 실제로 구현하기 위한 상세 개발 문서이다.

DataFlowBench의 목적은 Ghidra High PCode 기반 Backward Slice / Data Flow Trace 엔진을 검증하기 위한 **정답이 있는 테스트 바이너리**를 만드는 것이다.

이 문서의 범위는 **테스트베드 프로그램 개발**에만 한정한다.

포함하는 것:

- C/C++ 테스트 케이스 프로그램
- Source/Sink 함수
- 케이스별 함수 구조
- 함수별 input/output 관계를 분리해 검증할 수 있는 summary-friendly 함수 케이스
- direct-trace 전용 케이스와 function-summary-ready 케이스의 명시적 구분
- 빌드 시스템
- expected JSON
- 실행 방식
- CI에서 비교 가능한 결과 구조

포함하지 않는 것:

- BackwardSlicer 엔진 구현
- Ghidra High PCode 분석기 구현
- 함수 요약 DB
- SQLite 저장소
- 분석 결과 저장 DB
- LLM 분류기
- APK/SO 도메인 규칙

중요한 구현 금지 사항:

```text
이 저장소는 테스트 바이너리와 expected metadata만 생성한다.
Codex는 이 저장소 안에 BackwardSlicer, Ghidra 분석기, 함수 요약 DB, SQLite 기반 결과 저장소, 비교기 엔진을 구현하지 않는다.
PASS/FAIL/WARN 판정 규칙은 문서화만 하며, 실제 data-flow 분석은 외부 프로젝트가 수행한다.
```

---

## 1. 최종 구현 방향

DataFlowBench는 **Windows를 기본 비교 플랫폼으로 하는 다중 실행파일 방식**으로 구현한다.

기본 원칙:

- 기본 산출물은 Windows에서 빌드/실행/비교 가능한 실행파일이다.
- 플랫폼 의존성 때문에 의미가 달라지는 케이스는 fallback으로 우회하지 않고 별도 실행파일로 분리한다.
- C++/예외/버전/ABI 의존성 때문에 의미가 달라지는 케이스도 별도 실행파일로 분리한다.
- 동일한 DFB ID는 하나의 실행파일 안에서만 정의하며, 플랫폼에 따라 서로 다른 의미로 재사용하지 않는다.

### 1.1 선택 이유

다중 실행파일 방식의 장점:

- Windows를 기준 플랫폼으로 고정할 수 있다.
- 플랫폼에 따라 의미가 달라지는 fallback을 제거할 수 있다.
- C core, POSIX runtime, C++ 예외 케이스를 독립적으로 관리할 수 있다.
- 실행파일별 expected metadata를 분리해 비교 기준이 명확해진다.
- 특정 도메인만 재빌드하거나 재분석하기 쉽다.

단점:

- 빌드 타깃 수가 늘어난다.
- registry와 expected metadata를 자동 생성하지 않으면 관리 비용이 커진다.
- 사용자 문서에서 어떤 실행파일이 어떤 케이스를 담는지 명확히 설명해야 한다.

대응:

- 모든 케이스는 `case_DFBxxx_*` 이름으로 export한다.
- 각 실행파일은 자기에게 속한 케이스만 registry에 등록한다.
- source/sink는 `noinline`, `used`, `visibility(default)`를 적용한다.
- 전역 sink 변수는 `volatile`로 둔다.
- 플랫폼/언어 의존 케이스는 별도 타깃으로 분리하고, 의미가 달라지는 fallback 구현은 허용하지 않는다.
- registry와 expected metadata는 공통 manifest에서 생성한다.

### 1.2 산출물

최종적으로 다음 실행 파일들이 만들어져야 한다.

```text
dfbench_win_core
dfbench_posix_runtime
dfbench_cpp
dfbench_cpp_exceptions
```

필요 시 ABI/컴파일러 의존 케이스를 위해 추가 실행파일을 둘 수 있다.

지원 옵션:

```bash
./dfbench_win_core --list
./dfbench_win_core --run-all
./dfbench_win_core --run DFB001
```

`--list`는 케이스 목록을 출력한다.

`--run-all`은 모든 케이스를 실행한다.

`--run DFB001`은 특정 케이스 하나를 실행한다.

주의:

이 실행 결과는 데이터 흐름 분석 결과가 아니다.  
단지 바이너리가 정상 실행 가능한지 확인하기 위한 runtime sanity check이다.

실제 backward slice 검증은 외부 분석기가 실행파일별 expected metadata와 비교한다.

예:

```text
dfbench_win_core            <-> expected/dfbench_win_core.expected.json
dfbench_posix_runtime       <-> expected/dfbench_posix_runtime.expected.json
dfbench_cpp                 <-> expected/dfbench_cpp.expected.json
dfbench_cpp_exceptions      <-> expected/dfbench_cpp_exceptions.expected.json
```

### 1.3 플랫폼 및 버전 정책

정책은 다음과 같다.

1. 기본 기준 플랫폼은 Windows이다.
2. 기본 비교 대상 바이너리는 Windows에서 생성되는 `dfbench_win_core`이다.
3. 구현 특성상 다른 플랫폼에서만 의미를 보존할 수 있는 케이스는 별도 빌드 및 별도 실행파일로 분리한다.
4. 플랫폼 의존성이 아닌 C++, 예외 처리, ABI, 컴파일러/버전 차이 때문에 의미가 달라지는 케이스도 별도 실행파일로 분리한다.
5. 동일한 DFB ID는 플랫폼에 따라 서로 다른 의미로 재사용하지 않는다.
6. 의미가 달라지는 fallback 구현은 금지한다.
7. 지원 불가한 조합은 SKIP 처리하며, 다른 의미의 대체 구현으로 우회하지 않는다.

---

## 2. 저장소 구조

Codex는 아래 구조로 프로젝트를 생성해야 한다.

```text
dataflow-bench/
├── README.md
├── DESIGN.md
├── CMakeLists.txt
├── CMakePresets.json
│
├── include/
│   ├── dfbench.h
│   ├── dfbench_cases.h
│   └── dfbench_sources_sinks.h
│
├── src/
│   ├── main.c
│   ├── dfbench_runtime_win_core.c
│   ├── dfbench_runtime_posix.c
│   ├── dfbench_runtime_cpp.c
│   ├── dfbench_runtime_cpp_exceptions.c
│   ├── dfbench_sources_sinks.c
│   ├── cases_basic.c
│   ├── cases_control_flow.c
│   ├── cases_stack.c
│   ├── cases_heap.c
│   ├── cases_struct.c
│   ├── cases_array.c
│   ├── cases_global.c
│   ├── cases_interproc.c
│   ├── cases_indirect.c
│   ├── cases_memory_api.c
│   ├── cases_abi.c
│   ├── cases_exceptional.c
│   ├── cases_runtime_bridge.c
│   └── cases_import.c
│
├── manifests/
│   └── cases_manifest.json
│
├── importlib/
│   ├── dfbench_importlib.h
│   └── dfbench_importlib.c
│
├── cpp/
│   ├── cases_cpp.cpp
│   └── cases_cpp_exceptions.cpp
│
├── expected/
│   ├── dfbench_win_core.expected.json
│   ├── dfbench_posix_runtime.expected.json
│   ├── dfbench_cpp.expected.json
│   └── dfbench_cpp_exceptions.expected.json
│
├── scripts/
│   ├── build_all.sh
│   ├── build_all.ps1
│   ├── run_all.sh
│   ├── run_all.ps1
│   └── generate_expected_template.py
│
├── docs/
│   ├── DataFlowBench_Design_FINAL_V2.md <- (this file)
│   └── read_output_manual.md
└── tests/
    └── smoke_test.py
```

---

## 3. 공통 코딩 규칙

### 3.1 함수 이름 규칙

모든 케이스 함수는 다음 형식을 따른다.

```c
case_DFB001_direct_value
case_DFB002_arithmetic_value
case_DFB010_branch_phi
```

규칙:

```text
case_<ID>_<short_description>
```

ID는 고정된 3자리 번호를 사용한다.

예:

```text
DFB001
DFB010
DFB100
```

### 3.2 Source 함수 이름 규칙

```c
dfb_source_A()
dfb_source_B()
dfb_source_C()
dfb_source_ptr_A()
dfb_source_buf_A()
```

Source는 분석상 데이터의 시작점이다.

### 3.3 Sink 함수 이름 규칙

```c
dfb_sink_int(int x)
dfb_sink_long(long x)
dfb_sink_ptr(void *p)
dfb_sink_buf(char *p)
```

Sink는 backward slice anchor로 사용된다.

### 3.4 최적화 방지

모든 source, sink, case 함수는 가능한 한 제거되지 않아야 한다.

공통 매크로는 `include/dfbench.h`에 정의한다.

```c
#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
    #define DFB_NOINLINE __declspec(noinline)
    #define DFB_EXPORT __declspec(dllexport)
    #define DFB_USED
#else
    #define DFB_NOINLINE __attribute__((noinline))
    #define DFB_EXPORT __attribute__((visibility("default")))
    #define DFB_USED __attribute__((used))
#endif

#define DFB_CASE DFB_EXPORT DFB_NOINLINE DFB_USED
#define DFB_SOURCE DFB_EXPORT DFB_NOINLINE DFB_USED
#define DFB_SINK DFB_EXPORT DFB_NOINLINE DFB_USED

#if defined(__cplusplus)
    #define DFB_EXTERN_C extern "C"
#else
    #define DFB_EXTERN_C
#endif

#define DFB_TOUCH_INT(x) do { g_dfb_sink_int ^= (int)(x); } while (0)
#define DFB_TOUCH_LONG(x) do { g_dfb_sink_long ^= (long)(x); } while (0)
#define DFB_TOUCH_PTR(p) do { g_dfb_sink_ptr = (uintptr_t)(p); } while (0)
```

전역 변수:

```c
extern volatile int g_dfb_sink_int;
extern volatile long g_dfb_sink_long;
extern volatile uintptr_t g_dfb_sink_ptr;
extern volatile int g_dfb_source_seed;
```

---

## 4. 공통 Source / Sink 구현

파일: `include/dfbench_sources_sinks.h`

```c
#pragma once
#include "dfbench.h"

DFB_EXTERN_C extern volatile int g_dfb_sink_int;
DFB_EXTERN_C extern volatile long g_dfb_sink_long;
DFB_EXTERN_C extern volatile uintptr_t g_dfb_sink_ptr;
DFB_EXTERN_C extern volatile int g_dfb_source_seed;

DFB_EXTERN_C DFB_SOURCE int dfb_source_A(void);
DFB_EXTERN_C DFB_SOURCE int dfb_source_B(void);
DFB_EXTERN_C DFB_SOURCE int dfb_source_C(void);
DFB_EXTERN_C DFB_SOURCE long dfb_source_long_A(void);
DFB_EXTERN_C DFB_SOURCE char *dfb_source_buf_A(void);

DFB_EXTERN_C DFB_SINK void dfb_sink_int(int x);
DFB_EXTERN_C DFB_SINK void dfb_sink_long(long x);
DFB_EXTERN_C DFB_SINK void dfb_sink_ptr(void *p);
DFB_EXTERN_C DFB_SINK void dfb_sink_buf(char *p);
```

파일: `src/dfbench_sources_sinks.c`

```c
#include "dfbench_sources_sinks.h"

volatile int g_dfb_sink_int = 0;
volatile long g_dfb_sink_long = 0;
volatile uintptr_t g_dfb_sink_ptr = 0;
volatile int g_dfb_source_seed = 0x12345678;

static char g_dfb_source_buf[64] = {0};

DFB_SOURCE int dfb_source_A(void) {
    return g_dfb_source_seed + 1;
}

DFB_SOURCE int dfb_source_B(void) {
    return g_dfb_source_seed + 2;
}

DFB_SOURCE int dfb_source_C(void) {
    return g_dfb_source_seed + 3;
}

DFB_SOURCE long dfb_source_long_A(void) {
    return (long)g_dfb_source_seed + 0x1000L;
}

DFB_SOURCE char *dfb_source_buf_A(void) {
    g_dfb_source_buf[0] = (char)(g_dfb_source_seed & 0xff);
    return g_dfb_source_buf;
}

DFB_SINK void dfb_sink_int(int x) {
    DFB_TOUCH_INT(x);
}

DFB_SINK void dfb_sink_long(long x) {
    DFB_TOUCH_LONG(x);
}

DFB_SINK void dfb_sink_ptr(void *p) {
    DFB_TOUCH_PTR(p);
}

DFB_SINK void dfb_sink_buf(char *p) {
    if (p != 0) {
        DFB_TOUCH_INT((int)p[0]);
    }
}
```

---

## 5. 케이스 등록 방식

케이스 registry는 실행파일별로 분리한다.

중요 원칙:

- 각 실행파일은 자신에게 속한 케이스만 `g_cases[]`에 등록한다.
- `g_cases[]`는 수작업 정답 원본이 아니라, 공통 manifest에서 생성되는 실행용 registry이다.
- 동일한 DFB ID는 두 개 이상의 runtime registry에 중복 등록하지 않는다.
- `--list` 결과와 실행파일별 expected metadata는 동일한 케이스 집합을 가져야 한다.

파일: `include/dfbench_cases.h`

```c
#pragma once
#include "dfbench.h"

typedef void (*dfb_case_fn_t)(void);

typedef struct dfb_case_entry_t {
    const char *id;
    const char *name;
    dfb_case_fn_t fn;
} dfb_case_entry_t;

DFB_EXTERN_C const dfb_case_entry_t *dfb_get_cases(size_t *count);
```

파일: `src/dfbench_runtime_win_core.c`

아래는 Windows 기본 비교 플랫폼용 registry 예시다.

```c
#include <stddef.h>
#include "dfbench_cases.h"

/* case declarations */
DFB_EXTERN_C void case_DFB001_direct_value(void);
DFB_EXTERN_C void case_DFB002_arithmetic_value(void);
DFB_EXTERN_C void case_DFB003_cast_value(void);

DFB_EXTERN_C void case_DFB010_branch_phi(void);
DFB_EXTERN_C void case_DFB011_loop_phi(void);
DFB_EXTERN_C void case_DFB012_switch_merge(void);

DFB_EXTERN_C void case_DFB020_stack_local(void);
DFB_EXTERN_C void case_DFB021_stack_outparam(void);
DFB_EXTERN_C void case_DFB022_arg_to_outparam(void);
DFB_EXTERN_C void case_DFB023_double_pointer_outparam(void);
DFB_EXTERN_C void case_DFB024_global_value_flow(void);
DFB_EXTERN_C void case_DFB025_global_field_precise(void);
DFB_EXTERN_C void case_DFB026_global_interproc_reader(void);

DFB_EXTERN_C void case_DFB030_heap_field(void);
DFB_EXTERN_C void case_DFB031_heap_realloc_preserve(void);

DFB_EXTERN_C void case_DFB040_struct_field_precise(void);
DFB_EXTERN_C void case_DFB041_pointer_arithmetic_field(void);
DFB_EXTERN_C void case_DFB042_union_alias(void);
DFB_EXTERN_C void case_DFB043_array_constant_index(void);
DFB_EXTERN_C void case_DFB044_array_variable_index(void);
DFB_EXTERN_C void case_DFB045_nested_aggregate_field(void);
DFB_EXTERN_C void case_DFB046_partial_overwrite_subfield(void);

DFB_EXTERN_C void case_DFB050_identity_call(void);
DFB_EXTERN_C void case_DFB051_nested_call(void);
DFB_EXTERN_C void case_DFB052_callsite_context(void);
DFB_EXTERN_C void case_DFB053_large_struct_return(void);
DFB_EXTERN_C void case_DFB054_status_outparam(void);
DFB_EXTERN_C void case_DFB055_deep_field_passthrough(void);
DFB_EXTERN_C void case_DFB056_arg_to_ret_summary(void);
DFB_EXTERN_C void case_DFB057_struct_field_to_ret_summary(void);
DFB_EXTERN_C void case_DFB058_arg_to_outparam_summary(void);
DFB_EXTERN_C void case_DFB059_inout_field_update_summary(void);

DFB_EXTERN_C void case_DFB060_recursion(void);

DFB_EXTERN_C void case_DFB070_function_pointer(void);
DFB_EXTERN_C void case_DFB071_callback_registration(void);
DFB_EXTERN_C void case_DFB072_function_pointer_table(void);
DFB_EXTERN_C void case_DFB073_indirect_sink_wrapper(void);

DFB_EXTERN_C void case_DFB080_cpp_virtual_call(void);
DFB_EXTERN_C void case_DFB081_cpp_lambda_capture(void);

DFB_EXTERN_C void case_DFB091_tls_value(void);
DFB_EXTERN_C void case_DFB092_pthread_table_dispatch(void);

DFB_EXTERN_C void case_DFB100_varargs(void);
DFB_EXTERN_C void case_DFB101_tail_call_candidate(void);
DFB_EXTERN_C void case_DFB102_signed_unsigned_boundary(void);

DFB_EXTERN_C void case_DFB110_setjmp_longjmp(void);
DFB_EXTERN_C void case_DFB111_cpp_exception_flow(void);

DFB_EXTERN_C void case_DFB120_memcpy_buffer(void);
DFB_EXTERN_C void case_DFB121_memmove_buffer(void);
DFB_EXTERN_C void case_DFB122_strcpy_buffer(void);
DFB_EXTERN_C void case_DFB123_memset_partial_memcpy(void);

DFB_EXTERN_C void case_DFB130_shared_import_arg_to_ret(void);
DFB_EXTERN_C void case_DFB131_shared_import_outparam(void);

static const dfb_case_entry_t g_cases[] = {
    {"DFB001", "direct_value", case_DFB001_direct_value},
    {"DFB002", "arithmetic_value", case_DFB002_arithmetic_value},
    {"DFB003", "cast_value", case_DFB003_cast_value},

    {"DFB010", "branch_phi", case_DFB010_branch_phi},
    {"DFB011", "loop_phi", case_DFB011_loop_phi},
    {"DFB012", "switch_merge", case_DFB012_switch_merge},

    {"DFB020", "stack_local", case_DFB020_stack_local},
    {"DFB021", "stack_outparam", case_DFB021_stack_outparam},
    {"DFB022", "arg_to_outparam", case_DFB022_arg_to_outparam},
    {"DFB023", "double_pointer_outparam", case_DFB023_double_pointer_outparam},
    {"DFB024", "global_value_flow", case_DFB024_global_value_flow},
    {"DFB025", "global_field_precise", case_DFB025_global_field_precise},
    {"DFB026", "global_interproc_reader", case_DFB026_global_interproc_reader},

    {"DFB030", "heap_field", case_DFB030_heap_field},
    {"DFB031", "heap_realloc_preserve", case_DFB031_heap_realloc_preserve},

    {"DFB040", "struct_field_precise", case_DFB040_struct_field_precise},
    {"DFB041", "pointer_arithmetic_field", case_DFB041_pointer_arithmetic_field},
    {"DFB042", "union_alias", case_DFB042_union_alias},
    {"DFB043", "array_constant_index", case_DFB043_array_constant_index},
    {"DFB044", "array_variable_index", case_DFB044_array_variable_index},
    {"DFB045", "nested_aggregate_field", case_DFB045_nested_aggregate_field},
    {"DFB046", "partial_overwrite_subfield", case_DFB046_partial_overwrite_subfield},

    {"DFB050", "identity_call", case_DFB050_identity_call},
    {"DFB051", "nested_call", case_DFB051_nested_call},
    {"DFB052", "callsite_context", case_DFB052_callsite_context},
    {"DFB053", "large_struct_return", case_DFB053_large_struct_return},
    {"DFB054", "status_outparam", case_DFB054_status_outparam},
    {"DFB055", "deep_field_passthrough", case_DFB055_deep_field_passthrough},
    {"DFB056", "arg_to_ret_summary", case_DFB056_arg_to_ret_summary},
    {"DFB057", "struct_field_to_ret_summary", case_DFB057_struct_field_to_ret_summary},
    {"DFB058", "arg_to_outparam_summary", case_DFB058_arg_to_outparam_summary},
    {"DFB059", "inout_field_update_summary", case_DFB059_inout_field_update_summary},

    {"DFB060", "recursion", case_DFB060_recursion},

    {"DFB070", "function_pointer", case_DFB070_function_pointer},
    {"DFB071", "callback_registration", case_DFB071_callback_registration},
    {"DFB072", "function_pointer_table", case_DFB072_function_pointer_table},
    {"DFB073", "indirect_sink_wrapper", case_DFB073_indirect_sink_wrapper},

    {"DFB080", "cpp_virtual_call", case_DFB080_cpp_virtual_call},
    {"DFB081", "cpp_lambda_capture", case_DFB081_cpp_lambda_capture},

    {"DFB091", "tls_value", case_DFB091_tls_value},

    {"DFB100", "varargs", case_DFB100_varargs},
    {"DFB101", "tail_call_candidate", case_DFB101_tail_call_candidate},
    {"DFB102", "signed_unsigned_boundary", case_DFB102_signed_unsigned_boundary},

    {"DFB110", "setjmp_longjmp", case_DFB110_setjmp_longjmp},
    {"DFB111", "cpp_exception_flow", case_DFB111_cpp_exception_flow},

    {"DFB120", "memcpy_buffer", case_DFB120_memcpy_buffer},
    {"DFB121", "memmove_buffer", case_DFB121_memmove_buffer},
    {"DFB122", "strcpy_buffer", case_DFB122_strcpy_buffer},
    {"DFB123", "memset_partial_memcpy", case_DFB123_memset_partial_memcpy},

    {"DFB130", "shared_import_arg_to_ret", case_DFB130_shared_import_arg_to_ret},
    {"DFB131", "shared_import_outparam", case_DFB131_shared_import_outparam},
};

const dfb_case_entry_t *dfb_get_cases(size_t *count) {
    if (count) {
        *count = sizeof(g_cases) / sizeof(g_cases[0]);
    }
    return g_cases;
}
```

POSIX 전용 또는 C++ 전용 케이스는 각각 `src/dfbench_runtime_posix.c`, `src/dfbench_runtime_cpp.c`, `src/dfbench_runtime_cpp_exceptions.c`에 별도 등록한다.

파일: `src/main.c`

```c
#include <stdio.h>
#include <string.h>
#include "dfbench_cases.h"

static void usage(const char *argv0) {
    printf("Usage:\n");
    printf("  %s --list\n", argv0);
    printf("  %s --run-all\n", argv0);
    printf("  %s --run DFB001\n", argv0);
}

int main(int argc, char **argv) {
    size_t count = 0;
    const dfb_case_entry_t *cases = dfb_get_cases(&count);

    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        for (size_t i = 0; i < count; i++) {
            printf("%s %s\n", cases[i].id, cases[i].name);
        }
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--run-all") == 0) {
        for (size_t i = 0; i < count; i++) {
            cases[i].fn();
        }
        printf("OK\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "--run") == 0) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(argv[2], cases[i].id) == 0) {
                cases[i].fn();
                printf("OK %s\n", cases[i].id);
                return 0;
            }
        }
        fprintf(stderr, "Unknown case: %s\n", argv[2]);
        return 2;
    }

    usage(argv[0]);
    return 1;
}
```

### 5.1 Function-summary-ready 케이스 규칙

함수별 input/output DB 구축에 사용하는 케이스는 일반 direct-trace 케이스와 별도로 더 강한 규칙을 따라야 한다.

핵심 원칙:

```text
1. direct-trace 검증용 케이스와 function-summary-ready 케이스를 같은 의미로 취급하지 않는다.
2. function-summary-ready 케이스에서는 어느 한 함수 안에도 직접 source와 sink가 동시에 공존하면 안 된다.
3. 실제 데이터 전달 관계는 별도 helper/callee 함수 안에 존재해야 한다.
4. summary 대상 helper 함수 내부에는 직접 source나 sink를 두지 않는다.
5. case_DFBxxx_* wrapper는 source 준비 함수, helper 호출, sink 전달 함수만 조합하는 orchestration 역할만 맡는다.
6. 함수 요약 DB의 ground truth는 summary-ready 케이스만을 기준으로 만든다.
```

권장 패턴:

```c
static int dfb_prepare_input(void) {
    return dfb_source_A();
}

static int dfb_summary_target(const SomeType *in) {
    return in->field;
}

static void dfb_consume_output(int value) {
    dfb_sink_int(value);
}

DFB_CASE void case_DFBxxx_example(void) {
    SomeType value;
    value.field = dfb_prepare_input();
    value.other = 0;

    dfb_consume_output(dfb_summary_target(&value));
}
```

위 패턴에서 source 준비 함수, summary 함수, sink 전달 함수가 서로 분리된다. 따라서 단일 추적 검증과 함수 summary DB 검증을 동시에 지원하면서도, 어느 한 함수에도 source와 sink를 같이 두지 않을 수 있다.

---

## 6. 최소 Core Cases

이 섹션의 케이스는 반드시 v1에서 구현한다.

### DFB001 direct value

파일: `src/cases_basic.c`

```c
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB001_direct_value(void) {
    int a = dfb_source_A();
    dfb_sink_int(a);
}
```

기대 흐름:

```text
dfb_source_A.ret -> dfb_sink_int.arg0
```

---

### DFB002 arithmetic value

```c
DFB_CASE void case_DFB002_arithmetic_value(void) {
    int a = dfb_source_A();
    int b = (a + 3) ^ 0x55;
    dfb_sink_int(b);
}
```

기대 흐름:

```text
dfb_source_A.ret -> arithmetic ops -> dfb_sink_int.arg0
```

---

### DFB003 cast value

```c
DFB_CASE void case_DFB003_cast_value(void) {
    int a = dfb_source_A();
    long b = (long)a;
    int c = (int)b;
    dfb_sink_int(c);
}
```

기대 흐름:

```text
dfb_source_A.ret -> CAST/INT_ZEXT/INT_SEXT/SUBPIECE -> dfb_sink_int.arg0
```

---

### DFB010 branch phi

파일: `src/cases_control_flow.c`

```c
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB010_branch_phi(void) {
    int flag = dfb_source_C() & 1;
    int x;

    if (flag) {
        x = dfb_source_A();
    } else {
        x = dfb_source_B();
    }

    dfb_sink_int(x);
}
```

기대 흐름:

```text
dfb_source_A.ret -> MULTIEQUAL/PHI -> dfb_sink_int.arg0
dfb_source_B.ret -> MULTIEQUAL/PHI -> dfb_sink_int.arg0
```

---

### DFB011 loop phi

```c
DFB_CASE void case_DFB011_loop_phi(void) {
    int x = dfb_source_A();
    int n = (dfb_source_C() & 3) + 1;

    for (int i = 0; i < n; i++) {
        x = x + i;
    }

    dfb_sink_int(x);
}
```

기대 흐름:

```text
dfb_source_A.ret -> loop-carried value -> dfb_sink_int.arg0
```

---

### DFB012 switch merge

```c
DFB_CASE void case_DFB012_switch_merge(void) {
    int selector = dfb_source_C() & 3;
    int x;

    switch (selector) {
    case 0:
        x = dfb_source_A();
        break;
    case 1:
        x = dfb_source_B();
        break;
    default:
        x = dfb_source_C();
        break;
    }

    dfb_sink_int(x);
}
```

기대 흐름:

```text
source_A/source_B/source_C -> switch merge -> sink
```

---

## 7. Stack / Pointer Cases

파일: `src/cases_stack.c`

### DFB020 stack local

```c
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB020_stack_local(void) {
    int local = dfb_source_A();
    int copied = local;
    dfb_sink_int(copied);
}
```

---

### DFB021 stack out-param

```c
DFB_SOURCE void dfb_write_source_to_out(int *out) {
    *out = dfb_source_A();
}

DFB_CASE void case_DFB021_stack_outparam(void) {
    int local = 0;
    dfb_write_source_to_out(&local);
    dfb_sink_int(local);
}
```

기대 흐름:

```text
dfb_source_A.ret -> *out -> local -> sink
```

---

### DFB022 arg to out-param

```c
DFB_SOURCE void dfb_copy_arg_to_out(int in, int *out) {
    *out = in;
}

DFB_CASE void case_DFB022_arg_to_outparam(void) {
    int local = 0;
    int a = dfb_source_A();
    dfb_copy_arg_to_out(a, &local);
    dfb_sink_int(local);
}
```

기대 흐름:

```text
dfb_source_A.ret -> callee.arg0 -> callee.arg1* -> local -> sink
```

---

### DFB023 double pointer out-param

```c
DFB_SOURCE void dfb_store_through_double_pointer(int **pp, int value) {
    **pp = value;
}

DFB_CASE void case_DFB023_double_pointer_outparam(void) {
    int local = 0;
    int *p = &local;
    int a = dfb_source_A();

    dfb_store_through_double_pointer(&p, a);

    dfb_sink_int(local);
}
```

기대 흐름:

```text
dfb_source_A.ret -> value -> **pp -> local -> sink
```

---

## 7.1 Global Variable Cases

파일: `src/cases_global.c`

이 섹션은 일반 전역 변수 경유 흐름을 검증한다. TLS나 thread global과 별개로, 가장 기본적인 `source -> global -> sink` 및 전역 구조체 필드 민감도를 확인한다.

### DFB024 global value flow

```c
#include "dfbench_sources_sinks.h"

static volatile int g_dfb_global_value = 0;

DFB_CASE void case_DFB024_global_value_flow(void) {
    int a = dfb_source_A();
    g_dfb_global_value = a;
    dfb_sink_int(g_dfb_global_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> global:g_dfb_global_value -> dfb_sink_int.arg0
```

---

### DFB025 global field precise

```c
#include "dfbench_sources_sinks.h"

typedef struct DFBGlobalPair {
    int left;
    int right;
} DFBGlobalPair;

static volatile DFBGlobalPair g_dfb_global_pair;

DFB_CASE void case_DFB025_global_field_precise(void) {
    g_dfb_global_pair.left = dfb_source_A();
    g_dfb_global_pair.right = dfb_source_B();

    dfb_sink_int(g_dfb_global_pair.right);
}
```

기대 흐름:

```text
dfb_source_B.ret -> global:g_dfb_global_pair.right -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_A.ret must not reach sink
```

---

### DFB026 global interproc reader

이 케이스는 전역 변수 경유 흐름이 함수 경계를 한 번 더 통과할 때도 유지되는지 확인한다.

```c
#include "dfbench_sources_sinks.h"

static volatile int g_dfb_global_main_value = 0;
static volatile int g_dfb_global_shadow_value = 0;

DFB_SOURCE void dfb_write_global_values(void) {
    g_dfb_global_main_value = dfb_source_A();
    g_dfb_global_shadow_value = dfb_source_B();
}

DFB_SOURCE int dfb_read_global_main_value(void) {
    return g_dfb_global_main_value;
}

DFB_CASE void case_DFB026_global_interproc_reader(void) {
    dfb_write_global_values();
    dfb_sink_int(dfb_read_global_main_value());
}
```

기대 흐름:

```text
dfb_source_A.ret -> global:g_dfb_global_main_value -> callee return -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```


---

## 8. Heap Cases

파일: `src/cases_heap.c`

```c
#include <stdlib.h>
#include "dfbench_sources_sinks.h"

typedef struct DFBHeapObj {
    int value;
    int other;
} DFBHeapObj;

DFB_CASE void case_DFB030_heap_field(void) {
    DFBHeapObj *obj = (DFBHeapObj *)malloc(sizeof(DFBHeapObj));
    if (!obj) {
        return;
    }

    obj->value = dfb_source_A();
    obj->other = dfb_source_B();

    dfb_sink_int(obj->value);

    free(obj);
}
```

기대 흐름:

```text
dfb_source_A.ret -> heap obj.value -> sink
```

금지 흐름:

```text
dfb_source_B.ret should not reach sink
```

---

### DFB031 heap realloc preserve

이 케이스는 `realloc` 이후에도 기존 heap field의 source 흐름이 보존되는지 확인한다.

```c
#include <stdlib.h>
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB031_heap_realloc_preserve(void) {
    int *buf = (int *)malloc(sizeof(int) * 2);
    if (!buf) {
        return;
    }

    buf[0] = dfb_source_A();
    buf[1] = dfb_source_B();

    buf = (int *)realloc(buf, sizeof(int) * 4);
    if (!buf) {
        return;
    }

    dfb_sink_int(buf[0]);

    free(buf);
}
```

기대 흐름:

```text
dfb_source_A.ret -> heap buf[0] -> realloc-preserved heap -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

---

## 9. Struct / Field / Alias Cases

파일: `src/cases_struct.c`

### DFB040 struct field precise

```c
#include <stddef.h>
#include <stdint.h>
#include "dfbench_sources_sinks.h"

typedef struct DFBPoint {
    int x;
    int y;
} DFBPoint;

DFB_CASE void case_DFB040_struct_field_precise(void) {
    DFBPoint p;

    p.x = dfb_source_A();
    p.y = dfb_source_B();

    dfb_sink_int(p.y);
}
```

기대 흐름:

```text
dfb_source_B.ret -> p.y -> sink
```

금지 흐름:

```text
dfb_source_A.ret must not reach sink
```

---

### DFB041 pointer arithmetic field

```c
typedef struct DFBOffsetObj {
    int pad;
    int value;
} DFBOffsetObj;

DFB_CASE void case_DFB041_pointer_arithmetic_field(void) {
    DFBOffsetObj obj;
    char *base = (char *)&obj;

    *((int *)(base + offsetof(DFBOffsetObj, value))) = dfb_source_A();

    dfb_sink_int(obj.value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> PTRSUB/PTRADD field offset -> obj.value -> sink
```

---

### DFB042 union alias

```c
typedef union DFBUnion {
    int i;
    float f;
} DFBUnion;

DFB_CASE void case_DFB042_union_alias(void) {
    DFBUnion u;
    u.i = dfb_source_A();

    dfb_sink_int(u.i);
}
```

기대 흐름:

```text
dfb_source_A.ret -> union storage -> sink
```

---

## 9.1 Array / Index Cases

파일: `src/cases_array.c`

배열 케이스는 구조체 field sensitivity와 별도로 index/offset sensitivity를 검증한다.

### DFB043 array constant index

```c
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB043_array_constant_index(void) {
    int arr[4] = {0, 0, 0, 0};

    arr[0] = dfb_source_A();
    arr[2] = dfb_source_B();

    dfb_sink_int(arr[2]);
}
```

기대 흐름:

```text
dfb_source_B.ret -> arr[2] -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_A.ret must not reach sink
```

---

### DFB044 array variable index

```c
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB044_array_variable_index(void) {
    int arr[4] = {0, 0, 0, 0};

    int idx = dfb_source_C() & 3;
    int a = dfb_source_A();

    arr[idx] = a;

    dfb_sink_int(arr[idx]);
}
```

기대 흐름:

```text
dfb_source_A.ret -> arr[idx] -> dfb_sink_int.arg0
```

주의:

```text
idx가 variable offset이므로 분석기는 arr 전체를 conservative하게 merge할 수 있다.
이 케이스는 array-index 추적이 되면 PASS, unknown array memory로 source를 보존해도 WARN 허용 대상이다.
```

---

### DFB045 nested aggregate field

이 케이스는 중첩 구조체와 배열이 결합된 offset을 field-sensitive하게 추적할 수 있는지 확인한다.

함수 summary DB 구축도 고려하므로, 실제 field 선택은 별도 helper 함수에서 수행한다.

```c
#include "dfbench_sources_sinks.h"

typedef struct DFBInner {
    int values[2];
    int extra;
} DFBInner;

typedef struct DFBOuter {
    int tag;
    DFBInner inner;
} DFBOuter;

static int dfb_read_nested_selected_value(const DFBOuter *obj) {
    return obj->inner.values[1];
}

DFB_CASE void case_DFB045_nested_aggregate_field(void) {
    DFBOuter obj = {0};

    obj.inner.values[0] = dfb_source_B();
    obj.inner.values[1] = dfb_source_A();

    dfb_sink_int(dfb_read_nested_selected_value(&obj));
}
```

기대 흐름:

```text
dfb_source_A.ret -> caller obj.inner.values[1] -> callee.arg0.inner.values[1] -> callee.ret -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

권장 summary:

```text
dfb_read_nested_selected_value: arg0.inner.values[1] -> ret
```

---

### DFB046 partial overwrite subfield

이 케이스는 전체 워드 중 일부 바이트만 source로 덮었을 때 subfield 추적이 가능한지 확인한다.

```c
#include <stdint.h>
#include "dfbench_sources_sinks.h"

typedef union DFBPartialWord {
    uint32_t whole;
    uint8_t bytes[4];
} DFBPartialWord;

DFB_CASE void case_DFB046_partial_overwrite_subfield(void) {
    DFBPartialWord word;

    word.whole = 0x11223344U;
    word.bytes[0] = (uint8_t)dfb_source_A();
    word.bytes[1] = (uint8_t)dfb_source_B();

    dfb_sink_int((int)(word.whole & 0xff));
}
```

기대 흐름:

```text
dfb_source_A.ret -> partial byte overwrite -> word.low_byte -> mask -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```


---

## 10. Interprocedural Cases

파일: `src/cases_interproc.c`

### DFB050 identity call

```c
#include "dfbench_sources_sinks.h"

DFB_SOURCE int dfb_identity_int(int x) {
    return x;
}

DFB_CASE void case_DFB050_identity_call(void) {
    int a = dfb_source_A();
    int b = dfb_identity_int(a);
    dfb_sink_int(b);
}
```

---

### DFB051 nested call

```c
DFB_SOURCE int dfb_transform_int(int x) {
    return (x * 7) + 1;
}

DFB_SOURCE int dfb_nested_2(int x) {
    return dfb_transform_int(x);
}

DFB_SOURCE int dfb_nested_1(int x) {
    return dfb_nested_2(x);
}

DFB_CASE void case_DFB051_nested_call(void) {
    int a = dfb_source_A();
    int b = dfb_nested_1(a);
    dfb_sink_int(b);
}
```

---

### DFB052 callsite context

이 케이스는 같은 callee가 서로 다른 source를 받았을 때 callsite 구분이 되는지 확인한다.

```c
DFB_SOURCE int dfb_same_identity(int x) {
    return x;
}

static int dfb_callsite_context_source_A(void) {
    return dfb_source_A();
}

static int dfb_callsite_context_source_B(void) {
    return dfb_source_B();
}

static void dfb_callsite_context_sink(int value) {
    dfb_sink_int(value);
}

DFB_CASE void case_DFB052_callsite_context(void) {
    int a = dfb_callsite_context_source_A();
    int b = dfb_callsite_context_source_B();

    int x = dfb_same_identity(a);
    int y = dfb_same_identity(b);

    dfb_callsite_context_sink(x);

    /* y가 제거되지 않도록 사용 */
    DFB_TOUCH_INT(y);
}
```

기대 흐름:

```text
dfb_source_A.ret -> dfb_callsite_context_source_A.ret -> selected callsite arg0 -> dfb_same_identity.arg0 -> dfb_same_identity.ret -> dfb_callsite_context_sink.arg0 -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret -> dfb_callsite_context_source_B.ret -> unselected callsite arg0 -> dfb_same_identity.arg0 -> dfb_same_identity.ret must not reach dfb_sink_int.arg0
```

권장 summary:

```text
dfb_same_identity: arg0 -> ret
```

---

### DFB053 large struct return

대형 구조체 반환은 ABI에 따라 hidden sret pointer를 만들 수 있다.

```c
typedef struct DFBBigStruct {
    long a;
    long b;
    long c;
    long d;
} DFBBigStruct;

DFB_SOURCE DFBBigStruct dfb_make_big_struct(long x) {
    DFBBigStruct s;
    s.a = x;
    s.b = 0;
    s.c = 0;
    s.d = 0;
    return s;
}

DFB_CASE void case_DFB053_large_struct_return(void) {
    long a = dfb_source_long_A();
    DFBBigStruct s = dfb_make_big_struct(a);
    dfb_sink_long(s.a);
}
```

기대 흐름:

```text
dfb_source_long_A.ret -> hidden sret / struct return -> s.a -> sink
```

---

### DFB054 status out-param

이 케이스는 함수 return value와 out-param이 분리된 흔한 API 패턴을 검증한다.

```c
DFB_SOURCE int dfb_status_out_writer(int *out, int value) {
    *out = value;
    return 0;
}

DFB_CASE void case_DFB054_status_outparam(void) {
    int main_value = 0;
    int shadow_value = 0;

    DFB_TOUCH_INT(dfb_status_out_writer(&main_value, dfb_source_A()));
    DFB_TOUCH_INT(dfb_status_out_writer(&shadow_value, dfb_source_B()));

    dfb_sink_int(main_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> callee.arg1 -> callee.arg0* -> main_value -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

---

### DFB055 deep field passthrough

이 케이스는 질문에서 제시한 형태와 같이, 중간 call stack에서는 관심 필드를 직접 읽지 않고 더 깊은 callee에서만 최종 참조하는 경우를 검증한다.

핵심 포인트:

```text
- source는 struct field에 저장된다.
- 중간 함수들은 다른 field를 덮어쓰거나 동일 포인터를 전달만 한다.
- 최종적으로 가장 깊은 callee가 처음 저장된 field를 읽어 sink로 보낸다.
```

```c
#include "dfbench_sources_sinks.h"

typedef struct DFBDeepStruct {
    int aaa;
    int bbb;
    int ccc;
} DFBDeepStruct;

static void dfb_deep_func4(DFBDeepStruct *v) {
    dfb_sink_int(v->aaa);
}

static void dfb_deep_func2(DFBDeepStruct *v) {
    v->bbb = 4;
    v->ccc = 6;
    dfb_deep_func4(v);
}

static void dfb_deep_func1(DFBDeepStruct *v) {
    v->bbb = 2;
    v->ccc = 4;
    dfb_deep_func2(v);
}

DFB_CASE void case_DFB055_deep_field_passthrough(void) {
    DFBDeepStruct val;

    val.aaa = dfb_source_A();
    val.bbb = dfb_source_B();
    val.ccc = dfb_source_C();

    dfb_deep_func1(&val);
}
```

기대 흐름:

```text
dfb_source_A.ret -> val.aaa -> callee chain passthrough -> deepest callee load -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
dfb_source_C.ret must not reach sink
```

---

## 10.1 Function Summary / I-O Cases

이 섹션의 케이스는 backward slice의 단일 경로 추적뿐 아니라, 함수별 input/output 관계를 요약 DB 형태로 분리해 저장하려는 목적을 위해 추가한다.

핵심 원칙:

```text
- source와 sink가 같은 함수 내부에 공존하지 않는 패턴을 포함한다.
- 함수 단위로 보면 arg, ret, out-param, inout field가 각각 독립적인 입출력 슬롯이 된다.
- summary DB는 이 케이스들을 이용해 함수별 "입력 -> 출력" 관계를 학습/검증할 수 있어야 한다.
```

### DFB056 arg to ret summary

이 케이스는 가장 기본적인 함수 summary 패턴인 `arg0 -> return` 관계를 검증한다.

```c
DFB_SOURCE int dfb_summary_arg_to_ret(int x) {
    return x;
}

DFB_CASE void case_DFB056_arg_to_ret_summary(void) {
    int main_value = dfb_source_A();
    int shadow_value = dfb_source_B();
    int result = dfb_summary_arg_to_ret(main_value);

    DFB_TOUCH_INT(shadow_value);
    dfb_sink_int(result);
}
```

기대 흐름:

```text
dfb_source_A.ret -> caller arg0 -> callee.arg0 -> callee.ret -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

권장 summary:

```text
dfb_summary_arg_to_ret: arg0 -> ret
```

---

### DFB057 struct field to ret summary

이 케이스는 구조체 포인터 입력을 받아 특정 field만 return하는 함수 summary를 검증한다.

```c
typedef struct DFBSummaryStruct {
    int chosen;
    int other;
} DFBSummaryStruct;

DFB_SOURCE int dfb_summary_field_to_ret(const DFBSummaryStruct *v) {
    return v->chosen;
}

DFB_CASE void case_DFB057_struct_field_to_ret_summary(void) {
    DFBSummaryStruct value;

    value.chosen = dfb_source_A();
    value.other = dfb_source_B();

    dfb_sink_int(dfb_summary_field_to_ret(&value));
}
```

기대 흐름:

```text
dfb_source_A.ret -> caller struct field -> callee.arg0.chosen -> callee.ret -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

권장 summary:

```text
dfb_summary_field_to_ret: arg0.chosen -> ret
```

---

### DFB058 arg to out-param summary

이 케이스는 `arg1 -> *arg0` 형태의 out-param summary를 검증한다.

```c
DFB_SOURCE void dfb_summary_arg_to_out(int *out, int value) {
    *out = value;
}

DFB_CASE void case_DFB058_arg_to_outparam_summary(void) {
    int main_value = 0;
    int shadow_value = dfb_source_B();

    dfb_summary_arg_to_out(&main_value, dfb_source_A());
    DFB_TOUCH_INT(shadow_value);

    dfb_sink_int(main_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> caller arg1 -> callee.arg1 -> callee.arg0* -> main_value -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

권장 summary:

```text
dfb_summary_arg_to_out: arg1 -> arg0*
```

---

### DFB059 inout field update summary

이 케이스는 inout 포인터를 받아 특정 field를 갱신하는 함수 summary를 검증한다.

```c
typedef struct DFBInOutStruct {
    int tracked;
    int noise;
} DFBInOutStruct;

DFB_SOURCE void dfb_summary_inout_update(DFBInOutStruct *v, int value) {
    v->tracked = value;
}

DFB_CASE void case_DFB059_inout_field_update_summary(void) {
    DFBInOutStruct value;

    value.tracked = 0;
    value.noise = dfb_source_B();

    dfb_summary_inout_update(&value, dfb_source_A());
    dfb_sink_int(value.tracked);
}
```

기대 흐름:

```text
dfb_source_A.ret -> caller arg1 -> callee.arg1 -> callee.arg0.tracked* -> caller value.tracked -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

권장 summary:

```text
dfb_summary_inout_update: arg1 -> arg0.tracked*
```

---

## 11. Recursion Cases

파일: `src/cases_interproc.c`에 함께 넣어도 된다.

```c
DFB_SOURCE int dfb_recursive_transform(int x, int n) {
    if (n <= 0) {
        return x;
    }
    return dfb_recursive_transform(x + 1, n - 1);
}

DFB_CASE void case_DFB060_recursion(void) {
    int a = dfb_source_A();
    int b = dfb_recursive_transform(a, 3);
    dfb_sink_int(b);
}
```

기대 흐름:

```text
dfb_source_A.ret -> recursive arg0 -> recursive ret -> sink
```

테스트 목적:

```text
무한 추적 방지
visited 처리 검증
```

---

## 12. Indirect Call / Callback Cases

파일: `src/cases_indirect.c`

### DFB070 function pointer

```c
#include "dfbench_sources_sinks.h"

typedef int (*dfb_int_fn_t)(int);

DFB_SOURCE int dfb_fp_target(int x) {
    return x + 10;
}

DFB_CASE void case_DFB070_function_pointer(void) {
    dfb_int_fn_t fn = dfb_fp_target;

    int a = dfb_source_A();
    int b = fn(a);

    dfb_sink_int(b);
}
```

---

### DFB071 callback registration

```c
static void (*g_dfb_callback)(int) = 0;

DFB_SOURCE void dfb_register_callback(void (*cb)(int)) {
    g_dfb_callback = cb;
}

DFB_SOURCE void dfb_callback_target(int x) {
    dfb_sink_int(x);
}

DFB_CASE void case_DFB071_callback_registration(void) {
    int a = dfb_source_A();

    dfb_register_callback(dfb_callback_target);

    if (g_dfb_callback) {
        g_dfb_callback(a);
    }
}
```

기대 흐름:

```text
dfb_source_A.ret -> indirect callback arg -> dfb_sink_int.arg0
```

---

### DFB072 function pointer table

```c
DFB_SOURCE int dfb_fp_table_target_A(int x) {
    return x + 1;
}

DFB_SOURCE int dfb_fp_table_target_B(int x) {
    return x + 2;
}

typedef int (*dfb_table_fn_t)(int);

DFB_CASE void case_DFB072_function_pointer_table(void) {
    dfb_table_fn_t table[2];

    table[0] = dfb_fp_table_target_A;
    table[1] = dfb_fp_table_target_B;

    int selector = dfb_source_C() & 1;
    int a = dfb_source_A();

    int r = table[selector](a);

    dfb_sink_int(r);
}
```

기대 흐름:

```text
dfb_source_A.ret -> function pointer table dispatch -> selected target ret -> dfb_sink_int.arg0
```

주의:

```text
selector가 런타임 값이므로 target_A와 target_B가 모두 후보가 될 수 있다.
분석기는 간접 호출 target을 확정하지 못하더라도 source taint를 sink까지 보존하면 WARN 허용 가능하다.
```

---

### DFB073 indirect sink wrapper

이 케이스는 간접 호출의 target이 최종 sink wrapper인 경우에도 anchor까지 흐름이 이어지는지 확인한다.

```c
typedef void (*dfb_sink_wrapper_fn_t)(int);

static void dfb_sink_wrapper_impl(int x) {
    dfb_sink_int(x);
}

DFB_CASE void case_DFB073_indirect_sink_wrapper(void) {
    dfb_sink_wrapper_fn_t fn = dfb_sink_wrapper_impl;
    int a = dfb_source_A();
    int b = dfb_source_B();

    fn(a);
    DFB_TOUCH_INT(b);
}
```

기대 흐름:

```text
dfb_source_A.ret -> indirect wrapper arg -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```


---

## 13. C++ Cases

파일: `cpp/cases_cpp.cpp`

### DFB080 virtual call

```cpp
#include "dfbench_sources_sinks.h"

class DFBBase {
public:
    virtual ~DFBBase() {}
    virtual int get(int x) {
        return x;
    }
};

class DFBDerived : public DFBBase {
public:
    int get(int x) override {
        return x + 100;
    }
};

extern "C" DFB_CASE void case_DFB080_cpp_virtual_call(void) {
    DFBDerived d;
    DFBBase *b = &d;

    int a = dfb_source_A();
    int r = b->get(a);

    dfb_sink_int(r);
}
```

---

### DFB081 lambda capture

```cpp
#include "dfbench_sources_sinks.h"

extern "C" DFB_CASE void case_DFB081_cpp_lambda_capture(void) {
    int a = dfb_source_A();

    auto fn = [a]() -> int {
        return a;
    };

    dfb_sink_int(fn());
}
```

---

## 14. Thread / TLS Cases

파일: `src/cases_runtime_bridge.c`

### DFB090 thread shared memory

이 케이스는 POSIX runtime 전용 실행파일에만 포함한다.

Windows 기본 바이너리에는 등록하지 않는다.

```c
#include "dfbench_sources_sinks.h"

#if !defined(_WIN32)
#include <pthread.h>

static volatile int g_thread_value = 0;

static void *dfb_thread_worker(void *arg) {
    int *p = (int *)arg;
    g_thread_value = *p;
    return 0;
}

DFB_CASE void case_DFB090_thread_shared_memory(void) {
    int a = dfb_source_A();
    pthread_t tid;

    if (pthread_create(&tid, 0, dfb_thread_worker, &a) == 0) {
        pthread_join(tid, 0);
    }

    dfb_sink_int(g_thread_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> pthread_create arg -> worker arg -> global -> sink
```

정책:

```text
이 케이스는 의미 보존을 위해 POSIX 전용 실행파일로 분리한다.
Windows에서 direct flow fallback으로 대체하지 않는다.
```

---

### DFB091 TLS value

```c
#if defined(_MSC_VER)
__declspec(thread) static int g_tls_value;
#else
__thread static int g_tls_value;
#endif

DFB_CASE void case_DFB091_tls_value(void) {
    int a = dfb_source_A();
    g_tls_value = a;
    dfb_sink_int(g_tls_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> TLS storage -> sink
```

---

### DFB092 pthread table dispatch

이 케이스는 `pthread` 실행, 런타임 selector, 함수 포인터 테이블, 조건 분기가 결합된 융합형 dispatch를 검증한다.

핵심 포인트:

```text
- pthread worker 자체는 고정이지만, worker 내부에서 실제 target function이 런타임에 결정된다.
- target은 function pointer table에서 선택된다.
- selector는 조건문을 통해 정규화된다.
- 최종 target이 struct field를 읽어 sink로 보낸다.
```

```c
#include "dfbench_sources_sinks.h"

#if !defined(_WIN32)
#include <pthread.h>

typedef struct DFBThreadDispatch {
    int selected;
    int ignored;
} DFBThreadDispatch;

typedef void (*dfb_thread_target_t)(DFBThreadDispatch *ctx);

static void dfb_thread_target_A(DFBThreadDispatch *ctx) {
    dfb_sink_int(ctx->selected);
}

static void dfb_thread_target_B(DFBThreadDispatch *ctx) {
    DFB_TOUCH_INT(ctx->ignored);
}

static void *dfb_thread_dispatch_worker(void *arg) {
    DFBThreadDispatch *ctx = (DFBThreadDispatch *)arg;
    dfb_thread_target_t table[2];
    int selector;

    table[0] = dfb_thread_target_A;
    table[1] = dfb_thread_target_B;

    if ((dfb_source_C() & 1) == 0) {
        selector = 0;
    } else {
        selector = 1;
    }

    table[selector](ctx);
    return 0;
}

DFB_CASE void case_DFB092_pthread_table_dispatch(void) {
    DFBThreadDispatch ctx;
    pthread_t tid;

    ctx.selected = dfb_source_A();
    ctx.ignored = dfb_source_B();

    if (pthread_create(&tid, 0, dfb_thread_dispatch_worker, &ctx) == 0) {
        pthread_join(tid, 0);
    }
}

#endif
```

기대 흐름:

```text
dfb_source_A.ret -> ctx.selected -> pthread arg -> worker -> conditional selector -> function pointer table -> target_A load -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

주의:

```text
이 케이스는 POSIX runtime 전용 실행파일에만 포함한다.
thread dispatch와 indirect call을 함께 풀지 못하는 분석기는 WARN 또는 FAIL이 될 수 있다.
```

---

## 15. ABI / Optimization Cases

파일: `src/cases_abi.c`

### DFB100 varargs

```c
#include <stdarg.h>
#include "dfbench_sources_sinks.h"

DFB_SOURCE int dfb_pick_first_vararg(const char *fmt, ...) {
    (void)fmt;

    va_list ap;
    va_start(ap, fmt);
    int x = va_arg(ap, int);
    va_end(ap);

    return x;
}

DFB_CASE void case_DFB100_varargs(void) {
    int a = dfb_source_A();
    int b = dfb_pick_first_vararg("%d", a);
    dfb_sink_int(b);
}
```

기대 흐름:

```text
dfb_source_A.ret -> vararg slot -> va_arg -> ret -> sink
```

---

### DFB101 tail call candidate

이 케이스는 최적화 빌드에서 tail call 형태가 될 수 있다.

```c
DFB_SOURCE int dfb_tail_target(int x) {
    return x;
}

DFB_SOURCE int dfb_tail_wrapper(int x) {
    return dfb_tail_target(x);
}

DFB_CASE void case_DFB101_tail_call_candidate(void) {
    int a = dfb_source_A();
    int b = dfb_tail_wrapper(a);
    dfb_sink_int(b);
}
```

기대 흐름:

```text
dfb_source_A.ret -> wrapper -> tail target -> sink
```

---

### DFB102 signed / unsigned boundary

이 케이스는 signed/unsigned 변환과 폭 확장이 섞여도 source 동일성이 유지되는지 확인한다.

```c
#include <stdint.h>
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB102_signed_unsigned_boundary(void) {
    int a = dfb_source_A();
    int noise = dfb_source_B();
    unsigned int b = (unsigned int)a;
    uint64_t c = (uint64_t)b;
    int d = (int)c;

    DFB_TOUCH_INT(noise);
    dfb_sink_int(d);
}
```

기대 흐름:

```text
dfb_source_A.ret -> INT_ZEXT/CAST width changes -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

---

## 16. Exceptional Control Flow Cases

파일: `src/cases_exceptional.c`

### DFB110 setjmp / longjmp

```c
#include <setjmp.h>
#include "dfbench_sources_sinks.h"

static jmp_buf g_jmp_buf;
static volatile int g_jmp_value = 0;

DFB_SOURCE void dfb_longjmp_writer(int x) {
    g_jmp_value = x;
    longjmp(g_jmp_buf, 1);
}

DFB_CASE void case_DFB110_setjmp_longjmp(void) {
    int a = dfb_source_A();

    if (setjmp(g_jmp_buf) == 0) {
        dfb_longjmp_writer(a);
    }

    dfb_sink_int(g_jmp_value);
}
```

기대 흐름:

```text
dfb_source_A.ret -> global write before longjmp -> sink
```

---

### DFB111 C++ exception flow

파일: `cpp/cases_cpp_exceptions.cpp`

```cpp
#include "dfbench_sources_sinks.h"

static volatile int g_cpp_exception_value = 0;

static void dfb_cpp_throw_writer(int x) {
    g_cpp_exception_value = x;
    throw 1;
}

extern "C" DFB_CASE void case_DFB111_cpp_exception_flow(void) {
    int a = dfb_source_A();

    try {
        dfb_cpp_throw_writer(a);
    } catch (...) {
        dfb_sink_int(g_cpp_exception_value);
    }
}
```

기대 흐름:

```text
dfb_source_A.ret -> global write before throw -> catch -> sink
```

---

## 17. Memory API Cases

파일: `src/cases_memory_api.c`

### DFB120 memcpy buffer

```c
#include <string.h>
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB120_memcpy_buffer(void) {
    char src[16] = {0};
    char dst[16] = {0};

    src[0] = (char)dfb_source_A();

    memcpy(dst, src, sizeof(src));

    dfb_sink_int((int)dst[0]);
}
```

---

### DFB121 memmove buffer

```c
DFB_CASE void case_DFB121_memmove_buffer(void) {
    char buf[32] = {0};

    buf[0] = (char)dfb_source_A();

    memmove(buf + 8, buf, 8);

    dfb_sink_int((int)buf[8]);
}
```

---

### DFB122 strcpy buffer

```c
DFB_CASE void case_DFB122_strcpy_buffer(void) {
    char src[16] = {0};
    char dst[16] = {0};

    src[0] = (char)dfb_source_A();
    src[1] = 0;

    strcpy(dst, src);

    dfb_sink_int((int)dst[0]);
}
```

---

### DFB123 memset partial memcpy

이 케이스는 초기화와 부분 복사가 섞인 메모리 API 패턴에서 byte-precise 흐름이 유지되는지 확인한다.

```c
#include <string.h>
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB123_memset_partial_memcpy(void) {
    char src[16] = {0};
    char dst[16];

    memset(dst, 0x7f, sizeof(dst));

    src[0] = (char)dfb_source_A();
    src[1] = (char)dfb_source_B();

    memcpy(dst + 4, src, 1);

    dfb_sink_int((int)dst[4]);
}
```

기대 흐름:

```text
dfb_source_A.ret -> src[0] -> partial memcpy -> dst[4] -> dfb_sink_int.arg0
```

금지 흐름:

```text
dfb_source_B.ret must not reach sink
```

---

## 17.1 Shared Library / Import Cases

이 섹션은 외부 shared library, PLT/GOT, import thunk 경유 데이터 흐름을 검증한다.

단일 실행 파일 내부의 함수 호출과 다르게, import 함수는 Ghidra에서 thunk/import symbol로 보일 수 있으므로 backward slicer가 외부 함수 경계에서 끊기는지 확인할 수 있다.

파일: `importlib/dfbench_importlib.h`

Windows/MSVC에서는 shared library를 빌드하는 쪽과 사용하는 쪽의 import/export 지정이 다르다. 따라서 import library 전용 API 매크로를 별도로 둔다.

```c
#pragma once
#include "dfbench.h"

#if defined(_MSC_VER)
    #if defined(DFB_IMPORTLIB_BUILD)
        #define DFB_IMPORT_API __declspec(dllexport)
    #else
        #define DFB_IMPORT_API __declspec(dllimport)
    #endif
#else
    #define DFB_IMPORT_API __attribute__((visibility("default")))
#endif

DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE int dfb_import_identity(int x);
DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE void dfb_import_write_out(int *out, int value);
```

파일: `importlib/dfbench_importlib.c`

```c
#include "dfbench_importlib.h"

DFB_IMPORT_API DFB_NOINLINE int dfb_import_identity(int x) {
    return x;
}

DFB_IMPORT_API DFB_NOINLINE void dfb_import_write_out(int *out, int value) {
    *out = value;
}
```

파일: `src/cases_import.c`

### DFB130 shared import arg-to-ret

```c
#include "dfbench_sources_sinks.h"
#include "dfbench_importlib.h"

DFB_CASE void case_DFB130_shared_import_arg_to_ret(void) {
    int a = dfb_source_A();
    int r = dfb_import_identity(a);
    dfb_sink_int(r);
}
```

기대 흐름:

```text
dfb_source_A.ret -> imported dfb_import_identity.arg0 -> imported ret -> dfb_sink_int.arg0
```

---

### DFB131 shared import out-param

```c
#include "dfbench_sources_sinks.h"
#include "dfbench_importlib.h"

DFB_CASE void case_DFB131_shared_import_outparam(void) {
    int local = 0;
    int a = dfb_source_A();

    dfb_import_write_out(&local, a);

    dfb_sink_int(local);
}
```

기대 흐름:

```text
dfb_source_A.ret -> imported dfb_import_write_out.arg1 -> imported arg0* -> local -> dfb_sink_int.arg0
```

주의:

```text
이 케이스는 shared library build에서만 의미가 있다.
static link 또는 LTO로 import 경계가 사라지면 일반 interprocedural case처럼 보일 수 있다.
```


---

## 18. Expected JSON 스키마

파일: `expected/dfbench_win_core.expected.json`

expected metadata는 실행파일별로 분리한다.

분석기 결과와 비교하기 위한 최소 스키마는 다음과 같다.

```json
{
  "schema_version": 1,
    "program": "dfbench_win_core",
  "cases": [
    {
      "id": "DFB001",
      "function": "case_DFB001_direct_value",
      "anchor": {
        "callee": "dfb_sink_int",
        "arg_index": 0
      },
      "expected_sources": [
        "dfb_source_A.ret"
      ],
      "forbidden_sources": [],
      "expected_features": [
        "direct_value"
      ],
      "allowed_warnings": []
    }
  ]
}
```

필드 설명:

```text
id:
  케이스 ID

function:
  케이스 함수명

anchor.callee:
  backward slice를 시작할 sink 함수

anchor.arg_index:
  sink의 몇 번째 인자를 추적할지 지정

expected_sources:
  반드시 도달해야 하는 source 목록

forbidden_sources:
  도달하면 안 되는 source 목록

expected_features:
  분석기가 evidence로 남기면 좋은 기능 태그

allowed_warnings:
  이 케이스에서 허용 가능한 경고

binary:
    이 케이스가 속한 실행파일 이름
```

---

## 19. 정답 메타데이터 단일 원천

정답 메타데이터의 단일 원천은 `manifests/cases_manifest.json`이다.

원칙:

```text
1. cases_manifest.json이 canonical source이다.
2. 실행파일별 g_cases[] registry는 manifest에서 생성한다.
3. 실행파일별 expected JSON도 manifest에서 생성한다.
4. 문서/검증/CI에서 사용하는 케이스 목록도 manifest를 기준으로 파생한다.
5. 사람이 직접 동시에 수정해야 하는 정답 원본은 manifest 하나만 둔다.
```

예시 구조:

```json
{
    "schema_version": 1,
    "binaries": [
    {
            "name": "dfbench_win_core",
            "platform": "windows",
            "kind": "core_c"
    },
    {
            "name": "dfbench_posix_runtime",
            "platform": "posix",
            "kind": "runtime_bridge"
    },
    {
            "name": "dfbench_cpp",
            "platform": "windows",
            "kind": "cpp"
    },
    {
            "name": "dfbench_cpp_exceptions",
            "platform": "windows",
            "kind": "cpp_exceptions"
    }
    ],
    "cases": [
        {
            "id": "DFB001",
            "binary": "dfbench_win_core",
            "function": "case_DFB001_direct_value",
            "anchor": {"callee": "dfb_sink_int", "arg_index": 0},
            "expected_sources": ["dfb_source_A.ret"],
            "forbidden_sources": [],
            "expected_features": ["direct_value"],
            "allowed_warnings": []
        },
        {
            "id": "DFB090",
            "binary": "dfbench_posix_runtime",
            "function": "case_DFB090_thread_shared_memory",
            "anchor": {"callee": "dfb_sink_int", "arg_index": 0},
            "expected_sources": ["dfb_source_A.ret"],
            "forbidden_sources": [],
            "expected_features": ["thread_shared_memory"],
            "allowed_warnings": []
        }
  ]
}
```

생성 규칙:

```text
1. 각 expected/*.expected.json 파일은 해당 실행파일의 registry와 정확히 동일한 케이스 집합을 가져야 한다.
2. manifest에 존재하는 모든 케이스는 정확히 하나의 binary에 속해야 한다.
3. 어떤 케이스도 두 개 이상의 binary에 중복 배치되면 안 된다.
4. registry에 존재하지만 manifest 또는 대응 expected JSON에 없는 케이스는 오류다.
5. manifest에는 존재하지만 registry에 반영되지 않은 케이스도 오류다.
```

---

## 20. PASS / FAIL / WARN 기준

DataFlowBench 자체는 분석기가 아니므로, 직접 PASS/FAIL을 판정하지 않는다.

하지만 외부 비교기가 따라야 할 기준은 문서화한다.

### PASS

다음 조건을 모두 만족하면 PASS.

```text
1. expected_sources가 모두 actual_sources에 존재한다.
2. forbidden_sources가 actual_sources에 존재하지 않는다.
3. 분석기가 timeout 또는 crash를 내지 않았다.
```

### FAIL

다음 중 하나라도 해당하면 FAIL.

```text
1. expected_sources 중 하나 이상이 누락되었다.
2. forbidden_sources 중 하나 이상이 발견되었다.
3. anchor sink를 찾지 못했다.
4. 케이스 함수가 바이너리에 존재하지 않는다.
5. 분석기가 crash 했다.
```

### WARN

다음은 WARN으로 처리할 수 있다.

```text
1. indirect call target이 unresolved지만 source taint는 sink까지 보존됨
2. virtual call target이 unresolved지만 source taint는 sink까지 보존됨
3. varargs 내부 구조는 못 풀었지만 caller argument source는 보존됨
4. unresolved platform-specific detail이 있으나 required source set은 유지됨
```

### SKIP

다음은 SKIP.

```text
1. 플랫폼에서 지원하지 않는 실행파일 또는 케이스
2. 빌드 옵션상 의도적으로 제외된 실행파일 또는 케이스
3. POSIX 전용 실행파일을 Windows 기준 비교에서 제외
4. C++ exception 전용 실행파일을 C-only 빌드에서 제외
```

---

## 21. CMakeLists.txt 설계

최상위 `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)

project(DataFlowBench C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DFB_ENABLE_THREADS "Enable thread-related cases" ON)
option(DFB_ENABLE_EXCEPTIONS "Enable C++ exception cases" ON)
option(DFB_ENABLE_LTO "Enable link time optimization" OFF)
option(DFB_NO_INLINE "Force noinline-friendly build" ON)

add_library(dfbench_importlib SHARED
    importlib/dfbench_importlib.c
)

target_include_directories(dfbench_importlib PUBLIC include importlib)
target_compile_definitions(dfbench_importlib PRIVATE DFB_IMPORTLIB_BUILD)

add_executable(dfbench_win_core
    src/main.c
    src/dfbench_runtime_win_core.c
    src/dfbench_sources_sinks.c
    src/cases_basic.c
    src/cases_control_flow.c
    src/cases_stack.c
    src/cases_heap.c
    src/cases_struct.c
    src/cases_array.c
    src/cases_global.c
    src/cases_interproc.c
    src/cases_indirect.c
    src/cases_memory_api.c
    src/cases_abi.c
    src/cases_exceptional.c
    src/cases_import.c
)

add_executable(dfbench_posix_runtime
    src/main.c
    src/dfbench_runtime_posix.c
    src/dfbench_sources_sinks.c
    src/cases_runtime_bridge.c
)

add_executable(dfbench_cpp
    src/main.c
    src/dfbench_runtime_cpp.c
    src/dfbench_sources_sinks.c
    cpp/cases_cpp.cpp
)

add_executable(dfbench_cpp_exceptions
    src/main.c
    src/dfbench_runtime_cpp_exceptions.c
    src/dfbench_sources_sinks.c
    cpp/cases_cpp_exceptions.cpp
)

target_include_directories(dfbench_win_core PRIVATE include importlib)
target_include_directories(dfbench_posix_runtime PRIVATE include importlib)
target_include_directories(dfbench_cpp PRIVATE include importlib)
target_include_directories(dfbench_cpp_exceptions PRIVATE include importlib)

target_link_libraries(dfbench_win_core PRIVATE dfbench_importlib)

if (MSVC)
    target_compile_options(dfbench_win_core PRIVATE /W4)
    target_compile_options(dfbench_posix_runtime PRIVATE /W4)
    target_compile_options(dfbench_cpp PRIVATE /W4)
    target_compile_options(dfbench_cpp_exceptions PRIVATE /W4)
    target_compile_options(dfbench_importlib PRIVATE /W4)
else()
    target_compile_options(dfbench_win_core PRIVATE -Wall -Wextra -Wno-unused-function)
    target_compile_options(dfbench_posix_runtime PRIVATE -Wall -Wextra -Wno-unused-function)
    target_compile_options(dfbench_cpp PRIVATE -Wall -Wextra -Wno-unused-function)
    target_compile_options(dfbench_cpp_exceptions PRIVATE -Wall -Wextra -Wno-unused-function)
    target_compile_options(dfbench_importlib PRIVATE -Wall -Wextra -Wno-unused-function)
endif()

if (DFB_NO_INLINE)
    if (MSVC)
        target_compile_options(dfbench_win_core PRIVATE /Ob0)
        target_compile_options(dfbench_posix_runtime PRIVATE /Ob0)
        target_compile_options(dfbench_cpp PRIVATE /Ob0)
        target_compile_options(dfbench_cpp_exceptions PRIVATE /Ob0)
        target_compile_options(dfbench_importlib PRIVATE /Ob0)
    else()
        target_compile_options(dfbench_win_core PRIVATE -fno-inline -fno-inline-functions)
        target_compile_options(dfbench_posix_runtime PRIVATE -fno-inline -fno-inline-functions)
        target_compile_options(dfbench_cpp PRIVATE -fno-inline -fno-inline-functions)
        target_compile_options(dfbench_cpp_exceptions PRIVATE -fno-inline -fno-inline-functions)
        target_compile_options(dfbench_importlib PRIVATE -fno-inline -fno-inline-functions)
    endif()
endif()

if (DFB_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if (ipo_supported)
        set_property(TARGET dfbench_win_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET dfbench_posix_runtime PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET dfbench_cpp PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET dfbench_cpp_exceptions PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET dfbench_importlib PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(dfbench_posix_runtime PRIVATE pthread)
endif()

if (NOT MSVC)
    target_link_options(dfbench_win_core PRIVATE -Wl,--export-dynamic)
    target_link_options(dfbench_posix_runtime PRIVATE -Wl,--export-dynamic)
    target_link_options(dfbench_cpp PRIVATE -Wl,--export-dynamic)
    target_link_options(dfbench_cpp_exceptions PRIVATE -Wl,--export-dynamic)
endif()
```

권장 사항:

```text
- Windows 기본 실행 절차는 CMake Presets 또는 PowerShell 스크립트로 제공한다.
- Bash 스크립트는 보조 수단으로 유지한다.
- Windows 비교 플랫폼에서는 최소한 dfbench_win_core와 dfbench_cpp 계열 산출물을 공식 지원한다.
```

---

## 22. 빌드 스크립트

파일: `scripts/build_all.ps1`

Windows 기본 사용자는 PowerShell 또는 CMake Presets를 사용한다.

```powershell
cmake --preset win-debug
cmake --build --preset win-debug

cmake --preset win-release
cmake --build --preset win-release
```

파일: `scripts/build_all.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

build_variant() {
    local name="$1"
    shift

    cmake -S "$ROOT" -B "$ROOT/build/$name" -G Ninja "$@"
    cmake --build "$ROOT/build/$name"
}

build_variant "O0" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DDFB_ENABLE_LTO=OFF \
    -DDFB_NO_INLINE=ON

build_variant "O2_NOINLINE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DDFB_ENABLE_LTO=OFF \
    -DDFB_NO_INLINE=ON

build_variant "O2_LTO" \
    -DCMAKE_BUILD_TYPE=Release \
    -DDFB_ENABLE_LTO=ON \
    -DDFB_NO_INLINE=OFF

echo "[+] build complete"
```

파일: `scripts/run_all.ps1`

```powershell
$executables = @(
    "build/win-debug/dfbench_win_core.exe",
    "build/win-debug/dfbench_cpp.exe",
    "build/win-debug/dfbench_cpp_exceptions.exe"
)

foreach ($exe in $executables) {
    if (Test-Path $exe) {
        & $exe --list
        & $exe --run-all
    }
}
```

파일: `scripts/run_all.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

for exe in "$ROOT"/build/*/dfbench_win_core "$ROOT"/build/*/dfbench_cpp "$ROOT"/build/*/dfbench_cpp_exceptions; do
    if [[ -x "$exe" ]]; then
        echo "[*] $exe --list"
        "$exe" --list
        echo "[*] $exe --run-all"
        "$exe" --run-all
    fi
done
```

---

## 23. Smoke Test

파일: `tests/smoke_test.py`

```python
#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path

def main():
    root = Path(__file__).resolve().parents[1]
    candidates = []
    candidates.extend(root.glob("build/**/dfbench_win_core"))
    candidates.extend(root.glob("build/**/dfbench_win_core.exe"))
    candidates.extend(root.glob("build/**/dfbench_cpp"))
    candidates.extend(root.glob("build/**/dfbench_cpp.exe"))
    candidates.extend(root.glob("build/**/dfbench_cpp_exceptions"))
    candidates.extend(root.glob("build/**/dfbench_cpp_exceptions.exe"))
    candidates = [p for p in candidates if p.is_file()]

    if not candidates:
        print("no expected dfbench executable found", file=sys.stderr)
        return 1

    for exe in candidates:
        out = subprocess.check_output([str(exe), "--list"], text=True)
        assert "DFB" in out

        out = subprocess.check_output([str(exe), "--run-all"], text=True)
        assert "OK" in out

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
```

---

## 24. 구현 단계

### Phase 1: 프로젝트 골격

완료 조건:

```text
cmake --preset win-debug
cmake --build --preset win-debug
./build/win-debug/dfbench_win_core --list
./build/win-debug/dfbench_win_core --run-all
```

구현 파일:

```text
include/dfbench.h
include/dfbench_cases.h
include/dfbench_sources_sinks.h
src/main.c
src/dfbench_runtime_win_core.c
src/dfbench_sources_sinks.c
manifests/cases_manifest.json
```

### Phase 2: Core C 케이스

구현 케이스:

```text
DFB001
DFB002
DFB003
DFB010
DFB011
DFB012
DFB020
DFB021
DFB022
DFB023
DFB024
DFB025
DFB026
DFB030
DFB031
DFB040
DFB041
DFB042
DFB043
DFB044
DFB045
DFB046
DFB050
DFB051
DFB052
DFB053
DFB054
DFB055
DFB056
DFB057
DFB058
DFB059
DFB060
DFB070
DFB071
DFB072
DFB073
DFB100
DFB101
DFB102
DFB110
DFB120
DFB121
DFB122
DFB123
```

### Phase 3: C++ 케이스

구현 케이스:

```text
DFB080
DFB081
DFB111
```

산출물:

```text
dfbench_cpp
dfbench_cpp_exceptions
```

### Phase 4: 플랫폼 케이스

구현 케이스:

```text
DFB090
DFB091
DFB092
```

정책:

```text
DFB090은 POSIX runtime 전용 실행파일에 둔다.
DFB091은 의미가 유지되면 Windows 기본 실행파일에 둘 수 있다.
```

### Phase 5: Shared Library Import 케이스

구현 케이스:

```text
DFB130
DFB131
```

완료 조건:

```text
dfbench_importlib shared library가 생성된다.
dfbench_win_core가 dfbench_importlib에 동적 링크된다.
DFB130/DFB131이 --run에서 정상 실행된다.
```

### Phase 6: expected JSON 완성

모든 case에 대해 manifest를 작성하고, 실행파일별 expected JSON을 생성한다.

### Phase 7: CI 준비

최소 CI 검증:

```text
build windows debug
build windows release
run dfbench_win_core --list
run dfbench_win_core --run-all
run dfbench_cpp --list
run dfbench_cpp --run-all
verify manifest = registry = expected json
python tests/smoke_test.py
```

---

## 25. Codex 구현 지시 요약

Codex는 다음 순서로 구현한다.

1. 위 저장소 구조 생성
2. `dfbench.h` 작성
3. source/sink 함수 작성
4. cases manifest 작성
5. main CLI 작성
6. runtime registry 생성 규칙 작성
7. C core case 파일 작성
8. C++ case 파일 작성
9. importlib shared library 작성
10. CMakeLists.txt/CMakePresets 작성
11. scripts 작성
12. 실행파일별 expected JSON 생성 규칙 작성
13. smoke_test.py 작성
14. 로컬 빌드가 성공하는지 확인

필수 조건:

```text
- 모든 case 함수는 export/noinline/used 처리
- 모든 source/sink 함수는 export/noinline/used 처리
- 각 실행파일에서 해당 케이스를 호출 가능
- 각 실행파일의 --list와 --run-all은 정상 동작
- Windows 기본 빌드가 우선 지원 대상
- O0/O2_NOINLINE/O2_LTO 빌드 가능
- O2_NOINLINE은 실제로 /Ob0 또는 -fno-inline -fno-inline-functions를 적용
- 의미가 달라지는 POSIX 전용 코드는 Windows fallback으로 대체하지 않음
- C++ 코드는 extern "C"로 case 함수 export
- cases manifest가 정답 원본으로 존재
- 실행파일별 expected JSON은 각 registry와 정확히 일치
- importlib는 Windows에서 dllexport/dllimport를 구분
- BackwardSlicer, Ghidra 분석기, DB, 비교기 엔진은 구현하지 않음
```

---

## 26. 완료 기준

DataFlowBench v1 완료 기준:

```text
1. dfbench_win_core 바이너리가 생성된다.
2. 필요 시 dfbench_posix_runtime, dfbench_cpp, dfbench_cpp_exceptions 바이너리가 생성된다.
3. 각 실행파일의 --list가 해당 DFB 케이스만 출력한다.
4. 각 실행파일의 --run-all이 crash 없이 종료된다.
5. 실행파일별 expected JSON이 존재하며, 대응 registry와 정확히 같은 케이스 집합을 포함한다.
6. 모든 case 함수가 심볼 테이블에 남아 있다.
7. Windows 기준 O0/O2/LTO 빌드가 가능하다.
8. dfbench_importlib shared library가 생성되고 필요한 실행파일에서 사용된다.
9. smoke_test.py가 통과한다.
10. manifest = registry = expected json 일치 검증이 통과한다.
```

이 기준을 만족하면 BackwardSlicer 개발자는 대상 실행파일을 Ghidra에 import해서 각 `case_DFBxxx_*` 함수의 `dfb_sink_*` 호출 인자를 anchor로 backward slice 검증을 수행할 수 있다.
