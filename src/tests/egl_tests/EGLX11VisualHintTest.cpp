//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLX11VisualHintTest.cpp: tests for EGL_ANGLE_x11_visual_hint

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib.h>

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
}

class EGLX11VisualHintTest : public ANGLETest
{
  public:
    void SetUp() override
    {
        mEglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
    }

    std::vector<EGLint> getDisplayAttributes(int visualId) const
    {
        std::vector<EGLint> attribs;

        attribs.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        attribs.push_back(GetParam().getRenderer());
        attribs.push_back(EGL_ANGLE_X11_VISUAL_ID_HINT);
        attribs.push_back(visualId);
        attribs.push_back(EGL_NONE);

        return attribs;
    }

  protected:
    PFNEGLGETPLATFORMDISPLAYEXTPROC mEglGetPlatformDisplayEXT;
};

TEST_P(EGLX11VisualHintTest, InvalidVisualID)
{
    static const int gInvalidVisualId = -1;
    auto attributes                   = getDisplayAttributes(gInvalidVisualId);

    EGLDisplay display =
        mEglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, attributes.data());
    ASSERT_TRUE(display != EGL_NO_DISPLAY);

    ASSERT_EQ(EGL_FALSE, eglInitialize(display, nullptr, nullptr));
    ASSERT_EGL_ERROR(EGL_NOT_INITIALIZED);
}

TEST_P(EGLX11VisualHintTest, ValidVisualIDAndClear)
{
    // We'll test the extension with one visual ID but we don't care which one. This means we
    // can use OSWindow to create a window and just grab its visual.
    OSWindow *osWindow = CreateOSWindow();
    osWindow->initialize("EGLX11VisualHintTest", 500, 500);
    osWindow->setVisible(true);

    Display *xDisplay = osWindow->getNativeDisplay();
    Window xWindow    = osWindow->getNativeWindow();

    XWindowAttributes windowAttributes;
    ASSERT_TRUE(XGetWindowAttributes(xDisplay, xWindow, &windowAttributes) != 0);
    int visualId = windowAttributes.visual->visualid;

    auto attributes = getDisplayAttributes(visualId);
    EGLDisplay display =
        mEglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, attributes.data());
    ASSERT_TRUE(display != EGL_NO_DISPLAY);

    ASSERT_TRUE(eglInitialize(display, nullptr, nullptr) == EGL_TRUE);

    // While this is not required by the extension, test that our implementation returns only one
    // config, with the same native visual Id that we provided.
    int nConfigs = 0;
    ASSERT_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs) == EGL_TRUE);
    ASSERT_TRUE(nConfigs == 1);

    int nReturnedConfigs = 0;
    EGLConfig config;
    ASSERT_TRUE(eglGetConfigs(display, &config, 1, &nReturnedConfigs) == EGL_TRUE);
    ASSERT_TRUE(nConfigs == nReturnedConfigs);

    EGLint eglNativeId;
    ASSERT_TRUE(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &eglNativeId));
    ASSERT_EQ(visualId, eglNativeId);

    // Finally, try to do a clear on the window.
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    ASSERT_TRUE(context != EGL_NO_CONTEXT);

    EGLSurface window =
        eglCreateWindowSurface(display, config, osWindow->getNativeWindow(), nullptr);
    ASSERT_EGL_SUCCESS();

    eglMakeCurrent(display, window, window, context);
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, 500, 500);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);

    // Teardown
    eglDestroySurface(display, window);
    ASSERT_EGL_SUCCESS();

    eglDestroyContext(display, context);
    ASSERT_EGL_SUCCESS();

    SafeDelete(osWindow);

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(display);
}

ANGLE_INSTANTIATE_TEST(EGLX11VisualHintTest, ES2_OPENGL());
