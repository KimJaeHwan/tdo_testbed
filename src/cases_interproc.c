#include "dfbench_sources_sinks.h"

/* DFB050 */
DFB_HELPER int dfb_identity_int(int x) {
    return x;
}

DFB_CASE void case_DFB050_identity_call(void) {
    int a = dfb_source_A();
    int b = dfb_identity_int(a);
    dfb_sink_int(b);
}

/* DFB051 */
DFB_HELPER int dfb_transform_int(int x) {
    return (x * 7) + 1;
}

DFB_HELPER int dfb_nested_2(int x) {
    return dfb_transform_int(x);
}

DFB_HELPER int dfb_nested_1(int x) {
    return dfb_nested_2(x);
}

DFB_CASE void case_DFB051_nested_call(void) {
    int a = dfb_source_A();
    int b = dfb_nested_1(a);
    dfb_sink_int(b);
}

/* DFB052 */
DFB_HELPER int dfb_same_identity(int x) {
    return x;
}

DFB_CASE void case_DFB052_callsite_context(void) {
    int a = dfb_source_A();
    int b = dfb_source_B();

    int x = dfb_same_identity(a);
    int y = dfb_same_identity(b);

    dfb_sink_int(x);
    DFB_TOUCH_INT(y);
}

/* DFB053 */
typedef struct DFBBigStruct {
    long a;
    long b;
    long c;
    long d;
} DFBBigStruct;

DFB_HELPER DFBBigStruct dfb_make_big_struct(long x) {
    DFBBigStruct s;
    s.a = x;
    s.b = 0;
    s.c = 0;
    s.d = 0;
    return s;
}

DFB_CASE void case_DFB053_large_struct_return(void) {
    long a = dfb_source_long_A();
    DFBBigStruct s = dfb_make_big_struct(a);
    dfb_sink_long(s.a);
}

/* DFB054 */
DFB_HELPER int dfb_status_out_writer(int *out, int value) {
    *out = value;
    return 0;
}

DFB_CASE void case_DFB054_status_outparam(void) {
    int main_value   = 0;
    int shadow_value = 0;

    DFB_TOUCH_INT(dfb_status_out_writer(&main_value,   dfb_source_A()));
    DFB_TOUCH_INT(dfb_status_out_writer(&shadow_value, dfb_source_B()));

    dfb_sink_int(main_value);
}

/* DFB055 */
typedef struct DFBDeepStruct {
    int aaa;
    int bbb;
    int ccc;
} DFBDeepStruct;

static void dfb_deep_func4(DFBDeepStruct *v) {
    dfb_sink_int(v->aaa);
}

static void dfb_deep_func2(DFBDeepStruct *v) {
    v->bbb = 4;
    v->ccc = 6;
    dfb_deep_func4(v);
}

static void dfb_deep_func1(DFBDeepStruct *v) {
    v->bbb = 2;
    v->ccc = 4;
    dfb_deep_func2(v);
}

DFB_CASE void case_DFB055_deep_field_passthrough(void) {
    DFBDeepStruct val;
    val.aaa = dfb_source_A();
    val.bbb = dfb_source_B();
    val.ccc = dfb_source_C();
    dfb_deep_func1(&val);
}

/* DFB056 */
DFB_HELPER int dfb_summary_arg_to_ret(int x) {
    return x;
}

DFB_CASE void case_DFB056_arg_to_ret_summary(void) {
    int main_value   = dfb_source_A();
    int shadow_value = dfb_source_B();
    int result = dfb_summary_arg_to_ret(main_value);
    DFB_TOUCH_INT(shadow_value);
    dfb_sink_int(result);
}

/* DFB057 */
typedef struct DFBSummaryStruct {
    int chosen;
    int other;
} DFBSummaryStruct;

DFB_HELPER int dfb_summary_field_to_ret(const DFBSummaryStruct *v) {
    return v->chosen;
}

DFB_CASE void case_DFB057_struct_field_to_ret_summary(void) {
    DFBSummaryStruct value;
    value.chosen = dfb_source_A();
    value.other  = dfb_source_B();
    dfb_sink_int(dfb_summary_field_to_ret(&value));
}

/* DFB058 */
DFB_HELPER void dfb_summary_arg_to_out(int *out, int value) {
    *out = value;
}

DFB_CASE void case_DFB058_arg_to_outparam_summary(void) {
    int main_value   = 0;
    int shadow_value = dfb_source_B();
    dfb_summary_arg_to_out(&main_value, dfb_source_A());
    DFB_TOUCH_INT(shadow_value);
    dfb_sink_int(main_value);
}

/* DFB059 */
typedef struct DFBInOutStruct {
    int tracked;
    int noise;
} DFBInOutStruct;

DFB_HELPER void dfb_summary_inout_update(DFBInOutStruct *v, int value) {
    v->tracked = value;
}

DFB_CASE void case_DFB059_inout_field_update_summary(void) {
    DFBInOutStruct value;
    value.tracked = 0;
    value.noise   = dfb_source_B();
    dfb_summary_inout_update(&value, dfb_source_A());
    dfb_sink_int(value.tracked);
}

/* DFB060 */
DFB_HELPER int dfb_recursive_transform(int x, int n) {
    if (n <= 0) {
        return x;
    }
    return dfb_recursive_transform(x + 1, n - 1);
}

DFB_CASE void case_DFB060_recursion(void) {
    int a = dfb_source_A();
    int b = dfb_recursive_transform(a, 3);
    dfb_sink_int(b);
}

/* DFB066 — swap via two out-params; x receives source_B after the swap */
DFB_HELPER void dfb_swap(int *p, int *q) {
    int t = *p; *p = *q; *q = t;
}

DFB_CASE void case_DFB066_swap_target_independence(void) {
    int x = dfb_source_A();
    int y = dfb_source_B();
    dfb_swap(&x, &y);
    dfb_sink_int(x);      /* ground truth: source_B (was in y before swap) */
    DFB_TOUCH_INT(y);     /* y = source_A after swap */
}
