#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_device_execution : public testing::Test
{};

TEST_F(test_device_execution, device_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, device_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, execute_block)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_block)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_nested_blocks)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_wg_size)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_flags)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_multi_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, host_multi_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_ndrange)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, host_queue_order)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_device_execution, enqueue_profiling)
{
    runTest(::testing::UnitTest::GetInstance());
}
