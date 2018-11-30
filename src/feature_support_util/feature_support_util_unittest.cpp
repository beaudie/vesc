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
    StringPart defaultString;
    StringPart fooString("foo");
    StringPart fumString("fum");
    EXPECT_TRUE(defaultString.match(defaultString));
    EXPECT_TRUE(defaultString.match(fooString));
    EXPECT_TRUE(fooString.match(defaultString));

    EXPECT_TRUE(fooString.match(fooString));
    EXPECT_TRUE(fumString.match(fumString));
    EXPECT_FALSE(fooString.match(fumString));

    EXPECT_EQ("", defaultString.getPart());
    EXPECT_EQ("foo", fooString.getPart());
    EXPECT_EQ("fum", fumString.getPart());

    EXPECT_TRUE(defaultString.getWildcard());
    EXPECT_FALSE(fooString.getWildcard());
    EXPECT_FALSE(fumString.getWildcard());
}

// Test the IntegerPart class
TEST(FeatureSupportUtilTest, IntegerPart)
{
    unsigned int zero = 0;
    unsigned int one = 1;
    IntegerPart defaultInteger;
    IntegerPart zeroInteger(0);
    IntegerPart oneInteger(1);
    EXPECT_TRUE(defaultInteger.match(defaultInteger));
    EXPECT_TRUE(defaultInteger.match(zeroInteger));
    EXPECT_TRUE(zeroInteger.match(defaultInteger));

    EXPECT_TRUE(zeroInteger.match(zeroInteger));
    EXPECT_TRUE(oneInteger.match(oneInteger));
    EXPECT_FALSE(zeroInteger.match(oneInteger));

    EXPECT_EQ(zero, defaultInteger.getPart());
    EXPECT_EQ(zero, zeroInteger.getPart());
    EXPECT_EQ(one, oneInteger.getPart());

    EXPECT_TRUE(defaultInteger.getWildcard());
    EXPECT_FALSE(zeroInteger.getWildcard());
    EXPECT_FALSE(oneInteger.getWildcard());
}

