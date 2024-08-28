#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_vectors : public testing::Test {
};

TEST_F(test_vectors, step_type) {
	RUN_TEST();
}

TEST_F(test_vectors, step_var) {
	RUN_TEST();
}

TEST_F(test_vectors, step_typedef_type) {
	RUN_TEST();
}

TEST_F(test_vectors, step_typedef_var) {
	RUN_TEST();
}

TEST_F(test_vectors, vec_align_array) {
	RUN_TEST();
}

TEST_F(test_vectors, vec_align_struct) {
	RUN_TEST();
}

TEST_F(test_vectors, vec_align_packed_struct) {
	RUN_TEST();
}

TEST_F(test_vectors, vec_align_struct_arr) {
	RUN_TEST();
}

TEST_F(test_vectors, vec_align_packed_struct_arr) {
	RUN_TEST();
}
