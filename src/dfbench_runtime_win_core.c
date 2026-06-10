/* AUTO-GENERATED — do not edit manually.
 * Regenerate with: python tools/generate_registry_from_manifest.py
 */
#include <stddef.h>
#include "dfbench_cases.h"

/* case function declarations */
DFB_EXTERN_C void case_DFB001_direct_value(void);
DFB_EXTERN_C void case_DFB002_arithmetic_value(void);
DFB_EXTERN_C void case_DFB003_cast_value(void);
DFB_EXTERN_C void case_DFB010_branch_phi(void);
DFB_EXTERN_C void case_DFB011_loop_phi(void);
DFB_EXTERN_C void case_DFB012_switch_merge(void);
DFB_EXTERN_C void case_DFB020_stack_local(void);
DFB_EXTERN_C void case_DFB021_stack_outparam(void);
DFB_EXTERN_C void case_DFB022_arg_to_outparam(void);
DFB_EXTERN_C void case_DFB023_double_pointer_outparam(void);
DFB_EXTERN_C void case_DFB024_global_value_flow(void);
DFB_EXTERN_C void case_DFB025_global_field_precise(void);
DFB_EXTERN_C void case_DFB026_global_interproc_reader(void);
DFB_EXTERN_C void case_DFB030_heap_field(void);
DFB_EXTERN_C void case_DFB031_heap_realloc_preserve(void);
DFB_EXTERN_C void case_DFB040_struct_field_precise(void);
DFB_EXTERN_C void case_DFB041_pointer_arithmetic_field(void);
DFB_EXTERN_C void case_DFB042_union_alias(void);
DFB_EXTERN_C void case_DFB043_array_constant_index(void);
DFB_EXTERN_C void case_DFB044_array_variable_index(void);
DFB_EXTERN_C void case_DFB045_nested_aggregate_field(void);
DFB_EXTERN_C void case_DFB046_partial_overwrite_subfield(void);
DFB_EXTERN_C void case_DFB050_identity_call(void);
DFB_EXTERN_C void case_DFB051_nested_call(void);
DFB_EXTERN_C void case_DFB052_callsite_context(void);
DFB_EXTERN_C void case_DFB053_large_struct_return(void);
DFB_EXTERN_C void case_DFB054_status_outparam(void);
DFB_EXTERN_C void case_DFB055_deep_field_passthrough(void);
DFB_EXTERN_C void case_DFB056_arg_to_ret_summary(void);
DFB_EXTERN_C void case_DFB057_struct_field_to_ret_summary(void);
DFB_EXTERN_C void case_DFB058_arg_to_outparam_summary(void);
DFB_EXTERN_C void case_DFB059_inout_field_update_summary(void);
DFB_EXTERN_C void case_DFB060_recursion(void);
DFB_EXTERN_C void case_DFB070_function_pointer(void);
DFB_EXTERN_C void case_DFB071_callback_registration(void);
DFB_EXTERN_C void case_DFB072_function_pointer_table(void);
DFB_EXTERN_C void case_DFB073_indirect_sink_wrapper(void);
DFB_EXTERN_C void case_DFB091_tls_value(void);
DFB_EXTERN_C void case_DFB100_varargs(void);
DFB_EXTERN_C void case_DFB101_tail_call_candidate(void);
DFB_EXTERN_C void case_DFB102_signed_unsigned_boundary(void);
DFB_EXTERN_C void case_DFB110_setjmp_longjmp(void);
DFB_EXTERN_C void case_DFB120_memcpy_buffer(void);
DFB_EXTERN_C void case_DFB121_memmove_buffer(void);
DFB_EXTERN_C void case_DFB122_strcpy_buffer(void);
DFB_EXTERN_C void case_DFB123_memset_partial_memcpy(void);
DFB_EXTERN_C void case_DFB130_shared_import_arg_to_ret(void);
DFB_EXTERN_C void case_DFB131_shared_import_outparam(void);
DFB_EXTERN_C void case_DFB200_obf_bcf_multistep(void);
DFB_EXTERN_C void case_DFB201_obf_fla_statemachine(void);
DFB_EXTERN_C void case_DFB032_heap_raw_offset(void);
DFB_EXTERN_C void case_DFB034_bitfield_access(void);
DFB_EXTERN_C void case_DFB035_bitfield_access_zeroinit(void);
DFB_EXTERN_C void case_DFB047_struct_padding_offset(void);
DFB_EXTERN_C void case_DFB048_cast_range_overlap(void);
DFB_EXTERN_C void case_DFB049_negative_offset_arithmetic(void);