// Test the Version class
TEST(FeatureSupportUtilTest, Version)
{
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version defaultVersion;
    Version majorVersion1(one);
    Version majorVersion2(two);
    Version majorAndMinorVersion1Dot2(one, two);
    Version majorAndMinorVersion2Dot3(two, three);
    Version majorMinorAndSubMinorVersion1Dot2Dot3(one, two, three);
    Version majorMinorAndSubMinorVersion3Dot2Dot1(three, two, one);
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    EXPECT_TRUE(defaultVersion.match(defaultVersion));
    EXPECT_TRUE(defaultVersion.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(defaultVersion));
    EXPECT_TRUE(defaultVersion.match(majorAndMinorVersion1Dot2));
    EXPECT_TRUE(majorAndMinorVersion1Dot2.match(defaultVersion));
    EXPECT_TRUE(defaultVersion.match(majorMinorAndSubMinorVersion1Dot2Dot3));
    EXPECT_TRUE(majorMinorAndSubMinorVersion1Dot2Dot3.match(defaultVersion));
    EXPECT_TRUE(defaultVersion.match(fullVersion1Dot2Dot3Dot4));
    EXPECT_TRUE(fullVersion1Dot2Dot3Dot4.match(defaultVersion));

    EXPECT_TRUE(majorVersion1.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(majorAndMinorVersion1Dot2));
    EXPECT_TRUE(majorAndMinorVersion1Dot2.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(majorMinorAndSubMinorVersion1Dot2Dot3));
    EXPECT_TRUE(majorMinorAndSubMinorVersion1Dot2Dot3.match(majorVersion1));
    EXPECT_TRUE(majorVersion1.match(fullVersion1Dot2Dot3Dot4));
    EXPECT_TRUE(fullVersion1Dot2Dot3Dot4.match(majorVersion1));

    EXPECT_TRUE(majorAndMinorVersion1Dot2.match(majorMinorAndSubMinorVersion1Dot2Dot3));
    EXPECT_TRUE(majorMinorAndSubMinorVersion1Dot2Dot3.match(majorAndMinorVersion1Dot2));
    EXPECT_TRUE(majorAndMinorVersion1Dot2.match(fullVersion1Dot2Dot3Dot4));
    EXPECT_TRUE(fullVersion1Dot2Dot3Dot4.match(majorAndMinorVersion1Dot2));

    EXPECT_TRUE(majorMinorAndSubMinorVersion1Dot2Dot3.match(fullVersion1Dot2Dot3Dot4));
    EXPECT_TRUE(fullVersion1Dot2Dot3Dot4.match(majorMinorAndSubMinorVersion1Dot2Dot3));

    EXPECT_FALSE(majorVersion1.match(majorVersion2));
    EXPECT_FALSE(majorVersion2.match(majorVersion1));
    EXPECT_FALSE(majorVersion1.match(majorAndMinorVersion2Dot3));
    EXPECT_FALSE(majorAndMinorVersion2Dot3.match(majorVersion1));
    EXPECT_FALSE(majorVersion1.match(majorMinorAndSubMinorVersion3Dot2Dot1));
    EXPECT_FALSE(majorMinorAndSubMinorVersion3Dot2Dot1.match(majorVersion1));
    EXPECT_FALSE(majorVersion1.match(fullVersion4Dot3Dot2Dot1));
    EXPECT_FALSE(fullVersion4Dot3Dot2Dot1.match(majorVersion1));

    EXPECT_FALSE(majorAndMinorVersion1Dot2.match(majorAndMinorVersion2Dot3));
    EXPECT_FALSE(majorAndMinorVersion2Dot3.match(majorAndMinorVersion1Dot2));
    EXPECT_FALSE(majorAndMinorVersion1Dot2.match(majorMinorAndSubMinorVersion3Dot2Dot1));
    EXPECT_FALSE(majorMinorAndSubMinorVersion3Dot2Dot1.match(majorAndMinorVersion1Dot2));
    EXPECT_FALSE(majorAndMinorVersion1Dot2.match(fullVersion4Dot3Dot2Dot1));
    EXPECT_FALSE(fullVersion4Dot3Dot2Dot1.match(majorAndMinorVersion1Dot2));

    EXPECT_FALSE(majorMinorAndSubMinorVersion1Dot2Dot3.match(majorMinorAndSubMinorVersion3Dot2Dot1));
    EXPECT_FALSE(majorMinorAndSubMinorVersion3Dot2Dot1.match(majorMinorAndSubMinorVersion1Dot2Dot3));
    EXPECT_FALSE(majorMinorAndSubMinorVersion1Dot2Dot3.match(fullVersion4Dot3Dot2Dot1));
    EXPECT_FALSE(fullVersion4Dot3Dot2Dot1.match(majorMinorAndSubMinorVersion1Dot2Dot3));

    EXPECT_FALSE(fullVersion1Dot2Dot3Dot4.match(fullVersion4Dot3Dot2Dot1));
    EXPECT_FALSE(fullVersion4Dot3Dot2Dot1.match(fullVersion1Dot2Dot3Dot4));

    EXPECT_EQ("*", defaultVersion.getString());
    EXPECT_EQ("1.*", majorVersion1.getString());
    EXPECT_EQ("1.2.*", majorAndMinorVersion1Dot2.getString());
    EXPECT_EQ("1.2.3.*", majorMinorAndSubMinorVersion1Dot2Dot3.getString());
    EXPECT_EQ("1.2.3.4", fullVersion1Dot2Dot3Dot4.getString());
}




// Test the ListOf<T> class
TEST(FeatureSupportUtilTest, ListOf)
{
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    // FIXME/TODO - IMPLEMENT THIS TEST!!!
    EXPECT_TRUE(true);
}







// Test the Application class
TEST(FeatureSupportUtilTest, Application)
{
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version defaultVersion;
    Version majorVersion1(one);
    Version majorVersion2(two);
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    std::string app1 = "com.vendorA.app1";
    std::string app2 = "com.vendorB.app2";

    Application defaultApplication;
    Application app1NoVersion(app1);
    Application app2NoVersion(app2);
    Application app1Version1(app1, majorVersion1);
    Application app2Version2(app2, majorVersion2);
    Application app1Version1234(app1, fullVersion1Dot2Dot3Dot4);
    Application app1Version4321(app1, fullVersion4Dot3Dot2Dot1);
    Application app2Version4321(app2, fullVersion4Dot3Dot2Dot1);

    EXPECT_TRUE(defaultApplication.match(defaultApplication));
    EXPECT_TRUE(defaultApplication.match(app1NoVersion));
    EXPECT_TRUE(app1NoVersion.match(defaultApplication));
    EXPECT_TRUE(defaultApplication.match(app1Version4321));
    EXPECT_TRUE(app1Version4321.match(defaultApplication));

    EXPECT_TRUE(app1NoVersion.match(app1Version1));
    EXPECT_TRUE(app1Version1.match(app1NoVersion));
    EXPECT_TRUE(app1NoVersion.match(app1Version1234));
    EXPECT_TRUE(app1Version1234.match(app1NoVersion));
    EXPECT_TRUE(app1Version1.match(app1Version1234));
    EXPECT_TRUE(app1Version1234.match(app1Version1));

    EXPECT_FALSE(app1NoVersion.match(app2NoVersion));
    EXPECT_FALSE(app2NoVersion.match(app1NoVersion));

    EXPECT_FALSE(app1Version1234.match(app1Version4321));
    EXPECT_FALSE(app1Version4321.match(app1Version1234));
    EXPECT_FALSE(app1Version1234.match(app2Version4321));
    EXPECT_FALSE(app2Version4321.match(app1Version1234));
}

