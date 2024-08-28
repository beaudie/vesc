#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_basic : public testing::Test {
};

TEST_F(test_basic, hostptr) {
	RUN_TEST();
}

TEST_F(test_basic, fpmath) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_int) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_int2) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_int4) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_long) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_long2) {
	RUN_TEST();
}

TEST_F(test_basic, intmath_long4) {
	RUN_TEST();
}

TEST_F(test_basic, hiloeo) {
	RUN_TEST();
}

TEST_F(test_basic, if) {
	RUN_TEST();
}

TEST_F(test_basic, sizeof) {
	RUN_TEST();
}

TEST_F(test_basic, loop) {
	RUN_TEST();
}

TEST_F(test_basic, pointer_cast) {
	RUN_TEST();
}

TEST_F(test_basic, local_arg_def) {
	RUN_TEST();
}

TEST_F(test_basic, local_kernel_def) {
	RUN_TEST();
}

TEST_F(test_basic, local_kernel_scope) {
	RUN_TEST();
}

TEST_F(test_basic, constant) {
	RUN_TEST();
}

TEST_F(test_basic, constant_source) {
	RUN_TEST();
}

TEST_F(test_basic, readimage) {
	RUN_TEST();
}

TEST_F(test_basic, readimage_int16) {
	RUN_TEST();
}

TEST_F(test_basic, readimage_fp32) {
	RUN_TEST();
}

TEST_F(test_basic, writeimage) {
	RUN_TEST();
}

TEST_F(test_basic, writeimage_int16) {
	RUN_TEST();
}

TEST_F(test_basic, writeimage_fp32) {
	RUN_TEST();
}

TEST_F(test_basic, mri_one) {
	RUN_TEST();
}

TEST_F(test_basic, mri_multiple) {
	RUN_TEST();
}

TEST_F(test_basic, image_r8) {
	RUN_TEST();
}

TEST_F(test_basic, barrier) {
	RUN_TEST();
}

TEST_F(test_basic, wg_barrier) {
	RUN_TEST();
}

TEST_F(test_basic, int2fp) {
	RUN_TEST();
}

TEST_F(test_basic, fp2int) {
	RUN_TEST();
}

TEST_F(test_basic, imagereadwrite) {
	RUN_TEST();
}

TEST_F(test_basic, imagereadwrite3d) {
	RUN_TEST();
}

TEST_F(test_basic, readimage3d) {
	RUN_TEST();
}

TEST_F(test_basic, readimage3d_int16) {
	RUN_TEST();
}

TEST_F(test_basic, readimage3d_fp32) {
	RUN_TEST();
}

TEST_F(test_basic, bufferreadwriterect) {
	RUN_TEST();
}

TEST_F(test_basic, arrayreadwrite) {
	RUN_TEST();
}

TEST_F(test_basic, arraycopy) {
	RUN_TEST();
}

TEST_F(test_basic, imagearraycopy) {
	RUN_TEST();
}

TEST_F(test_basic, imagearraycopy3d) {
	RUN_TEST();
}

TEST_F(test_basic, imagecopy) {
	RUN_TEST();
}

TEST_F(test_basic, imagecopy3d) {
	RUN_TEST();
}

TEST_F(test_basic, imagerandomcopy) {
	RUN_TEST();
}

TEST_F(test_basic, arrayimagecopy) {
	RUN_TEST();
}

TEST_F(test_basic, arrayimagecopy3d) {
	RUN_TEST();
}

TEST_F(test_basic, imagenpot) {
	RUN_TEST();
}

TEST_F(test_basic, vload_global) {
	RUN_TEST();
}

TEST_F(test_basic, vload_local) {
	RUN_TEST();
}

TEST_F(test_basic, vload_constant) {
	RUN_TEST();
}

TEST_F(test_basic, vload_private) {
	RUN_TEST();
}

TEST_F(test_basic, vstore_global) {
	RUN_TEST();
}

TEST_F(test_basic, vstore_local) {
	RUN_TEST();
}

TEST_F(test_basic, vstore_private) {
	RUN_TEST();
}

TEST_F(test_basic, createkernelsinprogram) {
	RUN_TEST();
}

TEST_F(test_basic, imagedim_pow2) {
	RUN_TEST();
}

TEST_F(test_basic, imagedim_non_pow2) {
	RUN_TEST();
}

TEST_F(test_basic, image_param) {
	RUN_TEST();
}

TEST_F(test_basic, image_multipass_integer_coord) {
	RUN_TEST();
}

TEST_F(test_basic, image_multipass_float_coord) {
	RUN_TEST();
}

TEST_F(test_basic, explicit_s2v) {
	RUN_TEST();
}

TEST_F(test_basic, enqueue_map_buffer) {
	RUN_TEST();
}

TEST_F(test_basic, enqueue_map_image) {
	RUN_TEST();
}

TEST_F(test_basic, work_item_functions) {
	RUN_TEST();
}

TEST_F(test_basic, astype) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_global_to_local) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_local_to_global) {
	RUN_TEST();
}

TEST_F(test_basic, async_strided_copy_global_to_local) {
	RUN_TEST();
}

TEST_F(test_basic, async_strided_copy_local_to_global) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_global_to_local2D) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_local_to_global2D) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_global_to_local3D) {
	RUN_TEST();
}

TEST_F(test_basic, async_copy_local_to_global3D) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_local) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_global) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_global_and_local) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_local) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_global) {
	RUN_TEST();
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_global_and_local) {
	RUN_TEST();
}

TEST_F(test_basic, prefetch) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_call_kernel_function) {
	RUN_TEST();
}

TEST_F(test_basic, host_numeric_constants) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_numeric_constants) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_limit_constants) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_preprocessor_macros) {
	RUN_TEST();
}

TEST_F(test_basic, parameter_types) {
	RUN_TEST();
}

TEST_F(test_basic, vector_creation) {
	RUN_TEST();
}

TEST_F(test_basic, vector_swizzle) {
	RUN_TEST();
}

TEST_F(test_basic, vec_type_hint) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_memory_alignment_local) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_memory_alignment_global) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_memory_alignment_constant) {
	RUN_TEST();
}

TEST_F(test_basic, kernel_memory_alignment_private) {
	RUN_TEST();
}

TEST_F(test_basic, progvar_prog_scope_misc) {
	RUN_TEST();
}

TEST_F(test_basic, progvar_prog_scope_uninit) {
	RUN_TEST();
}

TEST_F(test_basic, progvar_prog_scope_init) {
	RUN_TEST();
}

TEST_F(test_basic, progvar_func_scope) {
	RUN_TEST();
}

TEST_F(test_basic, global_work_offsets) {
	RUN_TEST();
}

TEST_F(test_basic, get_global_offset) {
	RUN_TEST();
}

TEST_F(test_basic, global_linear_id) {
	RUN_TEST();
}

TEST_F(test_basic, local_linear_id) {
	RUN_TEST();
}

TEST_F(test_basic, enqueued_local_size) {
	RUN_TEST();
}

TEST_F(test_basic, simple_read_image_pitch) {
	RUN_TEST();
}

TEST_F(test_basic, simple_write_image_pitch) {
	RUN_TEST();
}

TEST_F(test_basic, queue_priority) {
	RUN_TEST();
}

TEST_F(test_basic, get_linear_ids) {
	RUN_TEST();
}

TEST_F(test_basic, rw_image_access_qualifier) {
	RUN_TEST();
}
