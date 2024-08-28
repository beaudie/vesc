#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_mem_host_flags : public testing::Test
{};

TEST_F(test_mem_host_flags, mem_host_read_only_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_read_only_subbuffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_write_only_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_write_only_subbuffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_no_access_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_no_access_subbuffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_read_only_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_write_only_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_mem_host_flags, mem_host_no_access_image)
{
    runTest(::testing::UnitTest::GetInstance());
}
