#include "dfbench_importlib.h"

DFB_IMPORT_API DFB_NOINLINE int dfb_import_identity(int x) {
    return x;
}

DFB_IMPORT_API DFB_NOINLINE void dfb_import_write_out(int *out, int value) {
    *out = value;
}

DFB_IMPORT_API DFB_NOINLINE dfb_opaque_fn_t dfb_get_opaque_fn(void) {
    /* returns dfb_import_identity as a function pointer;
     * the call site in cases_indirect.c dispatches via CALLIND */
    return dfb_import_identity;
}

DFB_IMPORT_API DFB_NOINLINE int dfb_external_no_summary(int x) {
    return x;
}
