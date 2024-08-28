#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_device_partition : public testing::Test {
};

TEST_F(test_device_partition, partition_equally) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_counts) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_numa) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_l4_cache) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_l3_cache) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_l2_cache) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_l1_cache) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_by_affinity_domain_next_partitionable) {
	RUN_TEST();
}

TEST_F(test_device_partition, partition_all) {
	RUN_TEST();
}
