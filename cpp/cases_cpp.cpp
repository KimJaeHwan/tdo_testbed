#include "dfbench_sources_sinks.h"

/* DFB080: virtual call */
class DFBBase {
public:
    virtual ~DFBBase() {}
    virtual int get(int x) {
        return x;
    }
};

class DFBDerived : public DFBBase {
public:
    int get(int x) override {
        return x + 100;
    }
};

extern "C" DFB_CASE void case_DFB080_cpp_virtual_call(void) {
    DFBDerived d;
    DFBBase *b = &d;
    int a = dfb_source_A();
    int r = b->get(a);
    dfb_sink_int(r);
}

/* DFB081: lambda capture */
extern "C" DFB_CASE void case_DFB081_cpp_lambda_capture(void) {
    int a = dfb_source_A();
    auto fn = [a]() -> int {
        return a;
    };
    dfb_sink_int(fn());
}
