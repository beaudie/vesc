#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_multiples : public testing::Test {
};

TEST_F(test_multiples, context_multiple_contexts_same_device) {
	RUN_TEST();
}

TEST_F(test_multiples, context_two_contexts_same_device) {
	RUN_TEST();
}

TEST_F(test_multiples, context_three_contexts_same_device) {
	RUN_TEST();
}

TEST_F(test_multiples, context_four_contexts_same_device) {
	RUN_TEST();
}

TEST_F(test_multiples, two_devices) {
	RUN_TEST();
}

TEST_F(test_multiples, max_devices) {
	RUN_TEST();
}

TEST_F(test_multiples, hundred_queues) {
	RUN_TEST();
}
