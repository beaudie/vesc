#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_kernel_image_methods : public testing::Test {
};

TEST_F(test_kernel_image_methods, 1D) {
	RUN_TEST();
}

TEST_F(test_kernel_image_methods, 2D) {
	RUN_TEST();
}

TEST_F(test_kernel_image_methods, 3D) {
	RUN_TEST();
}

TEST_F(test_kernel_image_methods, 1Darray) {
	RUN_TEST();
}

TEST_F(test_kernel_image_methods, 2Darray) {
	RUN_TEST();
}
