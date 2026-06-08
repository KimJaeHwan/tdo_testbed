#include <stdarg.h>
#include <stdint.h>
#include "dfbench_sources_sinks.h"

/* DFB100 */
DFB_HELPER int dfb_pick_first_vararg(const char *fmt, ...) {
    (void)fmt;
    va_list ap;
    va_start(ap, fmt);
    int x = va_arg(ap, int);
    va_end(ap);
    return x;
}

DFB_CASE void case_DFB100_varargs(void) {
    int a = dfb_source_A();
    int b = dfb_pick_first_vararg("%d", a);
    dfb_sink_int(b);
}

/* DFB101 */
DFB_HELPER int dfb_tail_target(int x) {
    return x;
}

DFB_HELPER int dfb_tail_wrapper(int x) {
    return dfb_tail_target(x);
}

DFB_CASE void case_DFB101_tail_call_candidate(void) {
    int a = dfb_source_A();
    int b = dfb_tail_wrapper(a);
    dfb_sink_int(b);
}

/* DFB102 */
DFB_CASE void case_DFB102_signed_unsigned_boundary(void) {
    int a = dfb_source_A();
    int noise = dfb_source_B();
    unsigned int b = (unsigned int)a;
    uint64_t c = (uint64_t)b;
    int d = (int)c;
    DFB_TOUCH_INT(noise);
    dfb_sink_int(d);
}
