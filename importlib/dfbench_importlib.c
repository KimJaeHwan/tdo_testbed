#include "dfbench_importlib.h"

DFB_IMPORT_API DFB_NOINLINE int dfb_import_identity(int x) {
    return x;
}

DFB_IMPORT_API DFB_NOINLINE void dfb_import_write_out(int *out, int value) {
    *out = value;
}
