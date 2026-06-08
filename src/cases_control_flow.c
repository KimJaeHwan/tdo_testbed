#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB010_branch_phi(void) {
    int flag = dfb_source_C() & 1;
    int x;

    if (flag) {
        x = dfb_source_A();
    } else {
        x = dfb_source_B();
    }

    dfb_sink_int(x);
}

DFB_CASE void case_DFB011_loop_phi(void) {
    int x = dfb_source_A();
    int n = (dfb_source_C() & 3) + 1;

    for (int i = 0; i < n; i++) {
        x = x + i;
    }

    dfb_sink_int(x);
}

DFB_CASE void case_DFB012_switch_merge(void) {
    int selector = dfb_source_C() & 3;
    int x;

    switch (selector) {
    case 0:
        x = dfb_source_A();
        break;
    case 1:
        x = dfb_source_B();
        break;
    default:
        x = dfb_source_C();
        break;
    }

    dfb_sink_int(x);
}