// Test the GPU class
TEST(FeatureSupportUtilTest, GPU)
{
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version defaultVersion;
    Version majorVersion1(one);
    Version majorVersion2(two);
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    std::string vendorA = "VendorA";
    std::string vendorB = "VendorB";

    GPU defaultGPU;
    GPU vendorANoDeviceNoVersion(vendorA);
    GPU vendorBNoDeviceNoVersion(vendorB);
    GPU vendorADevice1NoVersion(vendorA, one);
    GPU vendorADevice2NoVersion(vendorA, two);
    GPU vendorBDevice1NoVersion(vendorB, two);
    GPU vendorADevice1Version1(vendorA, one, majorVersion1);
    GPU vendorADevice2Version1(vendorA, two, majorVersion1);
    GPU vendorBDevice1Version2(vendorB, two, majorVersion2);
    GPU vendorADevice1Version1234(vendorA, one, fullVersion1Dot2Dot3Dot4);
    GPU vendorADevice2Version1234(vendorA, two, fullVersion1Dot2Dot3Dot4);
    GPU vendorADevice1Version4321(vendorA, one, fullVersion4Dot3Dot2Dot1);
    GPU vendorBDevice1Version4321(vendorB, one, fullVersion4Dot3Dot2Dot1);

    // Match the default with progressively, more-completely-specified GPUs:
    EXPECT_TRUE(defaultGPU.match(defaultGPU));
    EXPECT_TRUE(defaultGPU.match(vendorANoDeviceNoVersion));
    EXPECT_TRUE(vendorANoDeviceNoVersion.match(defaultGPU));
    EXPECT_TRUE(defaultGPU.match(vendorADevice1NoVersion));
    EXPECT_TRUE(vendorADevice1NoVersion.match(defaultGPU));
    EXPECT_TRUE(defaultGPU.match(vendorADevice1Version1));
    EXPECT_TRUE(vendorADevice1Version1.match(defaultGPU));
    EXPECT_TRUE(defaultGPU.match(vendorADevice1Version1234));
    EXPECT_TRUE(vendorADevice1Version1234.match(defaultGPU));

    // Match a vendor-A GPU with progressively, more-completely-specified GPUs:
    EXPECT_TRUE(vendorANoDeviceNoVersion.match(vendorANoDeviceNoVersion));
    EXPECT_TRUE(vendorANoDeviceNoVersion.match(vendorADevice1NoVersion));
    EXPECT_TRUE(vendorADevice1NoVersion.match(vendorADevice1NoVersion));
    EXPECT_TRUE(vendorANoDeviceNoVersion.match(vendorADevice1Version1));
    EXPECT_TRUE(vendorADevice1Version1.match(vendorANoDeviceNoVersion));
    EXPECT_TRUE(vendorANoDeviceNoVersion.match(vendorADevice1Version1234));
    EXPECT_TRUE(vendorADevice1Version1234.match(vendorANoDeviceNoVersion));

    // Match a vendor-A, Device-1 GPU with progressively, more-completely-specified GPUs:
    EXPECT_TRUE(vendorADevice1NoVersion.match(vendorADevice1NoVersion));
    EXPECT_TRUE(vendorADevice1NoVersion.match(vendorADevice1Version1));
    EXPECT_TRUE(vendorADevice1Version1.match(vendorADevice1NoVersion));
    EXPECT_TRUE(vendorADevice1NoVersion.match(vendorADevice1Version1234));
    EXPECT_TRUE(vendorADevice1Version1234.match(vendorADevice1NoVersion));

    // Match a completely-specified GPU:
    EXPECT_TRUE(vendorADevice1Version1234.match(vendorADevice1Version1234));

    // DON'T MATCH a vendor-A GPU with non-matching progressively, more-completely-specified GPUs:
    EXPECT_FALSE(vendorANoDeviceNoVersion.match(vendorBNoDeviceNoVersion));
    EXPECT_FALSE(vendorANoDeviceNoVersion.match(vendorBDevice1NoVersion));
    EXPECT_FALSE(vendorBDevice1NoVersion.match(vendorANoDeviceNoVersion));
    EXPECT_FALSE(vendorANoDeviceNoVersion.match(vendorBDevice1Version2));
    EXPECT_FALSE(vendorBDevice1Version2.match(vendorANoDeviceNoVersion));
    EXPECT_FALSE(vendorANoDeviceNoVersion.match(vendorBDevice1Version4321));
    EXPECT_FALSE(vendorBDevice1Version4321.match(vendorANoDeviceNoVersion));

    // DON'T MATCH a vendor-A, Device-1 GPU with non-matching progressively,
    // more-completely-specified GPUs:
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorADevice2NoVersion));
    EXPECT_FALSE(vendorADevice2NoVersion.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorBDevice1NoVersion));
    EXPECT_FALSE(vendorBDevice1NoVersion.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorADevice2Version1));
    EXPECT_FALSE(vendorADevice2Version1.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorBDevice1Version2));
    EXPECT_FALSE(vendorBDevice1Version2.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorADevice2Version1234));
    EXPECT_FALSE(vendorADevice2Version1234.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorADevice2Version1234));
    EXPECT_FALSE(vendorADevice2Version1234.match(vendorADevice1NoVersion));
    EXPECT_FALSE(vendorADevice1NoVersion.match(vendorBDevice1Version4321));
    EXPECT_FALSE(vendorBDevice1Version4321.match(vendorADevice1NoVersion));

    // DON'T MATCH a vendor-A, Device-1, Version-1.* GPU with non-matching progressively,
    // more-completely-specified GPUs:
    EXPECT_FALSE(vendorADevice1Version1.match(vendorADevice2Version1));
    EXPECT_FALSE(vendorADevice2Version1.match(vendorADevice1Version1));
    EXPECT_FALSE(vendorADevice1Version1.match(vendorBDevice1Version2));
    EXPECT_FALSE(vendorBDevice1Version2.match(vendorADevice1Version1));
    EXPECT_FALSE(vendorADevice1Version1.match(vendorADevice2Version1234));
    EXPECT_FALSE(vendorADevice2Version1234.match(vendorADevice1Version1));
    EXPECT_FALSE(vendorADevice1Version1.match(vendorADevice1Version4321));
    EXPECT_FALSE(vendorADevice1Version4321.match(vendorADevice1Version1));
    EXPECT_FALSE(vendorADevice1Version1.match(vendorBDevice1Version4321));
    EXPECT_FALSE(vendorBDevice1Version4321.match(vendorADevice1Version1));

    // DON'T MATCH a completely-specified GPU with non-matching completely-specified GPUs:
    EXPECT_FALSE(vendorADevice1Version1234.match(vendorADevice2Version1234));
    EXPECT_FALSE(vendorADevice2Version1234.match(vendorADevice1Version1234));
    EXPECT_FALSE(vendorADevice1Version1234.match(vendorADevice1Version4321));
    EXPECT_FALSE(vendorADevice1Version4321.match(vendorADevice1Version1234));
    EXPECT_FALSE(vendorADevice1Version1234.match(vendorBDevice1Version4321));
    EXPECT_FALSE(vendorBDevice1Version4321.match(vendorADevice1Version1234));
}






