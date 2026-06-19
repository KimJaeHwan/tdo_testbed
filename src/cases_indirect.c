#include "dfbench_sources_sinks.h"

/* DFB070 */
typedef int (*dfb_int_fn_t)(int);

DFB_HELPER int dfb_fp_target(int x) {
    return x + 10;
}

DFB_CASE void case_DFB070_function_pointer(void) {
    dfb_int_fn_t fn = dfb_fp_target;
    int a = dfb_source_A();
    int b = fn(a);
    dfb_sink_int(b);
}

/* DFB071 */
static void (*g_dfb_callback)(int) = 0;

DFB_HELPER void dfb_register_callback(void (*cb)(int)) {
    g_dfb_callback = cb;
}

DFB_HELPER void dfb_callback_target(int x) {
    dfb_sink_int(x);
}

DFB_CASE void case_DFB071_callback_registration(void) {
    int a = dfb_source_A();
    dfb_register_callback(dfb_callback_target);
    if (g_dfb_callback) {
        g_dfb_callback(a);
    }
}

/* DFB072 */
DFB_HELPER int dfb_fp_table_target_A(int x) {
    return x + 1;
}

DFB_HELPER int dfb_fp_table_target_B(int x) {
    return x + 2;
}

typedef int (*dfb_table_fn_t)(int);

DFB_CASE void case_DFB072_function_pointer_table(void) {
    dfb_table_fn_t table[2];
    table[0] = dfb_fp_table_target_A;
    table[1] = dfb_fp_table_target_B;

    int selector = dfb_source_C() & 1;
    int a = dfb_source_A();
    int r = table[selector](a);
    dfb_sink_int(r);
}

/* DFB073 */
typedef void (*dfb_sink_wrapper_fn_t)(int);

static void dfb_sink_wrapper_impl(int x) {
    dfb_sink_int(x);
}

DFB_CASE void case_DFB073_indirect_sink_wrapper(void) {
    dfb_sink_wrapper_fn_t fn = dfb_sink_wrapper_impl;
    int a = dfb_source_A();
    int b = dfb_source_B();
    fn(a);
    DFB_TOUCH_INT(b);
}

/* DFB074 — call through an opaque function pointer returned from importlib;
 * the target cannot be resolved statically → WARN unresolved_indirect_call */
#include "dfbench_importlib.h"

DFB_CASE void case_DFB074_unresolved_indirect(void) {
    dfb_int_fn_t fn = (dfb_int_fn_t)dfb_get_opaque_fn();
    int a = dfb_source_A();
    dfb_sink_int(fn(a));   /* ground truth: source_A (call_in candidate) */
}

/* DFB075 — call to a function with no analyzable summary in the slicer's scope;
 * the slicer must emit WARN unresolved_call_boundary and not fabricate through-flow */
DFB_CASE void case_DFB075_unresolved_call_boundary(void) {
    int a = dfb_source_A();
    dfb_sink_int(dfb_external_no_summary(a));
    /* ground truth: source_A (call_in candidate); WARN unresolved_call_boundary */
}
