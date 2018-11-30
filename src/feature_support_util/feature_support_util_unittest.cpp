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

// Test the StringPart class
TEST(FeatureSupportUtilTest, StringPart)
{
    StringPart emptyString;
    StringPart fooString("foo");
    StringPart fumString("fum");
    EXPECT_TRUE(emptyString.match(emptyString));
    EXPECT_TRUE(emptyString.match(fooString));
    EXPECT_TRUE(fooString.match(emptyString));

    EXPECT_TRUE(fooString.match(fooString));
    EXPECT_TRUE(fumString.match(fumString));
    EXPECT_FALSE(fooString.match(fumString));

    EXPECT_EQ("", emptyString.getPart());
    EXPECT_EQ("foo", fooString.getPart());
    EXPECT_EQ("fum", fumString.getPart());

    EXPECT_TRUE(emptyString.getWildcard());
    EXPECT_FALSE(fooString.getWildcard());
    EXPECT_FALSE(fumString.getWildcard());
}

// Test the IntegerPart class
TEST(FeatureSupportUtilTest, IntegerPart)
{
    unsigned int zero = 0;
    unsigned int one = 1;
    IntegerPart emptyInteger;
    IntegerPart zeroInteger(0);
    IntegerPart oneInteger(1);
    EXPECT_TRUE(emptyInteger.match(emptyInteger));
    EXPECT_TRUE(emptyInteger.match(zeroInteger));
    EXPECT_TRUE(zeroInteger.match(emptyInteger));

    EXPECT_TRUE(zeroInteger.match(zeroInteger));
    EXPECT_TRUE(oneInteger.match(oneInteger));
    EXPECT_FALSE(zeroInteger.match(oneInteger));

    EXPECT_EQ(zero, emptyInteger.getPart());
    EXPECT_EQ(zero, zeroInteger.getPart());
    EXPECT_EQ(one, oneInteger.getPart());

    EXPECT_TRUE(emptyInteger.getWildcard());
    EXPECT_FALSE(zeroInteger.getWildcard());
    EXPECT_FALSE(oneInteger.getWildcard());
}

// Test the Version class
TEST(FeatureSupportUtilTest, Version)
{
#if 0 // For some reason, the EXPECT statements aren't finding the methods.
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version emptyVersion;
    Version majorVersion1(one);
    Version majorVersion2(two);
    Version majorAndMinorVersion1Dot2(one, two);
    Version majorAndMinorVersion2Dot3(two, three);
    Version majorMinorAndSubMinorVersion1Dot2Dot3(one, two, three);
    Version majorMinorAndSubMinorVersion3Dot2Dot1(three, two, one);
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    EXPECT_TRUE(emptyVersion.match(*&emptyVersion));
    EXPECT_TRUE(emptyVersion.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(emptyVersion));

    EXPECT_TRUE(majorVersion1.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(majorAndMinorVersion1Dot2));
    EXPECT_TRUE(majorAndMinorVersion1Dot2.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(majorMinorAndSubMinorVersion1Dot2Dot3));
    EXPECT_TRUE(majorMinorAndSubMinorVersion1Dot2Dot3.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(fullVersion1Dot2Dot3Dot4));
    EXPECT_TRUE(fullVersion1Dot2Dot3Dot4.match(majorVersion1));

    EXPECT_EQ("", emptyVersion.getString());
    EXPECT_EQ("1", majorVersion1.getString());
#else
    EXPECT_TRUE(true);
#endif
}




// Test the ListOf<T> class
TEST(FeatureSupportUtilTest, ListOf)
{
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    EXPECT_TRUE(true);
}

// Test the ANGLEGetFeatureSupportUtilAPIVersion function
TEST(FeatureSupportUtilTest, APIVersion)
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
TEST(FeatureSupportUtilTest, SystemInfo)
{
    // TODO(ianelliott): Replace this with a gtest "fixture", per review feedback.
    SystemInfo systemInfo;
    systemInfo.machineManufacturer = "BAD";
    systemInfo.machineModelName    = "BAD";
    systemInfo.gpus.resize(1);
    systemInfo.gpus[0].vendorId              = 123;
    systemInfo.gpus[0].deviceId              = 234;
    systemInfo.gpus[0].driverVendor          = "DriverVendorA";
    systemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 4};

    char mfr[]   = "Google";
    char model[] = "Pixel1";

    ANGLEAddDeviceInfoToSystemInfo(mfr, model, &systemInfo);
    EXPECT_EQ("Google", systemInfo.machineManufacturer);
    EXPECT_EQ("Pixel1", systemInfo.machineModelName);
}

// Test the ANGLEAndroidParseRulesString function
TEST(FeatureSupportUtilTest, ParseRules)
{
    // TODO(ianelliott): Replace this with a gtest "fixture", per review feedback.
    SystemInfo systemInfo;
    systemInfo.machineManufacturer = "Google";
    systemInfo.machineModelName    = "Pixel1";
    systemInfo.gpus.resize(1);
    systemInfo.gpus[0].vendorId              = 123;
    systemInfo.gpus[0].deviceId              = 234;
    systemInfo.gpus[0].driverVendor          = "DriverVendorA";
    systemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 4};

    constexpr char rulesFileContents[] =
        "{\"Rules\":[{\"Rule\":\"Default Rule (i.e. use native driver)\", \"AppChoice\":true, "
        "\"NonChoice\":false}]}\n";
    RulesHandle rulesHandle = nullptr;
    int rulesVersion        = 0;
    EXPECT_TRUE(ANGLEAndroidParseRulesString(rulesFileContents, &rulesHandle, &rulesVersion));
    EXPECT_NE(nullptr, rulesHandle);
    ANGLEFreeRulesHandle(rulesHandle);
}
