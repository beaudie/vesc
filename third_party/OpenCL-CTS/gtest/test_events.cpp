#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_events : public testing::Test
{};

TEST_F(test_events, event_get_execute_status)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_get_write_array_status)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_get_read_array_status)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_get_info)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_wait_for_execute)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_wait_for_array)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_flush)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_finish_execute)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_finish_array)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_release_before_done)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_enqueue_marker)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_enqueue_marker_with_event_list)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, event_enqueue_barrier_with_event_list)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_waitlist_single_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_waitlist_multi_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_waitlist_multi_queue_multi_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_single_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_multi_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_multi_queue_multi_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_marker_single_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_marker_multi_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_marker_multi_queue_multi_device)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, out_of_order_event_enqueue_barrier_single_queue)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, waitlists)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, userevents)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, callbacks)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, callbacks_simultaneous)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_events, userevents_multithreaded)
{
    runTest(::testing::UnitTest::GetInstance());
}
