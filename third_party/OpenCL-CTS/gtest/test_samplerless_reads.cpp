#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_samplerless_reads : public testing::Test
{};

TEST_F(test_samplerless_reads, 1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_samplerless_reads, 1Dbuffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_samplerless_reads, 2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_samplerless_reads, 3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_samplerless_reads, 1Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_samplerless_reads, 2Darray)
{
    runTest(::testing::UnitTest::GetInstance());
}
