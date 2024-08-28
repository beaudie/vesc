#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_cl_khr_semaphore : public testing::Test {
};

TEST_F(test_cl_khr_semaphore, semaphores_simple_1) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_simple_2) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_reuse) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_cross_queues_ooo) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_cross_queues_io) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_multi_signal) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_multi_wait) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_queries) {
	RUN_TEST();
}

TEST_F(test_cl_khr_semaphore, semaphores_import_export_fd) {
	RUN_TEST();
}
