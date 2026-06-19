#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB001_direct_value(void) {
    int a = dfb_source_A();
    dfb_sink_int(a);
}

DFB_CASE void case_DFB002_arithmetic_value(void) {
    int a = dfb_source_A();
    int b = (a + 3) ^ 0x55;
    dfb_sink_int(b);
}

DFB_CASE void case_DFB003_cast_value(void) {
    int a = dfb_source_A();
    long b = (long)a;
    int c = (int)b;
    dfb_sink_int(c);
}

/* DFB004 — true-negative: sink receives a constant, no source reachable */
DFB_CASE void case_DFB004_no_source_constant(void) {
    int x = 0x1234;
    dfb_sink_int(x);
}

/* DFB005 — strong update: source_A is overwritten by a constant before sink */
DFB_CASE void case_DFB005_overwrite_kill(void) {
    int x = dfb_source_A();
    x = 0x55;           /* must-overwrite: source_A killed */
    dfb_sink_int(x);
}

/* DFB006 — both source_A and source_B reach the sink via arithmetic */
DFB_CASE void case_DFB006_multi_source_convergence(void) {
    int a = dfb_source_A();
    int b = dfb_source_B();
    dfb_sink_int(a + b);
}

/* DFB007 — SUBPIECE (char cast) then zero-extend back to int */
DFB_CASE void case_DFB007_subregister_alias(void) {
    int x  = dfb_source_A();
    char low = (char)x;                 /* SUBPIECE: low 8 bits */
    int  y   = (int)(unsigned char)low; /* INT_ZEXT */
    dfb_sink_int(y);
}
