#include "dfbench_sources_sinks.h"

static volatile int g_dfb_global_value = 0;

DFB_CASE void case_DFB024_global_value_flow(void) {
    int a = dfb_source_A();
    g_dfb_global_value = a;
    dfb_sink_int(g_dfb_global_value);
}

typedef struct DFBGlobalPair {
    int left;
    int right;
} DFBGlobalPair;

static volatile DFBGlobalPair g_dfb_global_pair;

DFB_CASE void case_DFB025_global_field_precise(void) {
    g_dfb_global_pair.left  = dfb_source_A();
    g_dfb_global_pair.right = dfb_source_B();
    dfb_sink_int(g_dfb_global_pair.right);
}

static volatile int g_dfb_global_main_value   = 0;
static volatile int g_dfb_global_shadow_value  = 0;

DFB_HELPER void dfb_write_global_values(void) {
    g_dfb_global_main_value  = dfb_source_A();
    g_dfb_global_shadow_value = dfb_source_B();
}

DFB_HELPER int dfb_read_global_main_value(void) {
    return g_dfb_global_main_value;
}

DFB_CASE void case_DFB026_global_interproc_reader(void) {
    dfb_write_global_values();
    dfb_sink_int(dfb_read_global_main_value());
}

/* DFB027 — read-only global (no runtime write); policy case for global classification */
static const int g_dfb_ro_config = 0x9999;

DFB_CASE void case_DFB027_global_readonly_source(void) {
    dfb_sink_int(g_dfb_ro_config);
    /* ground truth: {} — no dfb_source_* involved.
     * Policy case: tests whether the slicer treats a read-only global as a
     * "global_as_source" candidate or as a constant (§16.3). */
}
