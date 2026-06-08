cmake_minimum_required(VERSION 3.20)
project(DataFlowBench C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DFB_ENABLE_THREADS   "Enable thread-related cases (POSIX)"    ON)
option(DFB_ENABLE_EXCEPTIONS "Enable C++ exception cases"            ON)
option(DFB_ENABLE_LTO       "Enable link time optimization"          OFF)
option(DFB_NO_INLINE        "Force noinline-friendly build"          ON)
option(DFB_TARGET_POSIX     "Build POSIX runtime target"             OFF)

# importlib shared library
add_library(dfbench_importlib SHARED
    importlib/dfbench_importlib.c
)
target_include_directories(dfbench_importlib PUBLIC include importlib)
target_compile_definitions(dfbench_importlib PRIVATE DFB_IMPORTLIB_BUILD)

# dfbench_win_core — Windows core C cases
# NOTE: cases_runtime_bridge.c is included here for DFB091 (TLS).
#       DFB090/DFB092 inside that file are guarded by #if !defined(_WIN32).
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
    src/cases_runtime_bridge.c
    src/cases_import.c
    src/cases_obfuscation.c
)
target_link_libraries(dfbench_win_core PRIVATE dfbench_importlib)

# dfbench_cpp — C++ virtual call and lambda cases
add_executable(dfbench_cpp
    src/main.c
    src/dfbench_runtime_cpp.c
    src/dfbench_sources_sinks.c
    cpp/cases_cpp.cpp
)

# dfbench_cpp_exceptions — C++ exception flow cases (isolated)
if(DFB_ENABLE_EXCEPTIONS)
    add_executable(dfbench_cpp_exceptions
        src/main.c
        src/dfbench_runtime_cpp_exceptions.c
        src/dfbench_sources_sinks.c
        cpp/cases_cpp_exceptions.cpp
    )
endif()

# dfbench_posix_runtime — POSIX thread cases (Linux native or cross)
if(DFB_TARGET_POSIX AND DFB_ENABLE_THREADS)
    add_executable(dfbench_posix_runtime
        src/main.c
        src/dfbench_runtime_posix.c
        src/dfbench_sources_sinks.c
        src/cases_runtime_bridge.c
    )
    target_link_libraries(dfbench_posix_runtime PRIVATE pthread)
elseif(UNIX AND NOT APPLE AND NOT DFB_TARGET_POSIX AND DFB_ENABLE_THREADS)
    # Auto-detect native Linux build when DFB_TARGET_POSIX not explicitly set
    add_executable(dfbench_posix_runtime
        src/main.c
        src/dfbench_runtime_posix.c
        src/dfbench_sources_sinks.c
        src/cases_runtime_bridge.c
    )
    target_link_libraries(dfbench_posix_runtime PRIVATE pthread)
endif()

# Collect all targets
set(DFB_EXECUTABLE_TARGETS dfbench_win_core dfbench_cpp)

if(TARGET dfbench_cpp_exceptions)
    list(APPEND DFB_EXECUTABLE_TARGETS dfbench_cpp_exceptions)
endif()
if(TARGET dfbench_posix_runtime)
    list(APPEND DFB_EXECUTABLE_TARGETS dfbench_posix_runtime)
endif()

set(DFB_ALL_TARGETS ${DFB_EXECUTABLE_TARGETS} dfbench_importlib)

# Include directories
foreach(target_name IN LISTS DFB_EXECUTABLE_TARGETS)
    target_include_directories(${target_name} PRIVATE include importlib)
endforeach()

# Compiler warnings
if(MSVC)
    foreach(target_name IN LISTS DFB_ALL_TARGETS)
        target_compile_options(${target_name} PRIVATE /W4)
    endforeach()
else()
    foreach(target_name IN LISTS DFB_ALL_TARGETS)
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wno-unused-function)
    endforeach()
endif()

# Force noinline + strict-aliasing safety
# -fno-strict-aliasing: protects DFB041 (pointer arithmetic through char*)
#   where the compiler could otherwise discard the write as a non-aliasing store.
# -fno-inline*: belt-and-suspenders on top of per-function __attribute__((noinline)).
if(DFB_NO_INLINE)
    if(MSVC)
        foreach(target_name IN LISTS DFB_ALL_TARGETS)
            target_compile_options(${target_name} PRIVATE /Ob0)
        endforeach()
    else()
        foreach(target_name IN LISTS DFB_ALL_TARGETS)
            target_compile_options(${target_name} PRIVATE
                -fno-inline
                -fno-inline-functions
                -fno-strict-aliasing
            )
        endforeach()
    endif()
endif()

# Cap Release builds at -O2 (GCC/Clang default for Release is -O3 which enables
# more aggressive value-propagation that can collapse multi-step data flows).
# This keeps the "O2_NOINLINE" variant from design doc.
if(NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    foreach(target_name IN LISTS DFB_ALL_TARGETS)
        target_compile_options(${target_name} PRIVATE -O2)
    endforeach()
endif()


# LTO
if(DFB_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        foreach(target_name IN LISTS DFB_ALL_TARGETS)
            set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        endforeach()
    endif()
endif()

# Export all symbols (non-MSVC)
if(NOT MSVC)
    if(WIN32)
        foreach(target_name IN LISTS DFB_EXECUTABLE_TARGETS)
            target_link_options(${target_name} PRIVATE -Wl,--export-all-symbols)
        endforeach()
    else()
        foreach(target_name IN LISTS DFB_EXECUTABLE_TARGETS)
            target_link_options(${target_name} PRIVATE -Wl,--export-dynamic)
        endforeach()
    endif()
endif()
