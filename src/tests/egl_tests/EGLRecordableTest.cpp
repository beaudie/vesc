//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLRecordableTest.cpp:
//   Tests of EGL_ANDROID_recordable extension

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

namespace angle
{
class EGLRecordableTest : public ANGLETest
{
  protected:
    EGLRecordableTest() : mDisplay(0) {}

    void SetUp() override
    {
        ANGLETest::SetUp();

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
    }

    void TearDown() override {}

    bool hasExtension() const
    {
        return ANGLETest::eglDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_recordable");
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

// Test that the extension is always available (it is implemented in ANGLE's frontend).
TEST_P(EGLRecordableTest, ExtensionAlwaysAvailable)
{
    ASSERT_TRUE(hasExtension());
}

// Check that the default message filters and callbacks are correct
TEST_P(EGLRecordableTest, CheckAllContexts)
{
    ANGLE_SKIP_TEST_IF(!hasExtension());

    int nConfigs = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs));
    ASSERT_TRUE(nConfigs != 0);

    int nReturnedConfigs = 0;
    std::vector<EGLConfig> configs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), nConfigs, &nReturnedConfigs));
    ASSERT_TRUE(nConfigs == nReturnedConfigs);

    for (auto config : configs)
    {
        EGLint isRecordable;
        eglGetConfigAttrib(mDisplay, config, EGL_RECORDABLE_ANDROID, &isRecordable);
        ASSERT_FALSE(isRecordable);
    }

    const EGLint configAttributes[] = {
        EGL_RED_SIZE,     EGL_DONT_CARE,  EGL_GREEN_SIZE,         EGL_DONT_CARE,  EGL_BLUE_SIZE,
        EGL_DONT_CARE,    EGL_ALPHA_SIZE, EGL_DONT_CARE,          EGL_DEPTH_SIZE, EGL_DONT_CARE,
        EGL_STENCIL_SIZE, EGL_DONT_CARE,  EGL_RECORDABLE_ANDROID, EGL_FALSE,      EGL_NONE};
    EGLint configCount;
    ASSERT_TRUE(
        eglChooseConfig(mDisplay, configAttributes, configs.data(), configs.size(), &configCount));
}

}  // namespace angle
