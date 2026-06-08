#include <stddef.h>
#include <stdint.h>
#include "dfbench_sources_sinks.h"

typedef struct DFBPoint {
    int x;
    int y;
} DFBPoint;

DFB_CASE void case_DFB040_struct_field_precise(void) {
    DFBPoint p;
    p.x = dfb_source_A();
    p.y = dfb_source_B();
    dfb_sink_int(p.y);
}

typedef struct DFBOffsetObj {
    int pad;
    int value;
} DFBOffsetObj;

DFB_CASE void case_DFB041_pointer_arithmetic_field(void) {
    DFBOffsetObj obj;
    char *base = (char *)&obj;
    *((int *)(base + offsetof(DFBOffsetObj, value))) = dfb_source_A();
    dfb_sink_int(obj.value);
}

typedef union DFBUnion {
    int   i;
    float f;
} DFBUnion;

DFB_CASE void case_DFB042_union_alias(void) {
    DFBUnion u;
    u.i = dfb_source_A();
    dfb_sink_int(u.i);
}

typedef union DFBPartialWord {
    uint32_t whole;
    uint8_t  bytes[4];
} DFBPartialWord;

DFB_CASE void case_DFB046_partial_overwrite_subfield(void) {
    DFBPartialWord word;
    word.whole    = 0x11223344U;
    word.bytes[0] = (uint8_t)dfb_source_A();
    word.bytes[1] = (uint8_t)dfb_source_B();
    dfb_sink_int((int)(word.whole & 0xff));
}
