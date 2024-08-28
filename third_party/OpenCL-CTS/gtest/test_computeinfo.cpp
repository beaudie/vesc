#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_computeinfo : public testing::Test
{};

TEST_F(test_computeinfo, computeinfo)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_computeinfo, extended_versioning)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_computeinfo, device_uuid)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_computeinfo, conformance_version)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_computeinfo, pci_bus_info)
{
    runTest(::testing::UnitTest::GetInstance());
}
