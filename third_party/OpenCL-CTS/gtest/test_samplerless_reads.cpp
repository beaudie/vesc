#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_samplerless_reads : public testing::Test {
};

TEST_F(test_samplerless_reads, 1D) {
	RUN_TEST();
}

TEST_F(test_samplerless_reads, 1Dbuffer) {
	RUN_TEST();
}

TEST_F(test_samplerless_reads, 2D) {
	RUN_TEST();
}

TEST_F(test_samplerless_reads, 3D) {
	RUN_TEST();
}

TEST_F(test_samplerless_reads, 1Darray) {
	RUN_TEST();
}

TEST_F(test_samplerless_reads, 2Darray) {
	RUN_TEST();
}
