#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_device_execution : public testing::Test {
};

TEST_F(test_device_execution, device_info) {
	RUN_TEST();
}

TEST_F(test_device_execution, device_queue) {
	RUN_TEST();
}

TEST_F(test_device_execution, execute_block) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_block) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_nested_blocks) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_wg_size) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_flags) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_multi_queue) {
	RUN_TEST();
}

TEST_F(test_device_execution, host_multi_queue) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_ndrange) {
	RUN_TEST();
}

TEST_F(test_device_execution, host_queue_order) {
	RUN_TEST();
}

TEST_F(test_device_execution, enqueue_profiling) {
	RUN_TEST();
}
