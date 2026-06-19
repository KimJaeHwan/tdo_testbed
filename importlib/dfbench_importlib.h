#pragma once
#include "dfbench.h"

#if defined(_MSC_VER)
    #if defined(DFB_IMPORTLIB_BUILD)
        #define DFB_IMPORT_API __declspec(dllexport)
    #else
        #define DFB_IMPORT_API __declspec(dllimport)
    #endif
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #if defined(DFB_IMPORTLIB_BUILD)
        #define DFB_IMPORT_API __declspec(dllexport)
    #else
        #define DFB_IMPORT_API __declspec(dllimport)
    #endif
#else
    #define DFB_IMPORT_API __attribute__((visibility("default")))
#endif

DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE int  dfb_import_identity(int x);
DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE void dfb_import_write_out(int *out, int value);

/* DFB074: returns a function pointer — callers dispatch via CALLIND */
typedef int (*dfb_opaque_fn_t)(int);
DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE dfb_opaque_fn_t dfb_get_opaque_fn(void);

/* DFB075: opaque passthrough — slicer has no summary for this function */
DFB_EXTERN_C DFB_IMPORT_API DFB_NOINLINE int dfb_external_no_summary(int x);
