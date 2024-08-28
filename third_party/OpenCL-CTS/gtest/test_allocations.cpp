#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_allocations : public testing::Test
{};

TEST_F(test_allocations, buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_allocations, image2d_read)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_allocations, image2d_write)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_allocations, buffer_non_blocking)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_allocations, image2d_read_non_blocking)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_allocations, image2d_write_non_blocking)
{
    runTest(::testing::UnitTest::GetInstance());
}
