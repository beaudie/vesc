//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLContextNoErrorTest:
//   Tests pertaining to EGL_KHR_create_context_no_error and GL_KHR_no_error
//

#include <gtest/gtest.h>

#include <vector>

#include "common/platform.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

using namespace angle;

class EGLCreateContextNoErrorTest : public ANGLETest
{
  protected:
    EGLCreateContextNoErrorTest()
        : mDisplay(EGL_NO_DISPLAY), mContext(EGL_NO_CONTEXT), mNaughtyTexture(0)
    {}

    void testSetUp() override
    {
        EGLint displayAttributes[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                                      EGL_NONE};
        mDisplay                   = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY),
                                            displayAttributes);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr) != EGL_FALSE);
    }

    void testTearDown() override
    {
        if (mNaughtyTexture != 0)
        {
            glDeleteTextures(1, &mNaughtyTexture);
        }
        eglDestroyContext(mDisplay, mContext);
        eglMakeCurrent(mDisplay, nullptr, nullptr, nullptr);
        eglTerminate(mDisplay);
    }

    void initializeContext(EGLint *contextAttributes)
    {
        EGLint count = 0;
        EGLConfig config;
        EGLint cfgAttribList[] = {EGL_RENDERABLE_TYPE, (EGL_OPENGL_ES2_BIT), EGL_NONE};
        ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, cfgAttribList, &config, 1, &count));
        ASSERT_TRUE(count > 0);

        mContext = eglCreateContext(mDisplay, config, nullptr, contextAttributes);
        EXPECT_TRUE(eglGetError() == EGL_SUCCESS);

        EGLint pbufferAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE};
        EGLSurface surface         = eglCreatePbufferSurface(mDisplay, config, pbufferAttributes);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, surface, surface, mContext);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);
    }

    void bindNaughtyTexture()
    {
        glGenTextures(1, &mNaughtyTexture);
        ASSERT_GL_NO_ERROR();
        glBindTexture(GL_TEXTURE_CUBE_MAP, mNaughtyTexture);
        ASSERT_GL_NO_ERROR();

        // mNaughtyTexture should now be a GL_TEXTURE_CUBE_MAP texture, so rebinding it to
        // GL_TEXTURE_2D is an error
        glBindTexture(GL_TEXTURE_2D, mNaughtyTexture);
    }

    EGLDisplay mDisplay;
    EGLContext mContext;
    GLuint mNaughtyTexture;
};

// Tests that error reporting is enabled under normal conditions
TEST_P(EGLCreateContextNoErrorTest, YesError)
{
    EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, GetParam().majorVersion, EGL_NONE};
    initializeContext(contextAttributes);
    ASSERT_GL_NO_ERROR();

    bindNaughtyTexture();
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests that error reporting is suppressed when GL_KHR_no_error is enabled
TEST_P(EGLCreateContextNoErrorTest, NoError)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context_no_error"));

    EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, GetParam().majorVersion,
                                  EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE, EGL_NONE};
    initializeContext(contextAttributes);
    ASSERT_GL_NO_ERROR();

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_no_error"));

    bindNaughtyTexture();
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(EGLCreateContextNoErrorTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()),
                       WithNoFixture(ES2_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()));
