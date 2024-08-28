#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_non_uniform_work_group : public testing::Test
{};

TEST_F(test_non_uniform_work_group, non_uniform_1d_basic)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_1d_atomics)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_1d_barriers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_basic)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_atomics)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_barriers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_basic)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_atomics)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_barriers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_other_basic)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_other_atomics)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_non_uniform_work_group, non_uniform_other_barriers)
{
    runTest(::testing::UnitTest::GetInstance());
}
