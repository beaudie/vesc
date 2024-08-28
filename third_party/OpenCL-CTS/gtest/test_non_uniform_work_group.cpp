#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_non_uniform_work_group : public testing::Test {
};

TEST_F(test_non_uniform_work_group, non_uniform_1d_basic) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_1d_atomics) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_1d_barriers) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_basic) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_atomics) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_2d_barriers) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_basic) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_atomics) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_3d_barriers) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_other_basic) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_other_atomics) {
	RUN_TEST();
}

TEST_F(test_non_uniform_work_group, non_uniform_other_barriers) {
	RUN_TEST();
}
