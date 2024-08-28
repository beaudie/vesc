#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_ext_cxx_for_opencl : public testing::Test
{};

TEST_F(test_cl_ext_cxx_for_opencl, cxx_for_opencl_ext)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_ext_cxx_for_opencl, cxx_for_opencl_ver)
{
    runTest(::testing::UnitTest::GetInstance());
}
