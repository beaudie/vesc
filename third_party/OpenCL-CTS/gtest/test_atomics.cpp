#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_atomics : public testing::Test {
};

TEST_F(test_atomics, atomic_add) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_sub) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_xchg) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_min) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_max) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_inc) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_dec) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_cmpxchg) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_and) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_or) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_xor) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_add_index) {
	RUN_TEST();
}

TEST_F(test_atomics, atomic_add_index_bin) {
	RUN_TEST();
}
