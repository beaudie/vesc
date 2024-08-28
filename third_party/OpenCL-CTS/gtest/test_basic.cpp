#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_basic : public testing::Test
{};

TEST_F(test_basic, hostptr)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, fpmath)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_int)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_int2)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_int4)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_long)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_long2)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, intmath_long4)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, hiloeo)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, if)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, sizeof)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, loop)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, pointer_cast)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, local_arg_def)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, local_kernel_def)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, local_kernel_scope)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, constant)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, constant_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage_int16)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage_fp32)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, writeimage)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, writeimage_int16)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, writeimage_fp32)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, mri_one)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, mri_multiple)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, image_r8)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, barrier)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, wg_barrier)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, int2fp)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, fp2int)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagereadwrite)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagereadwrite3d)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage3d)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage3d_int16)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, readimage3d_fp32)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, bufferreadwriterect)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, arrayreadwrite)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, arraycopy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagearraycopy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagearraycopy3d)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagecopy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagecopy3d)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagerandomcopy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, arrayimagecopy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, arrayimagecopy3d)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagenpot)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vload_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vload_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vload_constant)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vload_private)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vstore_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vstore_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vstore_private)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, createkernelsinprogram)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagedim_pow2)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, imagedim_non_pow2)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, image_param)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, image_multipass_integer_coord)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, image_multipass_float_coord)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, explicit_s2v)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, enqueue_map_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, enqueue_map_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, work_item_functions)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, astype)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_global_to_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_local_to_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_strided_copy_global_to_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_strided_copy_local_to_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_global_to_local2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_local_to_global2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_global_to_local3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_copy_local_to_global3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_import_after_export_aliased_global_and_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, async_work_group_copy_fence_export_after_import_aliased_global_and_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, prefetch)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_call_kernel_function)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, host_numeric_constants)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_numeric_constants)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_limit_constants)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_preprocessor_macros)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, parameter_types)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vector_creation)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vector_swizzle)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, vec_type_hint)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_memory_alignment_local)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_memory_alignment_global)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_memory_alignment_constant)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, kernel_memory_alignment_private)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, progvar_prog_scope_misc)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, progvar_prog_scope_uninit)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, progvar_prog_scope_init)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, progvar_func_scope)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, global_work_offsets)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, get_global_offset)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, global_linear_id)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, local_linear_id)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, enqueued_local_size)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, simple_read_image_pitch)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, simple_write_image_pitch)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, queue_priority)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, get_linear_ids)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_basic, rw_image_access_qualifier)
{
    runTest(::testing::UnitTest::GetInstance());
}
