#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_vectors : public testing::Test
{};

TEST_F(test_vectors, step_type)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, step_var)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, step_typedef_type)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, step_typedef_var)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, vec_align_array)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, vec_align_struct)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, vec_align_packed_struct)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, vec_align_struct_arr)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_vectors, vec_align_packed_struct_arr)
{
    runTest(::testing::UnitTest::GetInstance());
}
