#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_svm : public testing::Test {
};

TEST_F(test_svm, svm_byte_granularity) {
	RUN_TEST();
}

TEST_F(test_svm, svm_set_kernel_exec_info_svm_ptrs) {
	RUN_TEST();
}

TEST_F(test_svm, svm_fine_grain_memory_consistency) {
	RUN_TEST();
}

TEST_F(test_svm, svm_fine_grain_sync_buffers) {
	RUN_TEST();
}

TEST_F(test_svm, svm_shared_address_space_fine_grain) {
	RUN_TEST();
}

TEST_F(test_svm, svm_shared_sub_buffers) {
	RUN_TEST();
}

TEST_F(test_svm, svm_shared_address_space_fine_grain_buffers) {
	RUN_TEST();
}

TEST_F(test_svm, svm_allocate_shared_buffer) {
	RUN_TEST();
}

TEST_F(test_svm, svm_shared_address_space_coarse_grain_old_api) {
	RUN_TEST();
}

TEST_F(test_svm, svm_shared_address_space_coarse_grain_new_api) {
	RUN_TEST();
}

TEST_F(test_svm, svm_cross_buffer_pointers_coarse_grain) {
	RUN_TEST();
}

TEST_F(test_svm, svm_pointer_passing) {
	RUN_TEST();
}

TEST_F(test_svm, svm_enqueue_api) {
	RUN_TEST();
}

TEST_F(test_svm, svm_migrate) {
	RUN_TEST();
}
