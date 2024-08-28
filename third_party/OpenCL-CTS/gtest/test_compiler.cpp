#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_compiler : public testing::Test {
};

TEST_F(test_compiler, load_program_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_multistring_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_two_kernel_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_null_terminated_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_null_terminated_multi_line_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_null_terminated_partial_multi_line_source) {
	RUN_TEST();
}

TEST_F(test_compiler, load_discreet_length_source) {
	RUN_TEST();
}

TEST_F(test_compiler, get_program_source) {
	RUN_TEST();
}

TEST_F(test_compiler, get_program_build_info) {
	RUN_TEST();
}

TEST_F(test_compiler, get_program_info) {
	RUN_TEST();
}

TEST_F(test_compiler, large_compile) {
	RUN_TEST();
}

TEST_F(test_compiler, async_build) {
	RUN_TEST();
}

TEST_F(test_compiler, options_build_optimizations) {
	RUN_TEST();
}

TEST_F(test_compiler, options_build_macro) {
	RUN_TEST();
}

TEST_F(test_compiler, options_build_macro_existence) {
	RUN_TEST();
}

TEST_F(test_compiler, options_include_directory) {
	RUN_TEST();
}

TEST_F(test_compiler, options_denorm_cache) {
	RUN_TEST();
}

TEST_F(test_compiler, preprocessor_define_udef) {
	RUN_TEST();
}

TEST_F(test_compiler, preprocessor_include) {
	RUN_TEST();
}

TEST_F(test_compiler, preprocessor_line_error) {
	RUN_TEST();
}

TEST_F(test_compiler, preprocessor_pragma) {
	RUN_TEST();
}

TEST_F(test_compiler, opencl_c_versions) {
	RUN_TEST();
}

TEST_F(test_compiler, compiler_defines_for_extensions) {
	RUN_TEST();
}

TEST_F(test_compiler, image_macro) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_compile_only) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_static_compile_only) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_extern_compile_only) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_compile_with_callback) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_embedded_header_compile) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_link_only) {
	RUN_TEST();
}

TEST_F(test_compiler, two_file_regular_variable_access) {
	RUN_TEST();
}

TEST_F(test_compiler, two_file_regular_struct_access) {
	RUN_TEST();
}

TEST_F(test_compiler, two_file_regular_function_access) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_link_with_callback) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_embedded_header_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_simple_compile_and_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_no_device_info) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_with_defines) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_with_callbacks) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_simple_library_with_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_two_file_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_embedded_header_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_included_header_link) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_serialize_reload_object) {
	RUN_TEST();
}

TEST_F(test_compiler, execute_after_serialize_reload_library) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_library_only) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_library_with_callback) {
	RUN_TEST();
}

TEST_F(test_compiler, simple_library_with_link) {
	RUN_TEST();
}

TEST_F(test_compiler, two_file_link) {
	RUN_TEST();
}

TEST_F(test_compiler, multi_file_libraries) {
	RUN_TEST();
}

TEST_F(test_compiler, multiple_files) {
	RUN_TEST();
}

TEST_F(test_compiler, multiple_libraries) {
	RUN_TEST();
}

TEST_F(test_compiler, multiple_files_multiple_libraries) {
	RUN_TEST();
}

TEST_F(test_compiler, multiple_embedded_headers) {
	RUN_TEST();
}

TEST_F(test_compiler, program_binary_type) {
	RUN_TEST();
}

TEST_F(test_compiler, compile_and_link_status_options_log) {
	RUN_TEST();
}

TEST_F(test_compiler, pragma_unroll) {
	RUN_TEST();
}

TEST_F(test_compiler, features_macro) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_valid) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_repeated) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_compile_unload_link) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_build_unload_create_kernel) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_link_different) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_build_threaded) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_build_info) {
	RUN_TEST();
}

TEST_F(test_compiler, unload_program_binaries) {
	RUN_TEST();
}
