#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_workgroups : public testing::Test
{};

TEST_F(test_workgroups, work_group_all)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_any)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_reduce_add)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_reduce_min)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_reduce_max)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_inclusive_add)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_inclusive_min)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_inclusive_max)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_exclusive_add)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_exclusive_min)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_scan_exclusive_max)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_broadcast_1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_broadcast_2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_broadcast_3D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_suggested_local_size_1D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_suggested_local_size_2D)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_workgroups, work_group_suggested_local_size_3D)
{
    runTest(::testing::UnitTest::GetInstance());
}
