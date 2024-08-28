#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_workgroups : public testing::Test {
};

TEST_F(test_workgroups, work_group_all) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_any) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_reduce_add) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_reduce_min) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_reduce_max) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_inclusive_add) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_inclusive_min) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_inclusive_max) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_exclusive_add) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_exclusive_min) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_scan_exclusive_max) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_broadcast_1D) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_broadcast_2D) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_broadcast_3D) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_suggested_local_size_1D) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_suggested_local_size_2D) {
	RUN_TEST();
}

TEST_F(test_workgroups, work_group_suggested_local_size_3D) {
	RUN_TEST();
}
