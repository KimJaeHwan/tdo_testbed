#pragma once
#include "dfbench.h"

typedef void (*dfb_case_fn_t)(void);

typedef struct dfb_case_entry_t {
    const char    *id;
    const char    *name;
    dfb_case_fn_t  fn;
} dfb_case_entry_t;

DFB_EXTERN_C const dfb_case_entry_t *dfb_get_cases(size_t *count);
