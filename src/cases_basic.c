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
