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

    // Choose configs with the explicit value of EGL_RGB_BUFFER for EGL_COLOR_BUFFER_TYPE:
    const EGLint explicitConfigAttributes[] = {EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER, EGL_NONE};
    EGLint explicitConfigCount;
    std::vector<EGLConfig> explicitConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, explicitConfigAttributes, explicitConfigs.data(),
                                    explicitConfigs.size(), &explicitConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(defaultConfigCount, explicitConfigCount);

    // Choose configs with the explicit value of EGL_LUMINANCE_BUFFER for EGL_COLOR_BUFFER_TYPE:
    const EGLint alternateConfigAttributes[] = {EGL_COLOR_BUFFER_TYPE, EGL_LUMINANCE_BUFFER,
                                                EGL_NONE};
    EGLint alternateConfigCount;
    std::vector<EGLConfig> alternateConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, alternateConfigAttributes, alternateConfigs.data(),
                                    alternateConfigs.size(), &alternateConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(alternateConfigCount, allConfigCount);
    // Because of other attributes, explicitConfigCount + alternateConfigCount
    // may still not add up to allConfigCount:
    ASSERT_LE(explicitConfigCount + alternateConfigCount, allConfigCount);
}

// Test that the EGL_LEVEL is defaulted to 0
TEST_P(EGLChooseConfigDefaultTest, Level)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that match the default value for EGL_LEVEL, which is 0:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);

    // Choose configs with the explicit value of 0 for EGL_LEVEL:
    const EGLint explicitConfigAttributes[] = {EGL_LEVEL, 0, EGL_NONE};
    EGLint explicitConfigCount;
    std::vector<EGLConfig> explicitConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, explicitConfigAttributes, explicitConfigs.data(),
                                    explicitConfigs.size(), &explicitConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(defaultConfigCount, explicitConfigCount);

    // Choose configs with the value of EGL_DONT_CARE for EGL_LEVEL:
    const EGLint alternateConfigAttributes[] = {EGL_LEVEL, EGL_DONT_CARE, EGL_NONE};
    EGLint alternateConfigCount;
    std::vector<EGLConfig> alternateConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, alternateConfigAttributes, alternateConfigs.data(),
                                    alternateConfigs.size(), &alternateConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(alternateConfigCount, allConfigCount);
}

// Test that the EGL_RENDERABLE_TYPE is defaulted to EGL_OPENGL_ES_BIT
TEST_P(EGLChooseConfigDefaultTest, RenderableType)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that match the default value for EGL_RENDERABLE_TYPE, which is
    // EGL_OPENGL_ES_BIT:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);

    // Choose configs with the explicit value of EGL_OPENGL_ES_BIT for EGL_RENDERABLE_TYPE:
    const EGLint explicitConfigAttributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT, EGL_NONE};
    EGLint explicitConfigCount;
    std::vector<EGLConfig> explicitConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, explicitConfigAttributes, explicitConfigs.data(),
                                    explicitConfigs.size(), &explicitConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(defaultConfigCount, explicitConfigCount);

    // Choose configs with the value of EGL_DONT_CARE for EGL_RENDERABLE_TYPE:
    const EGLint alternateConfigAttributes[] = {EGL_RENDERABLE_TYPE, EGL_DONT_CARE, EGL_NONE};
    EGLint alternateConfigCount;
    std::vector<EGLConfig> alternateConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, alternateConfigAttributes, alternateConfigs.data(),
                                    alternateConfigs.size(), &alternateConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(alternateConfigCount, allConfigCount);
}

// Test that the EGL_TRANSPARENT_TYPE is defaulted to EGL_NONE
TEST_P(EGLChooseConfigDefaultTest, TransparentType)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that match the default value for EGL_TRANSPARENT_TYPE, which is
    // EGL_NONE:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);

    // Choose configs with the explicit value of EGL_NONE for EGL_TRANSPARENT_TYPE:
    const EGLint explicitConfigAttributes[] = {EGL_TRANSPARENT_TYPE, EGL_NONE, EGL_NONE};
    EGLint explicitConfigCount;
    std::vector<EGLConfig> explicitConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, explicitConfigAttributes, explicitConfigs.data(),
                                    explicitConfigs.size(), &explicitConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(defaultConfigCount, explicitConfigCount);

    // Choose configs with the explicit value of EGL_TRANSPARENT_RGB for EGL_TRANSPARENT_TYPE:
    const EGLint alternateConfigAttributes[] = {EGL_TRANSPARENT_TYPE, EGL_TRANSPARENT_RGB,
                                                EGL_NONE};
    EGLint alternateConfigCount;
    std::vector<EGLConfig> alternateConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, alternateConfigAttributes, alternateConfigs.data(),
                                    alternateConfigs.size(), &alternateConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(alternateConfigCount, allConfigCount);
    // Because of other attributes, explicitConfigCount + alternateConfigCount
    // may still not add up to allConfigCount:
    ASSERT_LE(explicitConfigCount + alternateConfigCount, allConfigCount);
}

// Test that the EGL_COLOR_COMPONENT_TYPE_EXT is defaulted to EGL_COLOR_COMPONENT_TYPE_FIXED_EXT
TEST_P(EGLChooseConfigDefaultTest, ColorComponentType)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that match the default value for EGL_COLOR_COMPONENT_TYPE_EXT, which is
    // EGL_COLOR_COMPONENT_TYPE_FIXED_EXT:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);

    // Choose configs with the explicit value of EGL_COLOR_COMPONENT_TYPE_FIXED_EXT for
    // EGL_COLOR_COMPONENT_TYPE_EXT:
    const EGLint explicitConfigAttributes[] = {EGL_COLOR_COMPONENT_TYPE_EXT,
                                               EGL_COLOR_COMPONENT_TYPE_FIXED_EXT, EGL_NONE};
    EGLint explicitConfigCount;
    std::vector<EGLConfig> explicitConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, explicitConfigAttributes, explicitConfigs.data(),
                                    explicitConfigs.size(), &explicitConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(defaultConfigCount, explicitConfigCount);

    // Choose configs with the explicit value of EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT for
    // EGL_COLOR_COMPONENT_TYPE_EXT:
    const EGLint alternateConfigAttributes[] = {EGL_COLOR_COMPONENT_TYPE_EXT,
                                                EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT, EGL_NONE};
    EGLint alternateConfigCount;
    std::vector<EGLConfig> alternateConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, alternateConfigAttributes, alternateConfigs.data(),
                                    alternateConfigs.size(), &alternateConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(alternateConfigCount, allConfigCount);
    // Because of other attributes, explicitConfigCount + alternateConfigCount
    // may still not add up to allConfigCount:
    ASSERT_LE(explicitConfigCount + alternateConfigCount, allConfigCount);
}

}  // namespace angle

ANGLE_INSTANTIATE_TEST(EGLChooseConfigDefaultTest, ES2_D3D11(), ES2_OPENGL(), ES2_VULKAN());
