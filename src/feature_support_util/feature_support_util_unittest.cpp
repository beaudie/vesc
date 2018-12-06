//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util_unittest.cpp: Unit test for the feature-support utility.

#include <gtest/gtest.h>

#include "../gpu_info_util/SystemInfo.h"
#include "feature_support_util.h"

using namespace angle;

class FeatureSupportUtilTest : public testing::Test
{
  protected:
    FeatureSupportUtilTest()
    {
        mSystemInfo.machineManufacturer = "MfrFoo";
        mSystemInfo.machineModelName    = "ModelX";
        mSystemInfo.gpus.resize(1);
        mSystemInfo.gpus[0].vendorId              = 123;
        mSystemInfo.gpus[0].deviceId              = 234;
        mSystemInfo.gpus[0].driverVendor          = "GPUVendorA";
        mSystemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 4};
    }

    SystemInfo mSystemInfo;
};

// Test the ANGLEGetFeatureSupportUtilAPIVersion function
TEST_F(FeatureSupportUtilTest, APIVersion)
{
    unsigned int versionToUse;
    unsigned int zero = 0;

    versionToUse = kFeatureVersion_LowestSupported;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_LowestSupported, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);

    versionToUse = zero;
    EXPECT_FALSE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(zero, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported + 1;
    EXPECT_TRUE(ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);
}

// Test the ANGLEAddDeviceInfoToSystemInfo function
TEST_F(FeatureSupportUtilTest, SystemInfo)
{
    SystemInfo systemInfo = mSystemInfo;
    systemInfo.machineManufacturer = "BAD";
    systemInfo.machineModelName    = "BAD";

    char mfr[]   = "MfrFoo";
    char model[] = "ModelX";

    ANGLEAddDeviceInfoToSystemInfo(mfr, model, &systemInfo);
    EXPECT_EQ("MfrFoo", systemInfo.machineManufacturer);
    EXPECT_EQ("ModelX", systemInfo.machineModelName);
}

// Test the ANGLEAndroidParseRulesString function
TEST_F(FeatureSupportUtilTest, ParseRules)
{
    SystemInfo systemInfo = mSystemInfo;

    constexpr char rulesFileContents[] =
        "{\"Rules\":[{\"Rule\":\"Default Rule (i.e. use native driver)\", \"AppChoice\":true, "
        "\"NonChoice\":false}]}\n";
    RulesHandle rulesHandle = nullptr;
    int rulesVersion        = 0;
    EXPECT_TRUE(ANGLEAndroidParseRulesString(rulesFileContents, &rulesHandle, &rulesVersion));
    EXPECT_NE(nullptr, rulesHandle);
    ANGLEFreeRulesHandle(rulesHandle);
}

// Test the ANGLEAndroidParseRulesString and ANGLEShouldBeUsedForApplication functions
TEST_F(FeatureSupportUtilTest, TestRuleProcessing)
{
    SystemInfo systemInfo = mSystemInfo;

    constexpr char rulesFileContents[] =
        "{\n"
        "    \"Rules\":[\n"
        "        {\n"
        "            \"Rule\":\"Default Rule (i.e. do not use ANGLE)\",\n"
        "            \"UseANGLE\":false\n"
        "        },\n"
        "        {\n"
        "            \"Rule\":\"Supported application(s)\",\n"
        "            \"UseANGLE\":true,\n"
        "            \"Applications\":[\n"
        "                {\n"
        "                    \"AppName\":\"com.isvA.app1\"\n"
        "                }\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"Rule\":\"Exceptions for bad drivers(s)\",\n"
        "            \"UseANGLE\":false,\n"
        "            \"Applications\":[\n"
        "                {\n"
        "                    \"AppName\":\"com.isvA.app1\"\n"
        "                }\n"
        "            ],\n"
        "            \"Devices\":[\n"
        "                {\n"
        "                    \"Manufacturer\":\"MfrFoo\",\n"
        "                    \"Model\":\"ModelX\",\n"
        "                    \"GPUs\":[\n"
        "                        {\n"
        "                            \"Vendor\" : \"GPUVendorA\",\n"
        "                            \"DeviceId\" : 234,\n"
        "                            \"VerMajor\" : 1, \"VerMinor\" : 2,\n"
        "                            \"VerSubMinor\" : 3, \"VerPatch\" : 4}\n"
        "                        }\n"
        "                    ]\n"
        "                }\n"
        "            ]\n"
        "        }\n"
        "    ]\n"
        "}\n";
    RulesHandle rulesHandle = nullptr;
    int rulesVersion        = 0;
    EXPECT_TRUE(ANGLEAndroidParseRulesString(rulesFileContents, &rulesHandle, &rulesVersion));
    EXPECT_NE(nullptr, rulesHandle);

    // Test app1 with a SystemInfo that has an unsupported driver--should fail:
    char app1[] = "com.isvA.app1";
    EXPECT_FALSE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, app1));

    // Test app1 with a SystemInfo that has a supported driver--should pass:
    systemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 5};
    EXPECT_TRUE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, app1));

    // Test unsupported app2--should fail:
    char app2[] = "com.isvB.app2";
    EXPECT_FALSE(ANGLEShouldBeUsedForApplication(rulesHandle, rulesVersion, &systemInfo, app2));

    // Free the rules data structures:
    ANGLEFreeRulesHandle(rulesHandle);
}
