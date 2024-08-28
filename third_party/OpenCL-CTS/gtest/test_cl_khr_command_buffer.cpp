#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_khr_command_buffer : public testing::Test
{};

TEST_F(test_cl_khr_command_buffer, single_ndrange)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, interleaved_enqueue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, mixed_commands)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, explicit_flush)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, out_of_order)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, simultaneous_out_of_order)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, info_queues)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, info_ref_count)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, info_state)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, info_prop_array)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, info_context)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, basic_profiling)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, simultaneous_profiling)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, regular_wait_for_command_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_command_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_sec_command_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, return_event_callback)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, clwaitforevents_single)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, clwaitforevents)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, command_buffer_wait_for_regular)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, wait_for_sec_queue_event)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, user_event_wait)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, user_events_wait)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, user_event_callback)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, queue_substitution)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, properties_queue_substitution)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, simultaneous_queue_substitution)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, fill_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, fill_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, fill_svm_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_svm_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_buffer_to_image)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_image_to_buffer)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, copy_buffer_rect)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, barrier_wait_list)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, basic_printf)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, simultaneous_printf)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, basic_set_kernel_arg)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, pending_set_kernel_arg)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, event_info_command_type)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, event_info_command_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, event_info_execution_status)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, event_info_context)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, event_info_reference_count)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, finalize_invalid)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_cl_khr_command_buffer, finalize_empty)
{
    runTest(::testing::UnitTest::GetInstance());
}
