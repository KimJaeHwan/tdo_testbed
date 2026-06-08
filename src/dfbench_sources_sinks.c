#include "dfbench_sources_sinks.h"

volatile int       g_dfb_sink_int    = 0;
volatile long      g_dfb_sink_long   = 0;
volatile uintptr_t g_dfb_sink_ptr    = 0;
volatile int       g_dfb_source_seed = 0x12345678;

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
