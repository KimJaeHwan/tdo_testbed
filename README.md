# DataFlowBench

Ghidra High PCode 기반 Backward Slice / Data Flow Trace 엔진 검증을 위한 테스트 바이너리 생성 프로젝트.

분석기 개발자는 이 프로젝트가 생성한 바이너리를 Ghidra에 올리고, `expected/*.expected.json`에 기록된 정답과 자신의 분석 결과를 비교하여 엔진 정확도를 측정한다.

> **이 저장소의 범위**: 테스트 바이너리와 정답 메타데이터 생성만 담당한다.
> BackwardSlicer 엔진, Ghidra 분석기, 비교 엔진은 포함하지 않는다.

---

## 빠른 시작

### 빌드 없이 사용

GitHub Releases에서 미리 빌드된 산출물을 받을 수 있다.

- `dfbench-linux-elf-multiarch.tar.gz`: Linux ELF x86, x64, ARM, ARM64
- `dfbench-windows-pe-x86-x64.tar.gz`: Windows PE x86, x64
- `SHA256SUMS`: 릴리스 asset 체크섬

각 압축 파일에는 플랫폼별 `build/`, `expected/`, `build-info.txt`, `artifact-file-types.txt`가 포함된다.

### macOS / Docker — Linux multi-arch 빌드

Docker Desktop이 설치되어 있으면 macOS에서도 x86, x64, ARM, ARM64 Linux ELF 바이너리를 한 번에 빌드할 수 있다.

```bash
./scripts/docker_build_matrix.sh
```

기본 플랫폼 매핑:

| 이름 | Docker platform |
|---|---|
| x86 | `linux/386` |
| x64 | `linux/amd64` |
| arm | `linux/arm/v7` |
| arm64 | `linux/arm64` |

산출물은 기본적으로 `dist/docker-linux/` 아래에 플랫폼별 디렉터리로 생성된다.
각 플랫폼 빌드는 컨테이너 내부에서 manifest 검증, 런타임 레지스트리 생성, expected JSON 생성, CMake 빌드, smoke test를 수행한다.

```bash
# smoke test를 생략하고 빌드만 수행
DFB_RUN_SMOKE=0 ./scripts/docker_build_matrix.sh

# 플랫폼 일부만 빌드
DFB_DOCKER_PLATFORMS=linux/amd64,linux/arm64 ./scripts/docker_build_matrix.sh
```

### macOS / Windows / Docker — Windows PE 빌드

Docker Desktop의 Linux 컨테이너 모드를 사용하면 macOS와 Windows 양쪽에서 같은 명령으로 Windows PE x86/x64 바이너리를 빌드할 수 있다.

```bash
./scripts/docker_build_pe.sh
```

산출물은 기본적으로 `dist/docker-windows-pe/` 아래에 생성된다.

```bash
# x64 PE만 빌드
DFB_PE_TARGETS=x64 ./scripts/docker_build_pe.sh

# x86 PE만 빌드
DFB_PE_TARGETS=x86 ./scripts/docker_build_pe.sh
```

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
