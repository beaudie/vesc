#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_khr_command_buffer : public testing::Test {
};

TEST_F(test_cl_khr_command_buffer, single_ndrange) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, interleaved_enqueue) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, mixed_commands) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, explicit_flush) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, out_of_order) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, simultaneous_out_of_order) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, info_queues) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, info_ref_count) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, info_state) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, info_prop_array) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, info_context) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, basic_profiling) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, simultaneous_profiling) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, regular_wait_for_command_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_command_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_sec_command_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, return_event_callback) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, clwaitforevents_single) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, clwaitforevents) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_regular) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, wait_for_sec_queue_event) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, user_event_wait) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, user_events_wait) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, user_event_callback) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, queue_substitution) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, properties_queue_substitution) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, simultaneous_queue_substitution) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, fill_image) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, fill_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, fill_svm_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_image) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_svm_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_buffer_to_image) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_image_to_buffer) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, copy_buffer_rect) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, barrier_wait_list) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, basic_printf) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, simultaneous_printf) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, basic_set_kernel_arg) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, pending_set_kernel_arg) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, event_info_command_type) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, event_info_command_queue) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, event_info_execution_status) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, event_info_context) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, event_info_reference_count) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, finalize_invalid) {
	RUN_TEST();
}

TEST_F(test_cl_khr_command_buffer, finalize_empty) {
	RUN_TEST();
}
