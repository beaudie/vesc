#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_image_streams : public testing::Test {
};

TEST_F(test_image_streams, 1D) {
	RUN_TEST();
}

TEST_F(test_image_streams, 2D) {
	RUN_TEST();
}

TEST_F(test_image_streams, 3D) {
	RUN_TEST();
}

TEST_F(test_image_streams, 1Darray) {
	RUN_TEST();
}

TEST_F(test_image_streams, 2Darray) {
	RUN_TEST();
}

TEST_F(test_image_streams, cl_image_requirements_size_ext_negative) {
	RUN_TEST();
}

TEST_F(test_image_streams, cl_image_requirements_size_ext_consistency) {
	RUN_TEST();
}

TEST_F(test_image_streams, clGetImageRequirementsInfoEXT_negative) {
	RUN_TEST();
}

TEST_F(test_image_streams, cl_image_requirements_max_val_ext_negative) {
	RUN_TEST();
}

TEST_F(test_image_streams, cl_image_requirements_max_val_ext_positive) {
	RUN_TEST();
}

TEST_F(test_image_streams, image2d_from_buffer_positive) {
	RUN_TEST();
}

TEST_F(test_image_streams, memInfo_image_from_buffer_positive) {
	RUN_TEST();
}

TEST_F(test_image_streams, imageInfo_image_from_buffer_positive) {
	RUN_TEST();
}

TEST_F(test_image_streams, image_from_buffer_alignment_negative) {
	RUN_TEST();
}

TEST_F(test_image_streams, image_from_small_buffer_negative) {
	RUN_TEST();
}

TEST_F(test_image_streams, image_from_buffer_fill_positive) {
	RUN_TEST();
}

TEST_F(test_image_streams, image_from_buffer_read_positive) {
	RUN_TEST();
}
