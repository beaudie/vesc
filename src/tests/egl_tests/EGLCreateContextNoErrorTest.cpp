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
        : mDisplay(EGL_NO_DISPLAY),
          mContext(EGL_NO_CONTEXT),
          mNaughtyTexture(0),
          mEGLInitialized(EGL_FALSE)
    {
        setNoErrorEnabled(true);
    }

    void testSetUp() override
    {
        EGLint displayAttributes[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                                      EGL_NONE};
        mDisplay                   = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY),
                                            displayAttributes);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
        mEGLInitialized = eglInitialize(mDisplay, nullptr, nullptr);
    }

    void testTearDown() override
    {
        if (mNaughtyTexture != 0)
        {
            glDeleteTextures(1, &mNaughtyTexture);
        }
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
    EGLBoolean mEGLInitialized;
};

// Tests that error reporting is suppressed when GL_KHR_no_error is enabled
TEST_P(EGLCreateContextNoErrorTest, NoError)
{
    // anglebug.com/4441
    ANGLE_SKIP_TEST_IF(IsOSX() && isSwiftshader());
    ASSERT_EGL_TRUE(mEGLInitialized);

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_create_context_no_error"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_no_error"));

    bindNaughtyTexture();
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(EGLCreateContextNoErrorTest);
