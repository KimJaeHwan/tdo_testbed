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

#define DFB_CASE   DFB_EXPORT DFB_NOINLINE DFB_USED
#define DFB_SOURCE DFB_EXPORT DFB_NOINLINE DFB_USED
#define DFB_SINK   DFB_EXPORT DFB_NOINLINE DFB_USED
#define DFB_HELPER DFB_EXPORT DFB_NOINLINE DFB_USED

#if defined(__cplusplus)
    #define DFB_EXTERN_C extern "C"
#else
    #define DFB_EXTERN_C
#endif

#if defined(__cplusplus)
extern "C" {
#endif

extern volatile int       g_dfb_sink_int;
extern volatile long      g_dfb_sink_long;
extern volatile uintptr_t g_dfb_sink_ptr;
extern volatile int       g_dfb_source_seed;

#if defined(__cplusplus)
}
#endif

#define DFB_TOUCH_INT(x)  do { g_dfb_sink_int  ^= (int)(x); } while (0)
#define DFB_TOUCH_LONG(x) do { g_dfb_sink_long ^= (long)(x); } while (0)
#define DFB_TOUCH_PTR(p)  do { g_dfb_sink_ptr   = (uintptr_t)(p); } while (0)
