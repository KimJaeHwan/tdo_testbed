#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB043_array_constant_index(void) {
    int arr[4] = {0, 0, 0, 0};
    arr[0] = dfb_source_A();
    arr[2] = dfb_source_B();
    dfb_sink_int(arr[2]);
}

DFB_CASE void case_DFB044_array_variable_index(void) {
    int arr[4] = {0, 0, 0, 0};
    int idx = dfb_source_C() & 3;
    int a   = dfb_source_A();
    arr[idx] = a;
    dfb_sink_int(arr[idx]);
}

typedef struct DFBInner {
    int values[2];
    int extra;
} DFBInner;

typedef struct DFBOuter {
    int      tag;
    DFBInner inner;
} DFBOuter;

DFB_CASE void case_DFB045_nested_aggregate_field(void) {
    DFBOuter obj = {0};
    obj.inner.values[0] = dfb_source_B();
    obj.inner.values[1] = dfb_source_A();
    dfb_sink_int(obj.inner.values[1]);
}
