#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_api : public testing::Test {
};

TEST_F(test_api, get_platform_info) {
	RUN_TEST();
}

TEST_F(test_api, get_sampler_info) {
	RUN_TEST();
}

TEST_F(test_api, get_sampler_info_compatibility) {
	RUN_TEST();
}

TEST_F(test_api, get_command_queue_info) {
	RUN_TEST();
}

TEST_F(test_api, get_command_queue_info_compatibility) {
	RUN_TEST();
}

TEST_F(test_api, get_context_info) {
	RUN_TEST();
}

TEST_F(test_api, get_device_info) {
	RUN_TEST();
}

TEST_F(test_api, enqueue_task) {
	RUN_TEST();
}

TEST_F(test_api, binary_get) {
	RUN_TEST();
}

TEST_F(test_api, binary_create) {
	RUN_TEST();
}

TEST_F(test_api, kernel_required_group_size) {
	RUN_TEST();
}

TEST_F(test_api, release_kernel_order) {
	RUN_TEST();
}

TEST_F(test_api, release_during_execute) {
	RUN_TEST();
}

TEST_F(test_api, load_single_kernel) {
	RUN_TEST();
}

TEST_F(test_api, load_two_kernels) {
	RUN_TEST();
}

TEST_F(test_api, load_two_kernels_in_one) {
	RUN_TEST();
}

TEST_F(test_api, load_two_kernels_manually) {
	RUN_TEST();
}

TEST_F(test_api, get_program_info_kernel_names) {
	RUN_TEST();
}

TEST_F(test_api, get_kernel_arg_info) {
	RUN_TEST();
}

TEST_F(test_api, create_kernels_in_program) {
	RUN_TEST();
}

TEST_F(test_api, get_kernel_info) {
	RUN_TEST();
}

TEST_F(test_api, kernel_private_memory_size) {
	RUN_TEST();
}

TEST_F(test_api, execute_kernel_local_sizes) {
	RUN_TEST();
}

TEST_F(test_api, set_kernel_arg_by_index) {
	RUN_TEST();
}

TEST_F(test_api, set_kernel_arg_constant) {
	RUN_TEST();
}

TEST_F(test_api, set_kernel_arg_struct_array) {
	RUN_TEST();
}

TEST_F(test_api, kernel_global_constant) {
	RUN_TEST();
}

TEST_F(test_api, kernel_attributes) {
	RUN_TEST();
}

TEST_F(test_api, min_max_thread_dimensions) {
	RUN_TEST();
}

TEST_F(test_api, min_max_work_items_sizes) {
	RUN_TEST();
}

TEST_F(test_api, min_max_work_group_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_read_image_args) {
	RUN_TEST();
}

TEST_F(test_api, min_max_write_image_args) {
	RUN_TEST();
}

TEST_F(test_api, min_max_mem_alloc_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_2d_width) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_2d_height) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_3d_width) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_3d_height) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_3d_depth) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_array_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_image_buffer_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_parameter_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_samplers) {
	RUN_TEST();
}

TEST_F(test_api, min_max_constant_buffer_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_constant_args) {
	RUN_TEST();
}

TEST_F(test_api, min_max_compute_units) {
	RUN_TEST();
}

TEST_F(test_api, min_max_address_bits) {
	RUN_TEST();
}

TEST_F(test_api, min_max_single_fp_config) {
	RUN_TEST();
}

TEST_F(test_api, min_max_double_fp_config) {
	RUN_TEST();
}

TEST_F(test_api, min_max_local_mem_size) {
	RUN_TEST();
}

TEST_F(test_api, min_max_kernel_preferred_work_group_size_multiple) {
	RUN_TEST();
}

TEST_F(test_api, min_max_execution_capabilities) {
	RUN_TEST();
}

TEST_F(test_api, min_max_queue_properties) {
	RUN_TEST();
}

TEST_F(test_api, min_max_device_version) {
	RUN_TEST();
}

TEST_F(test_api, min_max_language_version) {
	RUN_TEST();
}

TEST_F(test_api, kernel_arg_changes) {
	RUN_TEST();
}

