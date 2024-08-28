#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_get_info : public testing::Test
{};

TEST_F(test_cl_get_info, 1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_get_info, 2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_get_info, 3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_get_info, 1Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_get_info, 2Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}
