#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_geometrics : public testing::Test
{};

TEST_F(test_geometrics, geom_cross)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_dot)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_distance)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_fast_distance)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_length)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_fast_length)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_normalize)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_geometrics, geom_fast_normalize)
{
    runTest(::testing::UnitTest::GetInstance());
}
