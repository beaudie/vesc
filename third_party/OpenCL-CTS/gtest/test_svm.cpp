#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_svm : public testing::Test
{};

TEST_F(test_svm, svm_byte_granularity)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_set_kernel_exec_info_svm_ptrs)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_fine_grain_memory_consistency)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_fine_grain_sync_buffers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_shared_address_space_fine_grain)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_shared_sub_buffers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_shared_address_space_fine_grain_buffers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_allocate_shared_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_shared_address_space_coarse_grain_old_api)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_shared_address_space_coarse_grain_new_api)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_cross_buffer_pointers_coarse_grain)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_pointer_passing)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_enqueue_api)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_svm, svm_migrate)
{
    runTest(::testing::UnitTest::GetInstance());
}
