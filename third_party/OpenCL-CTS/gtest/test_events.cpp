#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_events : public testing::Test {
};

TEST_F(test_events, event_get_execute_status) {
	RUN_TEST();
}

TEST_F(test_events, event_get_write_array_status) {
	RUN_TEST();
}

TEST_F(test_events, event_get_read_array_status) {
	RUN_TEST();
}

TEST_F(test_events, event_get_info) {
	RUN_TEST();
}

TEST_F(test_events, event_wait_for_execute) {
	RUN_TEST();
}

TEST_F(test_events, event_wait_for_array) {
	RUN_TEST();
}

TEST_F(test_events, event_flush) {
	RUN_TEST();
}

TEST_F(test_events, event_finish_execute) {
	RUN_TEST();
}

TEST_F(test_events, event_finish_array) {
	RUN_TEST();
}

TEST_F(test_events, event_release_before_done) {
	RUN_TEST();
}

TEST_F(test_events, event_enqueue_marker) {
	RUN_TEST();
}

TEST_F(test_events, event_enqueue_marker_with_event_list) {
	RUN_TEST();
}

TEST_F(test_events, event_enqueue_barrier_with_event_list) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_waitlist_single_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_waitlist_multi_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_waitlist_multi_queue_multi_device) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_single_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_multi_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_wait_for_events_multi_queue_multi_device) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_marker_single_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_marker_multi_queue) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_marker_multi_queue_multi_device) {
	RUN_TEST();
}

TEST_F(test_events, out_of_order_event_enqueue_barrier_single_queue) {
	RUN_TEST();
}

TEST_F(test_events, waitlists) {
	RUN_TEST();
}

TEST_F(test_events, userevents) {
	RUN_TEST();
}

TEST_F(test_events, callbacks) {
	RUN_TEST();
}

TEST_F(test_events, callbacks_simultaneous) {
	RUN_TEST();
}

TEST_F(test_events, userevents_multithreaded) {
	RUN_TEST();
}
