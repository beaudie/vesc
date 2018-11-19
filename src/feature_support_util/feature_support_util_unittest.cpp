//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../gpu_info_util/SystemInfo.h"
#include "feature_support_util.h"

using namespace angle;

TEST(FeatureSupportUtilTest, APIVersion)
{
    unsigned int versionToUse;
    unsigned int zero = 0;

    versionToUse = kFeatureVersion_LowestSupported;
    EXPECT_EQ(true, ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_LowestSupported, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported;
    EXPECT_EQ(true, ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);

    versionToUse = zero;
    EXPECT_EQ(false, ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(zero, versionToUse);

    versionToUse = kFeatureVersion_HighestSupported + 1;
    EXPECT_EQ(true, ANGLEGetFeatureSupportUtilAPIVersion(&versionToUse));
    EXPECT_EQ(kFeatureVersion_HighestSupported, versionToUse);
}

TEST(FeatureSupportUtilTest, SystemInfo)
{
    SystemInfo *systemInfoHandle = nullptr;
    unsigned int zero = 0;
    unsigned int one = 1;

    EXPECT_EQ(true, ANGLEGetSystemInfo(reinterpret_cast<void**>(&systemInfoHandle)));
    EXPECT_EQ("", systemInfoHandle->machineManufacturer);
    EXPECT_EQ("", systemInfoHandle->machineModelName);
    EXPECT_EQ(one, systemInfoHandle->gpus.size());
    EXPECT_NE(zero, systemInfoHandle->gpus[0].vendorId);
    EXPECT_NE(zero, systemInfoHandle->gpus[0].deviceId);
    EXPECT_NE(zero, systemInfoHandle->gpus[0].detailedDriverVersion.major);

    char mfr[] = "Google";
    char model[] = "Pixel1";

    ANGLEAddDeviceInfoToSystemInfo(mfr, model, systemInfoHandle);
    EXPECT_EQ("Google", systemInfoHandle->machineManufacturer);
    EXPECT_EQ("Pixel1", systemInfoHandle->machineModelName);
}

TEST(FeatureSupportUtilTest, ParseRules)
{
    SystemInfo systemInfo;
    systemInfo.machineManufacturer = "Google";
    systemInfo.machineModelName = "Pixel1";
    systemInfo.gpus.resize(1);
    systemInfo.gpus[0].vendorId = 123;
    systemInfo.gpus[0].deviceId = 234;
    systemInfo.gpus[0].driverVendor = "DriverVendorA";
    systemInfo.gpus[0].detailedDriverVersion = {1,2,3,4};

    char rulesFileContents[] =
            "{\"Rules\":[{\"Rule\":\"Default Rule (i.e. use native driver)\", \"AppChoice\":true, \"NonChoice\":false}]}\n";
    ssize_t rulesFileSize = strlen(rulesFileContents);
    void *rulesHandle = nullptr;
    int rulesVersion = 0;
    EXPECT_EQ(true, ANGLEAndroidParseRulesFile(rulesFileContents, rulesFileSize,
                                               &rulesHandle, &rulesVersion));
}
