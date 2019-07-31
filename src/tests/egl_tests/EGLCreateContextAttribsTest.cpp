//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

using namespace angle;

//
// This suite of test cases is to test valid/invalide attributes passed to eglCreateContext
// Section 3.7.1 of EGL 1.5 specification provides error cases
//
class EGLCreateContextAttribsTest : public ANGLETest
{
  public:
    EGLCreateContextAttribsTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) != EGL_FALSE);
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglTerminate(mDisplay);
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    EGLDisplay mDisplay;
};

// Specify invalid client version in the attributes to eglCreateContext
// and verify EGL_BAD_ATTRIBUTE
TEST_P(EGLCreateContextAttribsTest, InvalidClientVersion)
{
    EGLContext context = EGL_NO_CONTEXT;

    // Pick config
    EGLConfig config        = EGL_NO_CONFIG_KHR;
    EGLint count            = 0;
    EGLint cfgAttribList1[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT, EGL_NONE};
    EXPECT_TRUE(eglChooseConfig(mDisplay, cfgAttribList1, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(count == 0);

    // Setup invalid set of attributes
    EGLint contextAttribs1[] = {EGL_CONTEXT_MAJOR_VERSION, 0, EGL_CONTEXT_MINOR_VERSION, 0,
                                EGL_NONE};
    // Check eglCreateContext to catch fail with errors
    context = eglCreateContext(mDisplay, config, nullptr, contextAttribs1);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    EGLint cfgAttribList2[] = {EGL_RENDERABLE_TYPE, (EGL_OPENGL_ES2_BIT), EGL_NONE};
    EXPECT_TRUE(eglChooseConfig(mDisplay, cfgAttribList2, &config, 1, &count));
    ASSERT_TRUE(count > 0);

    EGLint contextAttribs2[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 1,
                                EGL_NONE};
    // Check eglCreateContext to catch fail with errors
    context = eglCreateContext(mDisplay, config, nullptr, contextAttribs2);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // Check eglCreateContext to catch fail with errors
    context = eglCreateContext(mDisplay, config, nullptr, contextAttribs2);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // Cleanup
    eglTerminate(mDisplay);
}

// Choose config that doesn't support client version, and verify that eglCreateContext
// sets EGL_BAD_MATCH
TEST_P(EGLCreateContextAttribsTest, IncompatibleConfig)
{
    // Get all the configs
    EGLint ncfg;
    EGLConfig *configList = nullptr;
    EXPECT_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &ncfg) != EGL_FALSE);
    EXPECT_TRUE(ncfg > 0);
    configList = new EGLConfig[ncfg];
    EXPECT_TRUE(configList != nullptr);
    EXPECT_TRUE(eglGetConfigs(mDisplay, configList, ncfg, &ncfg) != EGL_FALSE);

    EGLConfig notGLES1Config = EGL_NO_CONFIG_KHR;
    EGLConfig notGLES2Config = EGL_NO_CONFIG_KHR;
    EGLConfig notGLES3Config = EGL_NO_CONFIG_KHR;
    for (EGLint i = 0; i < ncfg; i++)
    {
        EGLConfig config = configList[i];
        EGLint value     = 0;
        EXPECT_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_RENDERABLE_TYPE, &value));

        if (((value & EGL_OPENGL_ES_BIT) == 0) && (notGLES1Config == EGL_NO_CONFIG_KHR))
        {
            notGLES1Config = config;
            continue;
        }
        if (((value & EGL_OPENGL_ES2_BIT) == 0) && (notGLES2Config == EGL_NO_CONFIG_KHR))
        {
            notGLES2Config = config;
            continue;
        }
        if (((value & EGL_OPENGL_ES3_BIT) == 0) && (notGLES3Config == EGL_NO_CONFIG_KHR))
        {
            notGLES3Config = config;
            continue;
        }
    }

    EGLContext context = EGL_NO_CONTEXT;
    // Check GLES1
    if (notGLES1Config != EGL_NO_CONFIG_KHR)
    {
        // Setup GLES 1.0 attributes
        EGLint contextAttribs1[] = {EGL_CONTEXT_MAJOR_VERSION, 1, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        // Check eglCreateContext to catch fail with errors
        context = eglCreateContext(mDisplay, notGLES1Config, nullptr, contextAttribs1);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Check GLES2
    if (notGLES2Config != EGL_NO_CONFIG_KHR)
    {
        // Setup GLES 2.0 attributes
        EGLint contextAttribs2[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        // Check eglCreateContext to catch fail with errors
        context = eglCreateContext(mDisplay, notGLES2Config, nullptr, contextAttribs2);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Check GLES3
    if (notGLES3Config != EGL_NO_CONFIG_KHR)
    {
        // Setup GLES 3.0 attributes
        EGLint contextAttribs3[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        // Check eglCreateContext to catch fail with errors
        context = eglCreateContext(mDisplay, notGLES3Config, nullptr, contextAttribs3);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Cleanup
    eglTerminate(mDisplay);
    delete[] configList;
    configList = nullptr;
}

ANGLE_INSTANTIATE_TEST(EGLCreateContextAttribsTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
