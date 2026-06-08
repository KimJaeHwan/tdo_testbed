#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB020_stack_local(void) {
    int local = dfb_source_A();
    int copied = local;
    dfb_sink_int(copied);
}

DFB_HELPER void dfb_write_source_to_out(int *out) {
    *out = dfb_source_A();
}

DFB_CASE void case_DFB021_stack_outparam(void) {
    int local = 0;
    dfb_write_source_to_out(&local);
    dfb_sink_int(local);
}

DFB_HELPER void dfb_copy_arg_to_out(int in, int *out) {
    *out = in;
}

DFB_CASE void case_DFB022_arg_to_outparam(void) {
    int local = 0;
    int a = dfb_source_A();
    dfb_copy_arg_to_out(a, &local);
    dfb_sink_int(local);
}

DFB_HELPER void dfb_store_through_double_pointer(int **pp, int value) {
    **pp = value;
}

DFB_CASE void case_DFB023_double_pointer_outparam(void) {
    int local = 0;
    int *p = &local;
    int a = dfb_source_A();
    dfb_store_through_double_pointer(&p, a);
    dfb_sink_int(local);
}
