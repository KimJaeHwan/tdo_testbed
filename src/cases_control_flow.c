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

/* DFB013 — loop-carried churn with unbounded trip count; source_A must survive */
DFB_CASE void case_DFB013_unbounded_loop_widen(void) {
    int x = dfb_source_A();
    int n = dfb_source_B();   /* unbounded trip count */
    for (int i = 0; i < n; i++) {
        x = (x * 31) + i;    /* loop-carried churn */
    }
    dfb_sink_int(x);          /* ground truth: source_A */
}

/* DFB014 — source_C controls the branch; the sink value is always a constant */
DFB_CASE void case_DFB014_control_only_dependency(void) {
    int cond = dfb_source_C();
    int x    = (cond & 1) ? 100 : 200;  /* value is constant; cond is control only */
    dfb_sink_int(x);                     /* data: no source / control: source_C */
}

/* DFB016 — two branches each write a different source to the same memory slot */
DFB_CASE void case_DFB016_memory_phi(void) {
    int buf[1] = {0};
    if (dfb_source_C() & 1)
        buf[0] = dfb_source_A();
    else
        buf[0] = dfb_source_B();
    dfb_sink_int(buf[0]);   /* ground truth: {source_A, source_B} */
}
