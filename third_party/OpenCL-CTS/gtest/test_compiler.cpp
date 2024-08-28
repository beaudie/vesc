#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_compiler : public testing::Test
{};

TEST_F(test_compiler, load_program_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_multistring_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_two_kernel_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_null_terminated_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_null_terminated_multi_line_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_null_terminated_partial_multi_line_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, load_discreet_length_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, get_program_source)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, get_program_build_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, get_program_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, large_compile)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, async_build)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, options_build_optimizations)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, options_build_macro)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, options_build_macro_existence)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, options_include_directory)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, options_denorm_cache)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, preprocessor_define_udef)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, preprocessor_include)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, preprocessor_line_error)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, preprocessor_pragma)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, opencl_c_versions)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, compiler_defines_for_extensions)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, image_macro)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_compile_only)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_static_compile_only)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_extern_compile_only)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_compile_with_callback)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_embedded_header_compile)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_link_only)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, two_file_regular_variable_access)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, two_file_regular_struct_access)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, two_file_regular_function_access)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_link_with_callback)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_embedded_header_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_simple_compile_and_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_no_device_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_with_defines)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_simple_compile_and_link_with_callbacks)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_simple_library_with_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_two_file_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_embedded_header_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_included_header_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_serialize_reload_object)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, execute_after_serialize_reload_library)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_library_only)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_library_with_callback)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, simple_library_with_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, two_file_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, multi_file_libraries)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, multiple_files)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, multiple_libraries)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, multiple_files_multiple_libraries)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, multiple_embedded_headers)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, program_binary_type)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, compile_and_link_status_options_log)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, pragma_unroll)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, features_macro)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_valid)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_repeated)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_compile_unload_link)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_build_unload_create_kernel)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_link_different)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_build_threaded)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_build_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_compiler, unload_program_binaries)
{
    runTest(::testing::UnitTest::GetInstance());
}
