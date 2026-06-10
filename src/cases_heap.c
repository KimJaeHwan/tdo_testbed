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

/* DFB032: heap_raw_offset
 * Tests raw pointer arithmetic on a heap-allocated buffer (not a struct cast).
 * malloc returns heap_buf; payload = heap_buf + 20 uses INT_ADD/PTRADD at High PCode.
 * The slicer must identify that STORE(heap_buf+20) and LOAD(heap_buf+20) share the
 * same base varnode + same offset — without a stack-frame (RSP-relative) anchor.
 * Current slicer: heap base varnode tracking is not implemented -> FAIL expected.
 */
DFB_CASE void case_DFB032_heap_raw_offset(void) {
    char *heap_buf = (char *)malloc(100);
    if (!heap_buf) {
        return;
    }

    int *payload = (int *)(heap_buf + 20);   /* raw +20 byte offset into heap buffer */
    *payload = dfb_source_A();

    dfb_sink_int(*(int *)(heap_buf + 20));   /* read back same offset */

    free(heap_buf);
}
