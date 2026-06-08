#include <setjmp.h>
#include "dfbench_sources_sinks.h"

static jmp_buf g_jmp_buf;
static volatile int g_jmp_value = 0;

DFB_HELPER void dfb_longjmp_writer(int x) {
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
