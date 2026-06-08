#include <string.h>
#include "dfbench_sources_sinks.h"

DFB_CASE void case_DFB120_memcpy_buffer(void) {
    char src[16] = {0};
    char dst[16] = {0};
    src[0] = (char)dfb_source_A();
    memcpy(dst, src, sizeof(src));
    dfb_sink_int((int)dst[0]);
}

DFB_CASE void case_DFB121_memmove_buffer(void) {
    char buf[32] = {0};
    buf[0] = (char)dfb_source_A();
    memmove(buf + 8, buf, 8);
    dfb_sink_int((int)buf[8]);
}

DFB_CASE void case_DFB122_strcpy_buffer(void) {
    char src[16] = {0};
    char dst[16] = {0};
    src[0] = (char)dfb_source_A();
    src[1] = 0;
    strcpy(dst, src);
    dfb_sink_int((int)dst[0]);
}

DFB_CASE void case_DFB123_memset_partial_memcpy(void) {
    char src[16] = {0};
    char dst[16];
    memset(dst, 0x7f, sizeof(dst));
    src[0] = (char)dfb_source_A();
    src[1] = (char)dfb_source_B();
    memcpy(dst + 4, src, 1);
    dfb_sink_int((int)dst[4]);
}
