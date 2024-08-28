#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_image_streams : public testing::Test
{};

TEST_F(test_image_streams, 1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, 2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, 3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, 1Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, 2Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, cl_image_requirements_size_ext_negative)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, cl_image_requirements_size_ext_consistency)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, clGetImageRequirementsInfoEXT_negative)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, cl_image_requirements_max_val_ext_negative)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, cl_image_requirements_max_val_ext_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, image2d_from_buffer_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, memInfo_image_from_buffer_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, imageInfo_image_from_buffer_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, image_from_buffer_alignment_negative)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, image_from_small_buffer_negative)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, image_from_buffer_fill_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_image_streams, image_from_buffer_read_positive)
{
    runTest(::testing::UnitTest::GetInstance());
}
