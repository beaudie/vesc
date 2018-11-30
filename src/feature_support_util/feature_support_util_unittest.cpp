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
    GPU gpuVendorADevice2Version1234(vendorA, two, fullVersion1Dot2Dot3Dot4);

    std::string mfrFoo = "Foo";
    std::string mfrFum = "Fum";
    std::string modelX = "X";
    std::string modelY = "Y";

    Device defaultDevice;
    Device deviceMfrFoo(mfrFoo);
    Device deviceMfrFum(mfrFum);
    Device deviceMfrFooModelX(mfrFoo, modelX);
    Device deviceMfrFooModelY(mfrFoo, modelY);
    Device deviceMfrFumModelX(mfrFum, modelX);
    Device deviceMfrFumModelY(mfrFum, modelY);
    Device deviceMfrFooModelXGpuAOne1234(mfrFoo, modelX);
    deviceMfrFooModelXGpuAOne1234.addGPU(gpuVendorADevice1Version1234);
    Device deviceMfrFooModelXGpuAOne4321(mfrFoo, modelX);
    deviceMfrFooModelXGpuAOne4321.addGPU(gpuVendorADevice1Version4321);
    Device deviceMfrFooModelXGpuBOne4321(mfrFoo, modelX);
    deviceMfrFooModelXGpuBOne4321.addGPU(gpuVendorBDevice1Version4321);
    Device deviceMfrFooModelXGpuATwo1234(mfrFoo, modelX);
    deviceMfrFooModelXGpuATwo1234.addGPU(gpuVendorADevice2Version1234);
    Device deviceMfrFumModelXGpuAOne4321(mfrFum, modelX);
    deviceMfrFumModelXGpuAOne4321.addGPU(gpuVendorADevice1Version4321);
    Device deviceMfrFooModelXWithManyGpus(mfrFoo, modelX);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version1234);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version4321);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorBDevice1Version4321);

    // Match the default with progressively, more-completely-specified Devices:
    EXPECT_TRUE(defaultDevice.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(deviceMfrFoo));
    EXPECT_TRUE(deviceMfrFoo.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(deviceMfrFooModelX));
    EXPECT_TRUE(deviceMfrFooModelX.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne1234.match(defaultDevice));
    EXPECT_TRUE(defaultDevice.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(defaultDevice));

    // Match a deviceMfr-Foo Device with progressively, more-completely-specified Devices:
    EXPECT_TRUE(deviceMfrFoo.match(deviceMfrFoo));
    EXPECT_TRUE(deviceMfrFoo.match(deviceMfrFooModelX));
    EXPECT_TRUE(deviceMfrFooModelX.match(deviceMfrFoo));
    EXPECT_TRUE(deviceMfrFoo.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFoo));
    EXPECT_TRUE(deviceMfrFoo.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFoo));

    // Match a deviceMfr-Foo, model-X Device with progressively, more-completely-specified Devices:
    EXPECT_TRUE(deviceMfrFooModelX.match(deviceMfrFooModelX));
    EXPECT_TRUE(deviceMfrFooModelX.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFooModelX));
    EXPECT_TRUE(deviceMfrFooModelX.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFooModelX));

    // Match a completely-specified Device:
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFooModelXGpuAOne1234));

    // Match a completely-specified Device with multiple GPUs:
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_TRUE(deviceMfrFooModelXGpuAOne4321.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFooModelXGpuAOne4321));
    EXPECT_TRUE(deviceMfrFooModelXGpuBOne4321.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_TRUE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFooModelXGpuBOne4321));

    // DON'T MATCH a deviceMfr-Foo Device with non-matching progressively,
    // more-completely-specified Devices:
    EXPECT_FALSE(deviceMfrFoo.match(deviceMfrFum));
    EXPECT_FALSE(deviceMfrFoo.match(deviceMfrFumModelX));
    EXPECT_FALSE(deviceMfrFumModelX.match(deviceMfrFoo));
    EXPECT_FALSE(deviceMfrFoo.match(deviceMfrFumModelXGpuAOne4321));
    EXPECT_FALSE(deviceMfrFumModelXGpuAOne4321.match(deviceMfrFoo));

    // DON'T MATCH a deviceMfr-Foo, model-X Device with non-matching progressively,
    // more-completely-specified Devices:
    EXPECT_FALSE(deviceMfrFooModelX.match(deviceMfrFooModelY));
    EXPECT_FALSE(deviceMfrFooModelY.match(deviceMfrFooModelX));
    EXPECT_FALSE(deviceMfrFooModelX.match(deviceMfrFumModelX));
    EXPECT_FALSE(deviceMfrFumModelX.match(deviceMfrFooModelX));
    EXPECT_FALSE(deviceMfrFumModelY.match(deviceMfrFooModelX));
    EXPECT_FALSE(deviceMfrFooModelX.match(deviceMfrFumModelXGpuAOne4321));
    EXPECT_FALSE(deviceMfrFumModelXGpuAOne4321.match(deviceMfrFooModelX));

    // DON'T MATCH a completely-specified Device with completely-specified Devices:
    EXPECT_FALSE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFumModelXGpuAOne4321));
    EXPECT_FALSE(deviceMfrFumModelXGpuAOne4321.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_FALSE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFooModelXGpuBOne4321));
    EXPECT_FALSE(deviceMfrFooModelXGpuBOne4321.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_FALSE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFooModelXGpuATwo1234));
    EXPECT_FALSE(deviceMfrFooModelXGpuATwo1234.match(deviceMfrFooModelXGpuAOne1234));
    EXPECT_FALSE(deviceMfrFooModelXGpuAOne1234.match(deviceMfrFumModelXGpuAOne4321));
    EXPECT_FALSE(deviceMfrFumModelXGpuAOne4321.match(deviceMfrFooModelXGpuAOne1234));

    // DON'T MATCH a completely-specified Device with multiple GPUs (none of which match):
    EXPECT_FALSE(deviceMfrFooModelXGpuATwo1234.match(deviceMfrFooModelXWithManyGpus));
    EXPECT_FALSE(deviceMfrFooModelXWithManyGpus.match(deviceMfrFooModelXGpuATwo1234));
}

