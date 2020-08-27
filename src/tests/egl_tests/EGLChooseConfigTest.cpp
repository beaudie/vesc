//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLChooseConfigTest.cpp:
//   Tests of proper default-value semantics for eglChooseConfig

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace angle
{
class EGLChooseConfigTest : public ANGLETest
{
  protected:
    EGLChooseConfigTest() : mDisplay(0) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
    }

    void testTearDown() override { eglTerminate(mDisplay); }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

// Test that the EGL_COLOR_BUFFER_TYPE is defaulted to EGL_RGB_BUFFER
TEST_P(EGLChooseConfigTest, Defaults)
{
    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that have the default attribute values:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);
    defaultConfigs.resize(defaultConfigCount);

    // Check that the default configs all have the default attribute values we care about:
    for (EGLConfig config : defaultConfigs)
    {
        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType;

        eglGetConfigAttrib(mDisplay, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        ASSERT_EQ(colorBufferType, EGL_RGB_BUFFER);

        eglGetConfigAttrib(mDisplay, config, EGL_LEVEL, &level);
        ASSERT_EQ(level, 0);

        eglGetConfigAttrib(mDisplay, config, EGL_RENDERABLE_TYPE, &renderableType);
        ASSERT_EQ(renderableType & EGL_OPENGL_ES_BIT, EGL_OPENGL_ES_BIT);

        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EQ(surfaceType & EGL_WINDOW_BIT, EGL_WINDOW_BIT);

        eglGetConfigAttrib(mDisplay, config, EGL_TRANSPARENT_TYPE, &transparentType);
        ASSERT_EQ(transparentType, EGL_NONE);

        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
            ASSERT_EQ(colorComponentType, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT);
        }
    }

    // Check that all of the configs that have the default attribute values are defaultConfigs,
    // and all that don't aren't:
    for (EGLConfig config : allConfigs)
    {
        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

        eglGetConfigAttrib(mDisplay, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        eglGetConfigAttrib(mDisplay, config, EGL_LEVEL, &level);
        eglGetConfigAttrib(mDisplay, config, EGL_RENDERABLE_TYPE, &renderableType);
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        eglGetConfigAttrib(mDisplay, config, EGL_TRANSPARENT_TYPE, &transparentType);
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
        }

        bool isADefault =
            ((colorBufferType == EGL_RGB_BUFFER) && (level == 0) &&
             ((renderableType & EGL_OPENGL_ES_BIT) == EGL_OPENGL_ES_BIT) &&
             ((surfaceType & EGL_WINDOW_BIT) == EGL_WINDOW_BIT) && (transparentType == EGL_NONE) &&
             (colorComponentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT));
        EGLint thisConfigID;
        eglGetConfigAttrib(mDisplay, config, EGL_CONFIG_ID, &thisConfigID);
        bool foundInDefaultConfigs = false;
        // Attempt to find this config ID in defaultConfigs:
        for (EGLConfig defaultConfig : defaultConfigs)
        {
            EGLint defaultConfigID;
            eglGetConfigAttrib(mDisplay, defaultConfig, EGL_CONFIG_ID, &defaultConfigID);
            if (defaultConfigID == thisConfigID)
            {
                foundInDefaultConfigs = true;
            }
        }
        ASSERT_EQ(isADefault, foundInDefaultConfigs);
    }
}

}  // namespace angle

ANGLE_INSTANTIATE_TEST(EGLChooseConfigTest,
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()));
