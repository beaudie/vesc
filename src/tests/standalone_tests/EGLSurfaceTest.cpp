//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSurfaceTest:
//   Tests pertaining to egl::Surface.
//

#include <gtest/gtest.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "common/angleutils.h"
#include "OSWindow.h"

namespace
{

class EGLSurfaceTest : public testing::Test
{
  protected:
    EGLSurfaceTest()
        : mDisplay(EGL_NO_DISPLAY),
          mSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mOSWindow(nullptr)
    {
    }

    void SetUp() override
    {
        mOSWindow = CreateOSWindow();
        mOSWindow->initialize("EGLSurfaceTest", 64, 64);
    }

    // Release any resources created in the test body
    void TearDown() override
    {
        mOSWindow->destroy();
        SafeDelete(mOSWindow);

        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mSurface);
                mSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        ASSERT_TRUE(mSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    OSWindow *mOSWindow;
};

// Test a surface bug where we could have two Window surfaces active
// at one time, blocking message loops. See http://crbug.com/475085
TEST_F(EGLSurfaceTest, MessageLoopBug)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE";
        return;
    }

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

    const EGLint displayAttributes[] =
    {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_NONE,
    };

    mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, mOSWindow->getNativeDisplay(), displayAttributes);
    ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

    EGLint majorVersion, minorVersion;
    ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);

    eglBindAPI(EGL_OPENGL_ES_API);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    const EGLint configAttributes[] =
    {
        EGL_RED_SIZE, EGL_DONT_CARE,
        EGL_GREEN_SIZE, EGL_DONT_CARE,
        EGL_BLUE_SIZE, EGL_DONT_CARE,
        EGL_ALPHA_SIZE, EGL_DONT_CARE,
        EGL_DEPTH_SIZE, EGL_DONT_CARE,
        EGL_STENCIL_SIZE, EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLint configCount;
    EGLConfig config;
    ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &config, 1, &configCount) || (configCount != 1) == EGL_TRUE);

    std::vector<EGLint> surfaceAttributes;
    surfaceAttributes.push_back(EGL_NONE);
    surfaceAttributes.push_back(EGL_NONE);

    // Create first window surface
    mSurface = eglCreateWindowSurface(mDisplay, config, mOSWindow->getNativeWindow(), &surfaceAttributes[0]);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    EGLint contextAttibutes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    mContext = eglCreateContext(mDisplay, config, nullptr, contextAttibutes);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(mDisplay, mSurface);

    // Create second window surface
    mSurface = eglCreateWindowSurface(mDisplay, config, mOSWindow->getNativeWindow(), &surfaceAttributes[0]);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    mOSWindow->signalTestEvent();
    mOSWindow->messageLoop();
    ASSERT_TRUE(mOSWindow->didTestEventFire());

    // Simple operation to test the FBO is set appropriately
    glClear(GL_COLOR_BUFFER_BIT);
}

}
