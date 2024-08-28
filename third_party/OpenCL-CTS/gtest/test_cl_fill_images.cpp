#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_fill_images : public testing::Test {
};

TEST_F(test_cl_fill_images, 1D) {
	RUN_TEST();
}

TEST_F(test_cl_fill_images, 2D) {
	RUN_TEST();
}

TEST_F(test_cl_fill_images, 3D) {
	RUN_TEST();
}

TEST_F(test_cl_fill_images, 1Darray) {
	RUN_TEST();
}

TEST_F(test_cl_fill_images, 2Darray) {
	RUN_TEST();
}
