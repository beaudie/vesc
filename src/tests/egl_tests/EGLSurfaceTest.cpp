//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSurfaceTest:
//   Tests pertaining to egl::Surface.
//

#include <gtest/gtest.h>

#include <vector>

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"

namespace
{

template <EGLint r, EGLint g, EGLint b, EGLint a, EGLint d, EGLint s, bool matchConfig>
class EGLSurfaceTest : public testing::Test
{
  protected:
    EGLSurfaceTest()
        : mDisplay(EGL_NO_DISPLAY),
          mWindowSurface(EGL_NO_SURFACE),
          mPbufferSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mSecondContext(EGL_NO_CONTEXT),
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
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mWindowSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mWindowSurface);
                mWindowSurface = EGL_NO_SURFACE;
            }

            if (mPbufferSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mPbufferSurface);
                mPbufferSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            if (mSecondContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mSecondContext);
                mSecondContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        SafeDelete(mOSWindow);

        ASSERT_TRUE(mWindowSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void initializeSurface(EGLenum platformType)
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

        std::vector<EGLint> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platformType);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);

        if (platformType == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE || platformType == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
        }
        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, mOSWindow->getNativeDisplay(), displayAttributes.data());
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        EGLint majorVersion, minorVersion;
        ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);

        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        EGLint configCount = 0;
        const EGLint configAttributes[] =
        {
            EGL_RED_SIZE, r,
            EGL_GREEN_SIZE, g,
            EGL_BLUE_SIZE, b,
            EGL_ALPHA_SIZE, a,
            EGL_DEPTH_SIZE, d,
            EGL_STENCIL_SIZE, s,
            EGL_SAMPLE_BUFFERS, 0,
            EGL_NONE
        };

        if (matchConfig)
        {
            if (EGLWindow::ChooseExactConfig(mDisplay, configAttributes, &mConfig, 1, &configCount) == EGL_TRUE && (configCount == 1))
            {
                std::cout << "Exact matching config for RED=" << r << " BLUE=" << b << " GREEN=" << g << " ALPHA=" << a << " is being used for surface creation" << std::endl;
            }
            else
            {
                std::cout << "Exact matching config for RED=" << r << " BLUE=" << b << " GREEN=" << g << " ALPHA=" << a << " was not found and is assumed to be not supported. Defaulting to eglChooseConfig behavior with same attributes." << std::endl;
                ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount) || (configCount != 1) == EGL_TRUE);
            }
        }
        else
        {
            ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount) || (configCount != 1) == EGL_TRUE);
        }

        std::vector<EGLint> surfaceAttributes;
        surfaceAttributes.push_back(EGL_NONE);
        surfaceAttributes.push_back(EGL_NONE);

        // Create first window surface
        mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), &surfaceAttributes[0]);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        mPbufferSurface = eglCreatePbufferSurface(mDisplay, mConfig, &surfaceAttributes[0]);

        EGLint contextAttibutes[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

        mContext = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        mSecondContext = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);
    }

    void runMessageLoopTest(EGLSurface secondSurface, EGLContext secondContext)
    {
        eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        // Make a second context current
        eglMakeCurrent(mDisplay, secondSurface, secondSurface, secondContext);
        eglDestroySurface(mDisplay, mWindowSurface);

        // Create second window surface
        std::vector<EGLint> surfaceAttributes;
        surfaceAttributes.push_back(EGL_NONE);
        surfaceAttributes.push_back(EGL_NONE);

        mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), &surfaceAttributes[0]);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
        ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

        mOSWindow->signalTestEvent();
        mOSWindow->messageLoop();
        ASSERT_TRUE(mOSWindow->didTestEventFire());

        // Simple operation to test the FBO is set appropriately
        glClear(GL_COLOR_BUFFER_BIT);
    }

    EGLDisplay mDisplay;
    EGLSurface mWindowSurface;
    EGLSurface mPbufferSurface;
    EGLContext mContext;
    EGLContext mSecondContext;
    EGLConfig mConfig;
    GLuint m2DProgram;
    GLint mTexture2DUniformLocation;
    OSWindow *mOSWindow;
};

typedef EGLSurfaceTest<EGL_DONT_CARE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_DONT_CARE, false> EGLSurfaceTestDefault;

// Test a surface bug where we could have two Window surfaces active
// at one time, blocking message loops. See http://crbug.com/475085
TEST_F(EGLSurfaceTestDefault, MessageLoopBug)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    runMessageLoopTest(EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

// Tests the message loop bug, but with setting a second context
// instead of null.
TEST_F(EGLSurfaceTestDefault, MessageLoopBugContext)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    runMessageLoopTest(mPbufferSurface, mSecondContext);
}

// Test a bug where calling makeCurrent twice would release the surface
TEST_F(EGLSurfaceTestDefault, MakeCurrentTwice)
{
    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_TRUE(eglGetError() == EGL_SUCCESS);

    // Simple operation to test the FBO is set appropriately
    glClear(GL_COLOR_BUFFER_BIT);
}

// Test that the D3D window surface is correctly resized after calling swapBuffers
TEST_F(EGLSurfaceTestDefault, ResizeD3DWindow)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    EGLint height;
    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(64, height);  // initial size

    // set window's height to 0
    mOSWindow->resize(64, 0);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(0, height);

    // restore window's height
    mOSWindow->resize(64, 64);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(64, height);
}

typedef EGLSurfaceTest<5, 6, 5, 0, 0, 0, true> EGLSurfaceTest_GL_RGB565;
TEST_F(EGLSurfaceTest_GL_RGB565, CreateSurface)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();
}

typedef EGLSurfaceTest<4, 4, 4, 4, 0, 0,true> EGLSurfaceTest_GL_RGBA4;
TEST_F(EGLSurfaceTest_GL_RGBA4, CreateSurface)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();
}

typedef EGLSurfaceTest<5, 5, 5, 1, 0, 0, true> EGLSurfaceTest_GL_RGB5_A1;
TEST_F(EGLSurfaceTest_GL_RGB5_A1, CreateSurface)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_ANGLE_platform_angle_d3d") == nullptr)
    {
        std::cout << "D3D Platform not supported in ANGLE" << std::endl;
        return;
    }

    initializeSurface(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();
}

}
