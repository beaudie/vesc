#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_khr_semaphore : public testing::Test
{};

TEST_F(test_cl_khr_semaphore, semaphores_simple_1)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_simple_2)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_reuse)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_cross_queues_ooo)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_cross_queues_io)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_multi_signal)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_multi_wait)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_queries)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_semaphore, semaphores_import_export_fd)
{
    runTest(::testing::UnitTest::GetInstance());
}