// Test the Scenario, Rule, and RuleList classes
TEST(FeatureSupportUtilTest, RuleList)
{
    unsigned int one = 1;
    unsigned int two = 2;
    unsigned int three = 3;
    unsigned int four = 4;

    Version defaultVersion;
    Version fullVersion1Dot2Dot3Dot4(one, two, three, four);
    Version fullVersion4Dot3Dot2Dot1(four, three, two, one);

    std::string app1 = "com.vendorA.app1";
    std::string app2 = "com.vendorB.app2";
    Application defaultApplication;
    Application app1NoVersion(app1);
    Application app1Version1234(app1, fullVersion1Dot2Dot3Dot4);
    Application app1Version4321(app1, fullVersion4Dot3Dot2Dot1);
    Application app2Version4321(app2, fullVersion4Dot3Dot2Dot1);

    std::string vendorA = "VendorA";
    std::string vendorB = "VendorB";
    GPU defaultGPU;
    GPU gpuVendorADevice1Version1234(vendorA, one, fullVersion1Dot2Dot3Dot4);
    GPU gpuVendorADevice1Version4321(vendorA, one, fullVersion4Dot3Dot2Dot1);
    GPU gpuVendorBDevice1Version4321(vendorB, one, fullVersion4Dot3Dot2Dot1);
    GPU gpuVendorADevice2Version1234(vendorA, two, fullVersion1Dot2Dot3Dot4);

    std::string mfrFoo = "Foo";
    std::string mfrFum = "Fum";
    std::string modelX = "X";
    std::string modelY = "Y";
    Device defaultDevice;
    Device deviceMfrFum(mfrFum);
    Device deviceMfrFooModelX(mfrFoo, modelX);
    Device deviceMfrFooModelY(mfrFoo, modelY);
    Device deviceMfrFumModelX(mfrFum, modelX);
    Device deviceMfrFumModelY(mfrFum, modelY);
    Device deviceMfrFooModelXGpuAOne1234(mfrFoo, modelX);
    deviceMfrFooModelXGpuAOne1234.addGPU(gpuVendorADevice1Version1234);
    Device deviceMfrFooModelXGpuAOne4321(mfrFoo, modelX);
    deviceMfrFooModelXGpuAOne4321.addGPU(gpuVendorADevice1Version4321);
    Device deviceMfrFooModelXGpuBOne4321(mfrFoo, modelX);
    deviceMfrFooModelXGpuBOne4321.addGPU(gpuVendorBDevice1Version4321);
    Device deviceMfrFooModelXGpuATwo1234(mfrFoo, modelX);
    deviceMfrFooModelXGpuATwo1234.addGPU(gpuVendorADevice2Version1234);
    Device deviceMfrFumModelXGpuAOne4321(mfrFum, modelX);
    deviceMfrFumModelXGpuAOne4321.addGPU(gpuVendorADevice1Version4321);
    Device deviceMfrFooModelXWithManyGpus(mfrFoo, modelX);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version1234);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorADevice1Version4321);
    deviceMfrFooModelXWithManyGpus.addGPU(gpuVendorBDevice1Version4321);

    Rule ruleDefault("Default", false);
    Rule ruleSupportedApps("SupportedApps", true);
    ruleSupportedApps.addApp(app1NoVersion);
    ruleSupportedApps.addApp(app2Version4321);
    Rule ruleApp1Exception("ruleApp1Exception", false);
    ruleApp1Exception.addApp(app1Version1234);
    ruleApp1Exception.addDevice(deviceMfrFooModelXGpuAOne1234);
    Rule ruleNoMfrFumDevices("ruleNoMfrFumDevices", false);
    ruleNoMfrFumDevices.addDevice(deviceMfrFum);

    RuleList rules;
    rules.addRule(ruleDefault);
    rules.addRule(ruleSupportedApps);
    rules.addRule(ruleApp1Exception);
    rules.addRule(ruleNoMfrFumDevices);

    Scenario scApp1Ver1234_FooXAOne1234(app1.c_str(), mfrFoo.c_str(), modelX.c_str());
    scApp1Ver1234_FooXAOne1234.mDevice.addGPU(gpuVendorADevice1Version1234);
    EXPECT_FALSE(rules.getUseANGLE(scApp1Ver1234_FooXAOne1234));

    Scenario scApp1Ver1234_FooXAOne4321(app1.c_str(), mfrFoo.c_str(), modelX.c_str());
    scApp1Ver1234_FooXAOne4321.mDevice.addGPU(gpuVendorADevice1Version4321);
    EXPECT_TRUE(rules.getUseANGLE(scApp1Ver1234_FooXAOne4321));
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

