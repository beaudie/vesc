#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_read_write_images : public testing::Test
{};

TEST_F(test_cl_read_write_images, 1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_read_write_images, 2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_read_write_images, 3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_read_write_images, 1Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_read_write_images, 2Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}
