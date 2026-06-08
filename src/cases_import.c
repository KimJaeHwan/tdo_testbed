#include "dfbench_sources_sinks.h"
#include "dfbench_importlib.h"

DFB_CASE void case_DFB130_shared_import_arg_to_ret(void) {
    int a = dfb_source_A();
    int r = dfb_import_identity(a);
    dfb_sink_int(r);
}

DFB_CASE void case_DFB131_shared_import_outparam(void) {
    int local = 0;
    int a = dfb_source_A();
    dfb_import_write_out(&local, a);
    dfb_sink_int(local);
}
