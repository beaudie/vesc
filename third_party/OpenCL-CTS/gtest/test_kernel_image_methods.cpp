#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_kernel_image_methods : public testing::Test
{};

TEST_F(test_kernel_image_methods, 1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_kernel_image_methods, 2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_kernel_image_methods, 3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_kernel_image_methods, 1Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_kernel_image_methods, 2Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}