static const dfb_case_entry_t g_cases[] = {
    {"DFB001", "direct_value", case_DFB001_direct_value},
    {"DFB002", "arithmetic_value", case_DFB002_arithmetic_value},
    {"DFB003", "cast_value", case_DFB003_cast_value},
    {"DFB010", "branch_phi", case_DFB010_branch_phi},
    {"DFB011", "loop_phi", case_DFB011_loop_phi},
    {"DFB012", "switch_merge", case_DFB012_switch_merge},
    {"DFB020", "stack_local", case_DFB020_stack_local},
    {"DFB021", "stack_outparam", case_DFB021_stack_outparam},
    {"DFB022", "arg_to_outparam", case_DFB022_arg_to_outparam},
    {"DFB023", "double_pointer_outparam", case_DFB023_double_pointer_outparam},
    {"DFB024", "global_value_flow", case_DFB024_global_value_flow},
    {"DFB025", "global_field_precise", case_DFB025_global_field_precise},
    {"DFB026", "global_interproc_reader", case_DFB026_global_interproc_reader},
    {"DFB030", "heap_field", case_DFB030_heap_field},
    {"DFB031", "heap_realloc_preserve", case_DFB031_heap_realloc_preserve},
    {"DFB040", "struct_field_precise", case_DFB040_struct_field_precise},
    {"DFB041", "pointer_arithmetic_field", case_DFB041_pointer_arithmetic_field},
    {"DFB042", "union_alias", case_DFB042_union_alias},
    {"DFB043", "array_constant_index", case_DFB043_array_constant_index},
    {"DFB044", "array_variable_index", case_DFB044_array_variable_index},
    {"DFB045", "nested_aggregate_field", case_DFB045_nested_aggregate_field},
    {"DFB046", "partial_overwrite_subfield", case_DFB046_partial_overwrite_subfield},
    {"DFB050", "identity_call", case_DFB050_identity_call},
    {"DFB051", "nested_call", case_DFB051_nested_call},
    {"DFB052", "callsite_context", case_DFB052_callsite_context},
    {"DFB053", "large_struct_return", case_DFB053_large_struct_return},
    {"DFB054", "status_outparam", case_DFB054_status_outparam},
    {"DFB055", "deep_field_passthrough", case_DFB055_deep_field_passthrough},
    {"DFB056", "arg_to_ret_summary", case_DFB056_arg_to_ret_summary},
    {"DFB057", "struct_field_to_ret_summary", case_DFB057_struct_field_to_ret_summary},
    {"DFB058", "arg_to_outparam_summary", case_DFB058_arg_to_outparam_summary},
    {"DFB059", "inout_field_update_summary", case_DFB059_inout_field_update_summary},
    {"DFB060", "recursion", case_DFB060_recursion},
    {"DFB070", "function_pointer", case_DFB070_function_pointer},
    {"DFB071", "callback_registration", case_DFB071_callback_registration},
    {"DFB072", "function_pointer_table", case_DFB072_function_pointer_table},
    {"DFB073", "indirect_sink_wrapper", case_DFB073_indirect_sink_wrapper},
    {"DFB091", "tls_value", case_DFB091_tls_value},
    {"DFB100", "varargs", case_DFB100_varargs},
    {"DFB101", "tail_call_candidate", case_DFB101_tail_call_candidate},
    {"DFB102", "signed_unsigned_boundary", case_DFB102_signed_unsigned_boundary},
    {"DFB110", "setjmp_longjmp", case_DFB110_setjmp_longjmp},
    {"DFB120", "memcpy_buffer", case_DFB120_memcpy_buffer},
    {"DFB121", "memmove_buffer", case_DFB121_memmove_buffer},
    {"DFB122", "strcpy_buffer", case_DFB122_strcpy_buffer},
    {"DFB123", "memset_partial_memcpy", case_DFB123_memset_partial_memcpy},
    {"DFB130", "shared_import_arg_to_ret", case_DFB130_shared_import_arg_to_ret},
    {"DFB131", "shared_import_outparam", case_DFB131_shared_import_outparam},
    {"DFB200", "obf_bcf_multistep", case_DFB200_obf_bcf_multistep},
    {"DFB201", "obf_fla_statemachine", case_DFB201_obf_fla_statemachine},
    {"DFB032", "heap_raw_offset", case_DFB032_heap_raw_offset},
    {"DFB034", "bitfield_access", case_DFB034_bitfield_access},
    {"DFB035", "bitfield_access_zeroinit", case_DFB035_bitfield_access_zeroinit},
    {"DFB047", "struct_padding_offset", case_DFB047_struct_padding_offset},
    {"DFB048", "cast_range_overlap", case_DFB048_cast_range_overlap},
    {"DFB049", "negative_offset_arithmetic", case_DFB049_negative_offset_arithmetic},
};

const dfb_case_entry_t *dfb_get_cases(size_t *count) {
    if (count) {
        *count = sizeof(g_cases) / sizeof(g_cases[0]);
    }
    return g_cases;
}
