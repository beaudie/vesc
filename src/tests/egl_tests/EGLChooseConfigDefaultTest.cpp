//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLChooseConfigDefaultTest.cpp:
//   Tests of proper default-value semantics for eglChooseConfig

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace angle
{
class EGLChooseConfigDefaultTest : public ANGLETest
{
  protected:
    EGLChooseConfigDefaultTest() {}
};

// Test that the EGL_COLOR_BUFFER_TYPE is defaulted to EGL_RGB_BUFFER
TEST_P(EGLChooseConfigDefaultTest, ColorBufferType)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that match the default value for EGL_COLOR_BUFFER_TYPE, which is
    // EGL_RGB_BUFFER:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);

    // Check that the default configs all have the default attribute values:
    for (EGLConfig config : defaultConfigs)
    {
        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType;

        eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        ASSERT_EQ(colorBufferType, EGL_RGB_BUFFER);

        eglGetConfigAttrib(display, config, EGL_LEVEL, &level);
        ASSERT_EQ(level, 0);

        eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &renderableType);
        ASSERT_EQ(renderableType & EGL_OPENGL_ES_BIT, EGL_OPENGL_ES_BIT);

        eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EQ(surfaceType & EGL_WINDOW_BIT, EGL_WINDOW_BIT);

        eglGetConfigAttrib(display, config, EGL_TRANSPARENT_TYPE, &transparentType);
        ASSERT_EQ(transparentType, EGL_NONE);

        eglGetConfigAttrib(display, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
        ASSERT_EQ(colorComponentType, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT);
    }
}

}  // namespace angle

ANGLE_INSTANTIATE_TEST(EGLChooseConfigDefaultTest, ES2_D3D11(), ES2_OPENGL(), ES2_VULKAN());
