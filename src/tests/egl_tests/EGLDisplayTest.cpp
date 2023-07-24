//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>
#include <vector>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLDisplayTest : public ANGLETest<>
{};

// Tests that an EGLDisplay can be re-initialized.
TEST_P(EGLDisplayTest, InitializeTerminateInitialize)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);
    EXPECT_EGL_TRUE(eglTerminate(display) != EGL_FALSE);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);
}

// Tests current Context leaking when call eglTerminate() while it is current.
TEST_P(EGLDisplayTest, ContextLeakAfterTerminate)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);

    const EGLint configAttribs[] = {EGL_RED_SIZE,
                                    8,
                                    EGL_GREEN_SIZE,
                                    8,
                                    EGL_BLUE_SIZE,
                                    8,
                                    EGL_ALPHA_SIZE,
                                    8,
                                    EGL_RENDERABLE_TYPE,
                                    EGL_OPENGL_ES2_BIT,
                                    EGL_SURFACE_TYPE,
                                    EGL_PBUFFER_BIT,
                                    EGL_NONE};
    EGLConfig config             = EGL_NO_CONFIG_KHR;
    EGLint count                 = 0;
    EXPECT_EGL_TRUE(eglChooseConfig(display, configAttribs, &config, 1, &count));
    EXPECT_EGL_TRUE(count > 0);

    const EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE};
    EGLContext context            = eglCreateContext(display, config, nullptr, contextAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    const EGLint surfaceAttribs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 64, EGL_NONE};
    EGLSurface surface            = eglCreatePbufferSurface(display, config, surfaceAttribs);
    EXPECT_NE(surface, EGL_NO_SURFACE);

    // Make "context" current.
    EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context));

    // Terminate display while "context" is current.
    EXPECT_EGL_TRUE(eglTerminate(display) != EGL_FALSE);

    // Unmake "context" from current and allow Display to actually terminate.
    (void)eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    // Get EGLDisplay again.
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // Check if Display was actually terminated.
    EGLint val;
    EXPECT_EGL_FALSE(eglQueryContext(display, context, EGL_CONTEXT_CLIENT_TYPE, &val));
    EXPECT_EQ(eglGetError(), EGL_NOT_INITIALIZED);
}

ANGLE_INSTANTIATE_TEST(EGLDisplayTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
