#include <stdlib.h>
#include "dfbench_sources_sinks.h"

typedef struct DFBHeapObj {
    int value;
    int other;
} DFBHeapObj;

DFB_CASE void case_DFB030_heap_field(void) {
    DFBHeapObj *obj = (DFBHeapObj *)malloc(sizeof(DFBHeapObj));
    if (!obj) {
        return;
    }

    obj->value = dfb_source_A();
    obj->other = dfb_source_B();

    dfb_sink_int(obj->value);

    free(obj);
}

DFB_CASE void case_DFB031_heap_realloc_preserve(void) {
    int *buf = (int *)malloc(sizeof(int) * 2);
    if (!buf) {
        return;
    }

    buf[0] = dfb_source_A();
    buf[1] = dfb_source_B();

    buf = (int *)realloc(buf, sizeof(int) * 4);
    if (!buf) {
        return;
    }

    dfb_sink_int(buf[0]);

    free(buf);
}