// Test the ANGLEAndroidParseRulesString and ANGLEShouldBeUsedForApplication functions
TEST(FeatureSupportUtilTest, TestRuleProcessing)
{
    // TODO(ianelliott): Replace this with a gtest "fixture", per review feedback.
    SystemInfo systemInfo;
    systemInfo.machineManufacturer = "MfrFoo";
    systemInfo.machineModelName    = "ModelX";
    systemInfo.gpus.resize(1);
    systemInfo.gpus[0].vendorId              = 123;
    systemInfo.gpus[0].deviceId              = 234;
    systemInfo.gpus[0].driverVendor          = "DriverVendorA";
    systemInfo.gpus[0].detailedDriverVersion = {1, 2, 3, 4};

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
        "                    \"Manufacturer\":\"MfrFoo\"\n"
        "                    \"GPUs\":[\n"
        "                        {\n"
        "                            \"Vendor\" : \"GPUVendorA\"\n"
        "                            \"DeviceId\" : 234\n"
        "                            \"VerMajor\" : 1, \"VerMinor\" : 2}\n"
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
    ANGLEFreeRulesHandle(rulesHandle);

    // FIXME/TODO: Finish the rest of this test!!!  ALSO: redo the previous test, etc.
    // FIXME/TODO: Finish the rest of this test!!!  ALSO: redo the previous test, etc.
    // FIXME/TODO: Finish the rest of this test!!!  ALSO: redo the previous test, etc.

}
