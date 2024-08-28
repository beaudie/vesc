#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_device_timer : public testing::Test
{};

TEST_F(test_device_timer, timer_resolution_queries)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_timer, device_and_host_timers)
{
    runTest(::testing::UnitTest::GetInstance());
}
