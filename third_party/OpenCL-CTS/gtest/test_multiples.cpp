#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_multiples : public testing::Test
{};

TEST_F(test_multiples, context_multiple_contexts_same_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, context_two_contexts_same_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, context_three_contexts_same_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, context_four_contexts_same_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, two_devices)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, max_devices)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_multiples, hundred_queues)
{
    runTest(::testing::UnitTest::GetInstance());
}
