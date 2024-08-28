#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_copy_images : public testing::Test {
};

TEST_F(test_cl_copy_images, 1D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 3D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 1Darray) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2Darray) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2Dto3D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 3Dto2D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2Darrayto2D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2Dto2Darray) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 2Darrayto3D) {
	RUN_TEST();
}

TEST_F(test_cl_copy_images, 3Dto2Darray) {
	RUN_TEST();
}
