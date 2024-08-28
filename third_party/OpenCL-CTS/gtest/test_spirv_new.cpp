#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_spirv_new : public testing::Test {
};

TEST_F(test_spirv_new, basic_versions) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fadd_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fsub_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fmul_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fshiftleft_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fnegate_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fadd_uint) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fsub_uint) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fmul_uint) {
	RUN_TEST();
}

TEST_F(test_spirv_new, ext_cl_khr_spirv_no_integer_wrap_decoration_fshiftleft_uint) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_restrict) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_aliased) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_alignment) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_constant) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_cpacked) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_half_to_char) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_half_to_uchar) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_float_to_char) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_float_to_uchar) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_float_to_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_float_to_ushort) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_double_to_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_saturated_conversion_double_to_uint) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rte_half_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtz_half_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtp_half_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtn_half_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rte_float_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtz_float_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtp_float_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtn_float_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rte_double_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtz_double_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtp_double_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, decorate_fp_rounding_mode_rtn_double_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, get_program_il) {
	RUN_TEST();
}

TEST_F(test_spirv_new, linkage_export_function_compile) {
	RUN_TEST();
}

TEST_F(test_spirv_new, linkage_import_function_compile) {
	RUN_TEST();
}

TEST_F(test_spirv_new, linkage_import_function_link) {
	RUN_TEST();
}

TEST_F(test_spirv_new, linkage_linkonce_odr) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_atomic_inc_global) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_atomic_dec_global) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_label_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_branch_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_unreachable_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_branch_conditional) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_branch_conditional_weighted) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_composite_construct_int4) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_composite_construct_struct) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_true_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_false_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_int_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_uint_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_uchar_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_ushort_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_long_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_ulong_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_short_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_double_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_int4_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_int3_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_struct_int_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_struct_int_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_struct_struct_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_constant_half_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_int_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_uint_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_uchar_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_ushort_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_long_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_ulong_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_short_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_double_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_int4_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_int3_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_struct_int_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_struct_int_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_struct_struct_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_copy_half_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_float_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_float_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_float_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_float_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_float_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_float_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_float_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_float_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_double_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_double_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_double_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_double_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_double_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_double_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_double_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_double_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_float4_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_float4_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_float4_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_float4_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_float4_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_float4_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_float4_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_float4_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_double2_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_double2_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_double2_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_double2_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_double2_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_double2_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_double2_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_double2_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_half_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_half_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_half_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_half_regular) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fadd_half_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fsub_half_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fmul_half_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_fdiv_half_fast) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_none) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_inline) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_noinline) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_pure) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_const) {
	RUN_TEST();
}

TEST_F(test_spirv_new, function_pure_ptr) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_lifetime_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_none) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_unroll) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_dont_unroll) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_conditional_none) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_conditional_unroll) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_loop_merge_branch_conditional_dont_unroll) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_half) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_float) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_double) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_not_int) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_not_long) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_not_short) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_float4) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_neg_int4) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_not_int4) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_type_opaque_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_phi_2_blocks) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_phi_3_blocks) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_phi_4_blocks) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_if_none) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_if_flatten) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_if_dont_flatten) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_swith_none) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_swith_flatten) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_selection_merge_swith_dont_flatten) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_uint_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_uchar_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_ushort_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_ulong_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_half_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_double_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_true_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_spec_constant_false_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_true_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_false_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_int_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_uint_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_uchar_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_ushort_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_long_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_ulong_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_short_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_double_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_int4_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_int3_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_struct_int_float_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_struct_int_char_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_struct_struct_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_undef_half_simple) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_half8_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_int4_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_float4_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_long2_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_double2_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_char16_extract) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_half8_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_int4_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_float4_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_long2_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_double2_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_char16_insert) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_times_scalar_float) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_times_scalar_double) {
	RUN_TEST();
}

TEST_F(test_spirv_new, op_vector_times_scalar_half) {
	RUN_TEST();
}
