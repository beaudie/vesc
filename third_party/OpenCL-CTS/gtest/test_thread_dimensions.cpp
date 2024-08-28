#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_thread_dimensions : public testing::Test {
};

TEST_F(test_thread_dimensions, quick_1d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, quick_2d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, quick_3d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, quick_1d_implicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, quick_2d_implicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, quick_3d_implicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_1d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_2d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_3d_explicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_1d_implicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_2d_implicit_local) {
	RUN_TEST();
}

TEST_F(test_thread_dimensions, full_3d_implicit_local) {
	RUN_TEST();
}
