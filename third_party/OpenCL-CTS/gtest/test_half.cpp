#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_half : public testing::Test {
};

TEST_F(test_half, vload_half) {
	RUN_TEST();
}

TEST_F(test_half, vloada_half) {
	RUN_TEST();
}

TEST_F(test_half, vstore_half) {
	RUN_TEST();
}

TEST_F(test_half, vstorea_half) {
	RUN_TEST();
}

TEST_F(test_half, vstore_half_rte) {
	RUN_TEST();
}

TEST_F(test_half, vstorea_half_rte) {
	RUN_TEST();
}

TEST_F(test_half, vstore_half_rtz) {
	RUN_TEST();
}

TEST_F(test_half, vstorea_half_rtz) {
	RUN_TEST();
}

TEST_F(test_half, vstore_half_rtp) {
	RUN_TEST();
}

TEST_F(test_half, vstorea_half_rtp) {
	RUN_TEST();
}

TEST_F(test_half, vstore_half_rtn) {
	RUN_TEST();
}

TEST_F(test_half, vstorea_half_rtn) {
	RUN_TEST();
}

TEST_F(test_half, roundTrip) {
	RUN_TEST();
}
