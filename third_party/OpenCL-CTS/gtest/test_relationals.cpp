#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_relationals : public testing::Test {
};

TEST_F(test_relationals, relational_any) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_all) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_bitselect) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_select_signed) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_select_unsigned) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_isequal) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_isnotequal) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_isgreater) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_isgreaterequal) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_isless) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_islessequal) {
	RUN_TEST();
}

TEST_F(test_relationals, relational_islessgreater) {
	RUN_TEST();
}

TEST_F(test_relationals, shuffle_copy) {
	RUN_TEST();
}

TEST_F(test_relationals, shuffle_function_call) {
	RUN_TEST();
}

TEST_F(test_relationals, shuffle_array_cast) {
	RUN_TEST();
}

TEST_F(test_relationals, shuffle_built_in) {
	RUN_TEST();
}

TEST_F(test_relationals, shuffle_built_in_dual_input) {
	RUN_TEST();
}
