#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_half : public testing::Test
{};

TEST_F(test_half, vload_half)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vloada_half)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstore_half)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstorea_half)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstore_half_rte)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstorea_half_rte)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstore_half_rtz)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstorea_half_rtz)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstore_half_rtp)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstorea_half_rtp)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstore_half_rtn)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, vstorea_half_rtn)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_half, roundTrip)
{
    runTest(::testing::UnitTest::GetInstance());
}
