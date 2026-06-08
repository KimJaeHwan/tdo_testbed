#pragma once
#include "dfbench.h"

/* globals already declared with C linkage in dfbench.h */

DFB_EXTERN_C DFB_SOURCE int    dfb_source_A(void);
DFB_EXTERN_C DFB_SOURCE int    dfb_source_B(void);
DFB_EXTERN_C DFB_SOURCE int    dfb_source_C(void);
DFB_EXTERN_C DFB_SOURCE long   dfb_source_long_A(void);
DFB_EXTERN_C DFB_SOURCE char  *dfb_source_buf_A(void);

DFB_EXTERN_C DFB_SINK void dfb_sink_int(int x);
DFB_EXTERN_C DFB_SINK void dfb_sink_long(long x);
DFB_EXTERN_C DFB_SINK void dfb_sink_ptr(void *p);
DFB_EXTERN_C DFB_SINK void dfb_sink_buf(char *p);
