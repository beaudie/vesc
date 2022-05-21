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
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace angle
{
class EGLChooseConfigTest : public ANGLETest
{
  protected:
    EGLChooseConfigTest() {}
};

// Test that the EGL_COLOR_BUFFER_TYPE is defaulted to EGL_RGB_BUFFER
TEST_P(EGLChooseConfigTest, Defaults)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that have the default attribute values:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);
    defaultConfigs.resize(defaultConfigCount);

    // Check that the default configs all have the default attribute values we care about:
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

        if (IsEGLDisplayExtensionEnabled(display, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(display, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
            ASSERT_EQ(colorComponentType, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT);
        }
    }

    // Check that all of the configs that have the default attribute values are are defaultConfigs,
    // and all that don't aren't:
    for (EGLConfig config : allConfigs)
    {
        // Some debug code
        EGLint redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, numSamples = 0;
        eglGetConfigAttrib(display, config, EGL_RED_SIZE, &redBits);
        eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &greenBits);
        eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blueBits);
        eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alphaBits);
        eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depthBits);
        eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &stencilBits);
        eglGetConfigAttrib(display, config, EGL_SAMPLES, &numSamples);

        std::printf(
            "redBits: %d; greenBits: %d; blueBits: %d; alphaBits: %d; "
            "depthBits: %d; stencilBits: %d; numSamples: %d\n",
            redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, numSamples);

        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

        eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        eglGetConfigAttrib(display, config, EGL_LEVEL, &level);
        eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &renderableType);
        eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
        eglGetConfigAttrib(display, config, EGL_TRANSPARENT_TYPE, &transparentType);
        if (IsEGLDisplayExtensionEnabled(display, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(display, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
        }

        bool isADefault =
            ((colorBufferType == EGL_RGB_BUFFER) && (level == 0) &&
             ((renderableType & EGL_OPENGL_ES_BIT) == EGL_OPENGL_ES_BIT) &&
             ((surfaceType & EGL_WINDOW_BIT) == EGL_WINDOW_BIT) && (transparentType == EGL_NONE) &&
             (colorComponentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT));
        EGLint thisConfigID;
        eglGetConfigAttrib(display, config, EGL_CONFIG_ID, &thisConfigID);
        bool foundInDefaultConfigs = false;
        // Attempt to find this config ID in defaultConfigs:
        for (EGLConfig defaultConfig : defaultConfigs)
        {
            EGLint defaultConfigID;
            eglGetConfigAttrib(display, defaultConfig, EGL_CONFIG_ID, &defaultConfigID);
            if (defaultConfigID == thisConfigID)
            {
                foundInDefaultConfigs = true;
            }
        }
        ASSERT_EQ(isADefault, foundInDefaultConfigs);
    }
}

// Test to reproduce a deqp test failure on Pixel6:
// dEQP.EGL/functional_render_multi_context_gles2_gles3_rgb888_pbuffer
TEST_P(EGLChooseConfigTest, deqpBug)
{
    std::vector<EGLint> configAttributes = {
        EGL_RED_SIZE,   8,  EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_SAMPLES,   0, EGL_NONE};

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(display, configAttributes.data(), &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for the specific format is not supported, skipping test"
                  << std::endl;
        return;
    }

    EGLSurface surface;
    EGLint attribs[] = {EGL_WIDTH, getWindowWidth(), EGL_HEIGHT, getWindowHeight(), EGL_NONE};
    surface          = eglCreatePbufferSurface(display, config, attribs);
    ASSERT_EGL_SUCCESS();
    if (surface == EGL_NO_SURFACE)
    {
        std::cout
            << "EGL pbuffer surface cannot be created with this specific config, skipping test"
            << std::endl;
    }
    EGLContext context = eglCreateContext(display, config, nullptr, nullptr);
    ASSERT_EGL_SUCCESS();
    eglMakeCurrent(display, surface, surface, context);
    ASSERT_EGL_SUCCESS();

    const char kSimpleAttributeVS[] = R"(attribute vec4 a_position;
    attribute vec4 a_color;
    varying vec4 v_color;
    void main()
    {
      gl_Position = a_position;
      v_color = a_color;
    })";

    const char kSimpleAttributeFS[] = R"(precision mediump float;
      varying vec4 v_color;
      void main()
      {
        gl_FragColor = v_color;
      })";

    ANGLE_GL_PROGRAM(program, kSimpleAttributeVS, kSimpleAttributeFS);
    glUseProgram(program);
    GLint colorLoc    = glGetAttribLocation(program, "a_color");
    GLint positionLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, colorLoc);
    ASSERT_NE(-1, positionLoc);

    std::array<GLfloat, 3 * 4> positionData = {-1, -1, 0, 1, 0, 1, 0, 1, 1, -1, 0, 1};
    std::array<GLfloat, 3 * 4> colorData    = {1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1};

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionLoc);
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, size(colorData), colorData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(colorLoc);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glClearColor(0, 0, 0, 1);
    glClearDepthf(1.0f);
    glClearStencil(0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 0, ~0u);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glDisable(GL_DITHER);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

}  // namespace angle

ANGLE_INSTANTIATE_TEST(EGLChooseConfigTest,
                       ES2_D3D11(),
                       ES2_D3D9(),
                       ES2_METAL(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES2_VULKAN());
