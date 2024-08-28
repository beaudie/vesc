#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_computeinfo : public testing::Test {
};

TEST_F(test_computeinfo, computeinfo) {
	RUN_TEST();
}

TEST_F(test_computeinfo, extended_versioning) {
	RUN_TEST();
}

TEST_F(test_computeinfo, device_uuid) {
	RUN_TEST();
}

TEST_F(test_computeinfo, conformance_version) {
	RUN_TEST();
}

TEST_F(test_computeinfo, pci_bus_info) {
	RUN_TEST();
}
