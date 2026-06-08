# DataFlowBench

Ghidra High PCode 기반 Backward Slice / Data Flow Trace 엔진 검증을 위한 테스트 바이너리 생성 프로젝트.

분석기 개발자는 이 프로젝트가 생성한 바이너리를 Ghidra에 올리고, `expected/*.expected.json`에 기록된 정답과 자신의 분석 결과를 비교하여 엔진 정확도를 측정한다.

> **이 저장소의 범위**: 테스트 바이너리와 정답 메타데이터 생성만 담당한다.
> BackwardSlicer 엔진, Ghidra 분석기, 비교 엔진은 포함하지 않는다.

---

## 빠른 시작

### Windows (네이티브 빌드)

```powershell
# 사전 준비: CMake 3.20+, Ninja, MinGW-w64 or MSVC, Python 3.8+

# 1. 정답 메타데이터 생성
python tools/validate_manifest.py
python tools/generate_registry_from_manifest.py
python tools/generate_expected_from_manifest.py

# 2. 빌드
cmake --preset win-debug
cmake --build --preset win-debug

# 3. 동작 확인
.\build\win-debug\dfbench_win_core.exe --list
.\build\win-debug\dfbench_win_core.exe --run-all   # → OK

# 4. 스모크 테스트
python tests/smoke_test.py
```

### Linux (크로스 빌드 — 전체 케이스 포함)

```bash
# 사전 준비: cmake, ninja, mingw-w64, gcc, python3

# Windows PE 바이너리 크로스컴파일
cmake --preset win-cross-debug
cmake --build --preset win-cross-debug

# POSIX 스레드 케이스 (DFB090, DFB092) 네이티브 빌드
cmake --preset linux-native-debug
cmake --build --preset linux-native-debug

python tests/smoke_test.py
```

---

## 전체 문서

자세한 설계 문서는 [`docs/GUIDE.md`](docs/GUIDE.md)를 참조한다.