#if 0
// Test the Device class
TEST(FeatureSupportUtilTest, Device)
{
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version defaultVersion;
    Version majorVersion1(one);
    Version majorVersion2(two);
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    std::string vendorA = "VendorA";
    std::string vendorB = "VendorB";

    GPU defaultGPU;
    GPU gpuVendorADevice1Version1234(vendorA, one, fullVersion1Dot2Dot3Dot4);
    GPU gpuVendorADevice1Version4321(vendorA, one, fullVersion4Dot3Dot2Dot1);
    GPU gpuVendorBDevice1Version4321(vendorB, one, fullVersion4Dot3Dot2Dot1);

    std::string mfrFoo = "Foo";
    std::string mfrFum = "Fum";
    std::string modelX = "X";
    std::string modelY = "Y";
    std::string modelZ = "Z";

    Device defaultDevice;
    Device mfrFoo(mfrFoo);
    Device mfrFum(mfrFum);
    Device mfrFooModelX(mfrFoo, modelX);
    Device mfrFooModelY(mfrFoo, modelY);
    Device mfrFumModelX(mfrFum, modelX);
    Device mfrFumModelY(mfrFum, modelY);
    Device mfrFooModelXGpuAOne1234(mfrFoo, modelX);
    mfrFooModelZGpuAOne1234.addGPU(gpuVendorADevice1Version1234);
    Device mfrFooModelXGpuAOne4321(mfrFoo, modelX);
    mfrFooModelZGpuAOne1234.addGPU(gpuVendorADevice1Version4321);
    Device mfrFumModelXGpuAOne4321(mfrFum, modelY);
    mfrFumModelXGpuAOne4321.addGPU(gpuVendorADevice1Version4321);
    Device mfrFooModelXWithManyGpus(mfrFoo, modelX);
    mfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version1234);
    mfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version4321);
    mfrFooModelXWithManyGpus.addGPU(gpuVendorBDevice1Version4321);

    // Match the default with progressively, more-completely-specified Devices:
    EXPECT_TRUE(defaultDevice.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(mfrFoo));
    EXPECT_TRUE(mfrFoo.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(mfrFooModelX));
    EXPECT_TRUE(mfrFooModelX.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(mfrFooModelXGpuAOne1234));
    EXPECT_TRUE(mfrFooModelXGpuAOne1234.match(defaultDevice));

    // Match a mfr-Foo Device with progressively, more-completely-specified Devices:
    EXPECT_TRUE(mfrFoo.match(mfrFoo));
    EXPECT_TRUE(mfrFoo.match(mfrFooModelX));
    EXPECT_TRUE(mfrFooModelX.match(mfrFoo));
    EXPECT_TRUE(mfrFoo.match(mfrFooModelXGpuAOne1234));
    EXPECT_TRUE(mfrFooModelXGpuAOne1234.match(mfrFoo));

    // Match a mfr-Foo, model-X Device with progressively, more-completely-specified Devices:
    EXPECT_TRUE(mfrFooModelX.match(mfrFooModelX));
    EXPECT_TRUE(mfrFooModelX.match(mfrFooModelXGpuAOne1234));
    EXPECT_TRUE(mfrFooModelXGpuAOne1234.match(mfrFooModelX));

    // Match a completely-specified Device:
    EXPECT_TRUE(mfrFooModelXGpuAOne1234.match(mfrFooModelXGpuAOne1234));

    // DON'T MATCH a mfr-Foo Device with non-matching progressively,
    // more-completely-specified Devices:
    EXPECT_FALSE(mfrFoo.match(mfrFum));
    EXPECT_FALSE(mfrFoo.match(mfrFumModelX));
    EXPECT_FALSE(mfrFumModelX.match(mfrFoo));
    EXPECT_FALSE(mfrFoo.match(mfrFumModelXGpuAOne4321));
    EXPECT_FALSE(mfrFumModelXGpuAOne4321.match(mfrFoo));

    // DON'T MATCH a mfr-Foo, model-X Device with non-matching progressively,
    // more-completely-specified Devices:
    EXPECT_FALSE(mfrFooModelX.match(mfrFooModelY));
    EXPECT_FALSE(mfrFooModelY.match(mfrFooModelX));
    EXPECT_FALSE(mfrFooModelX.match(mfrFumModelX));
    EXPECT_FALSE(mfrFumModelX.match(mfrFooModelX));
    EXPECT_FALSE(mfrFumModelY.match(mfrFooModelX));
    EXPECT_FALSE(mfrFooModelX.match(mfrFumModelXGpuAOne4321));
    EXPECT_FALSE(mfrFumModelXGpuAOne4321.match(mfrFooModelX));

    // DON'T MATCH a completely-specified Device with completely-specified Devices:
//    EXPECT_FALSE(mfrFooModelXGpuAOne1234.match(vendorADevice2Version1));

    // FIXME/TODO: TEST MORE POSITIVE AND NEGATIVE COMBINATIONS!!!
    // FIXME/TODO: TEST MORE POSITIVE AND NEGATIVE COMBINATIONS!!!
    // FIXME/TODO: TEST MORE POSITIVE AND NEGATIVE COMBINATIONS!!!
    // FIXME/TODO: TEST MORE POSITIVE AND NEGATIVE COMBINATIONS!!!
    // FIXME/TODO: TEST MORE POSITIVE AND NEGATIVE COMBINATIONS!!!
}
#endif





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

#if 0
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
#endif