TEST_F(test_api, kernel_arg_multi_setup_random) {
	RUN_TEST();
}

TEST_F(test_api, native_kernel) {
	RUN_TEST();
}

TEST_F(test_api, create_context_from_type) {
	RUN_TEST();
}

TEST_F(test_api, platform_extensions) {
	RUN_TEST();
}

TEST_F(test_api, get_platform_ids) {
	RUN_TEST();
}

TEST_F(test_api, bool_type) {
	RUN_TEST();
}

TEST_F(test_api, repeated_setup_cleanup) {
	RUN_TEST();
}

TEST_F(test_api, retain_queue_single) {
	RUN_TEST();
}

TEST_F(test_api, retain_queue_multiple) {
	RUN_TEST();
}

TEST_F(test_api, retain_mem_object_single) {
	RUN_TEST();
}

TEST_F(test_api, retain_mem_object_multiple) {
	RUN_TEST();
}

TEST_F(test_api, retain_mem_object_set_kernel_arg) {
	RUN_TEST();
}

TEST_F(test_api, min_data_type_align_size_alignment) {
	RUN_TEST();
}

TEST_F(test_api, context_destructor_callback) {
	RUN_TEST();
}

TEST_F(test_api, mem_object_destructor_callback) {
	RUN_TEST();
}

TEST_F(test_api, null_buffer_arg) {
	RUN_TEST();
}

TEST_F(test_api, get_buffer_info) {
	RUN_TEST();
}

TEST_F(test_api, get_image2d_info) {
	RUN_TEST();
}

TEST_F(test_api, get_image3d_info) {
	RUN_TEST();
}

TEST_F(test_api, get_image1d_info) {
	RUN_TEST();
}

TEST_F(test_api, get_image1d_array_info) {
	RUN_TEST();
}

TEST_F(test_api, get_image2d_array_info) {
	RUN_TEST();
}

TEST_F(test_api, queue_flush_on_release) {
	RUN_TEST();
}

TEST_F(test_api, queue_hint) {
	RUN_TEST();
}

TEST_F(test_api, queue_properties) {
	RUN_TEST();
}

TEST_F(test_api, sub_group_dispatch) {
	RUN_TEST();
}

TEST_F(test_api, clone_kernel) {
	RUN_TEST();
}

TEST_F(test_api, zero_sized_enqueue) {
	RUN_TEST();
}

TEST_F(test_api, buffer_properties_queries) {
	RUN_TEST();
}

TEST_F(test_api, image_properties_queries) {
	RUN_TEST();
}

TEST_F(test_api, queue_properties_queries) {
	RUN_TEST();
}

TEST_F(test_api, pipe_properties_queries) {
	RUN_TEST();
}

TEST_F(test_api, consistency_svm) {
	RUN_TEST();
}

TEST_F(test_api, consistency_memory_model) {
	RUN_TEST();
}

TEST_F(test_api, consistency_device_enqueue) {
	RUN_TEST();
}

TEST_F(test_api, consistency_pipes) {
	RUN_TEST();
}

TEST_F(test_api, consistency_progvar) {
	RUN_TEST();
}

TEST_F(test_api, consistency_non_uniform_work_group) {
	RUN_TEST();
}

TEST_F(test_api, consistency_read_write_images) {
	RUN_TEST();
}

TEST_F(test_api, consistency_2d_image_from_buffer) {
	RUN_TEST();
}

TEST_F(test_api, consistency_depth_images) {
	RUN_TEST();
}

TEST_F(test_api, consistency_device_and_host_timer) {
	RUN_TEST();
}

TEST_F(test_api, consistency_il_programs) {
	RUN_TEST();
}

TEST_F(test_api, consistency_subgroups) {
	RUN_TEST();
}

TEST_F(test_api, consistency_prog_ctor_dtor) {
	RUN_TEST();
}

TEST_F(test_api, consistency_3d_image_writes) {
	RUN_TEST();
}

TEST_F(test_api, min_image_formats) {
	RUN_TEST();
}

TEST_F(test_api, negative_get_platform_info) {
	RUN_TEST();
}

TEST_F(test_api, negative_get_platform_ids) {
	RUN_TEST();
}
