#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_allocations : public testing::Test {
};

TEST_F(test_allocations, buffer) {
	RUN_TEST();
}

TEST_F(test_allocations, image2d_read) {
	RUN_TEST();
}

TEST_F(test_allocations, image2d_write) {
	RUN_TEST();
}

TEST_F(test_allocations, buffer_non_blocking) {
	RUN_TEST();
}

TEST_F(test_allocations, image2d_read_non_blocking) {
	RUN_TEST();
}

TEST_F(test_allocations, image2d_write_non_blocking) {
	RUN_TEST();
}
