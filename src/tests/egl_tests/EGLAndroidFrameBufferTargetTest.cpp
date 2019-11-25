//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLAndroidFramebufferTargetTest.cpp:
//   This test verifies the extension EGL_ANDROID_framebuffer_target
// 1. When the EGLFRAME_BUFFER_TARGET_ANDROID attribute is used with eglChooseConfig
// It should match with configs according to Config selection rules and the extension
//

#include <gtest/gtest.h>

#include "common/string_utils.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLAndroidFramebufferTargetTest : public ANGLETest
{
  protected:
    EGLAndroidFramebufferTargetTest() {}

    void testSetUp() override
    {
        mDisplay = getEGLWindow()->getDisplay();
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

// Query a EGL attribute
static EGLint GetAttrib(EGLDisplay display, EGLConfig config, EGLint attrib)
{
    EGLint value = 0;
    EXPECT_EGL_TRUE(eglGetConfigAttrib(display, config, attrib, &value));
    return value;
}

// Choose configs matching the extension
TEST_P(EGLAndroidFramebufferTargetTest, MatchFramebufferTargetConfigs)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_framebuffer_target"));

    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count));
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count));
    configs.resize(count);
    // Filter out all non-frmaebuffertarget configs
    std::vector<EGLConfig> filterConfigs(0);
    for (auto config : configs)
    {
        if (EGL_TRUE == GetAttrib(mDisplay, config, EGL_FRAMEBUFFER_TARGET_ANDROID))
        {
            filterConfigs.push_back(config);
        }
    }
    // sort configs by increaing ID
    std::sort(filterConfigs.begin(), filterConfigs.end(), [this](EGLConfig a, EGLConfig b) -> bool {
        return GetAttrib(mDisplay, a, EGL_CONFIG_ID) < GetAttrib(mDisplay, b, EGL_CONFIG_ID);
    });
    std::cout << "eglGetConfigs Filtered Configs: ";
    for (auto config : filterConfigs)
    {
        // Config ID
        std::cout << GetAttrib(mDisplay, config, EGL_CONFIG_ID) << ", ";
    }
    std::cout << std::endl;
    std::cout << "Configs containing framebuffer_target attribute - Count: " << filterConfigs.size()
              << std::endl;

    // Now get configs that selection algorithm identifies
    EGLint attribs[] = {EGL_FRAMEBUFFER_TARGET_ANDROID,
                        EGL_TRUE,
                        EGL_COLOR_BUFFER_TYPE,
                        EGL_DONT_CARE,
                        EGL_COLOR_COMPONENT_TYPE_EXT,
                        EGL_DONT_CARE,
                        EGL_NONE};
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, nullptr, 0, &count));
    std::vector<EGLConfig> matchConfigs(count);
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, matchConfigs.data(), count, &count));
    matchConfigs.resize(count);
    // sort configs by increaing ID
    std::sort(matchConfigs.begin(), matchConfigs.end(), [this](EGLConfig a, EGLConfig b) -> bool {
        return GetAttrib(mDisplay, a, EGL_CONFIG_ID) < GetAttrib(mDisplay, b, EGL_CONFIG_ID);
    });
    std::cout << "eglChooseConfig Matching Configs: ";
    for (auto config : matchConfigs)
    {
        // Config ID
        std::cout << GetAttrib(mDisplay, config, EGL_CONFIG_ID) << ", ";
    }
    std::cout << std::endl;
    std::cout << "Configs matching framebuffer_target attribute - Count: " << matchConfigs.size()
              << std::endl;

    std::cout << "Filtered == Matching: " << ((matchConfigs == filterConfigs) ? "TRUE" : "FALSE")
              << std::endl;
}

ANGLE_INSTANTIATE_TEST(EGLAndroidFramebufferTargetTest, ES2_VULKAN(), ES3_VULKAN());
