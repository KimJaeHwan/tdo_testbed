#include "dfbench_sources_sinks.h"

static volatile int g_cpp_exception_value = 0;

static void dfb_cpp_throw_writer(int x) {
    g_cpp_exception_value = x;
    throw 1;
}

extern "C" DFB_CASE void case_DFB111_cpp_exception_flow(void) {
    int a = dfb_source_A();
    try {
        dfb_cpp_throw_writer(a);
    } catch (...) {
        dfb_sink_int(g_cpp_exception_value);
    }
}
