#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_geometrics : public testing::Test {
};

TEST_F(test_geometrics, geom_cross) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_dot) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_distance) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_fast_distance) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_length) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_fast_length) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_normalize) {
	RUN_TEST();
}

TEST_F(test_geometrics, geom_fast_normalize) {
	RUN_TEST();
}
