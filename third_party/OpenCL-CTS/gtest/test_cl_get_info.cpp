#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_get_info : public testing::Test {
};

TEST_F(test_cl_get_info, 1D) {
	RUN_TEST();
}

TEST_F(test_cl_get_info, 2D) {
	RUN_TEST();
}

TEST_F(test_cl_get_info, 3D) {
	RUN_TEST();
}

TEST_F(test_cl_get_info, 1Darray) {
	RUN_TEST();
}

TEST_F(test_cl_get_info, 2Darray) {
	RUN_TEST();
}
