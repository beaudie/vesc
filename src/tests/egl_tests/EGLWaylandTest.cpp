//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLWaylandTest.cpp: tests for EGL_EXT_platform_wayland

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-client.h>
#include <wayland-egl-backend.h>

#include "test_utils/ANGLETest.h"
#include "util/linux/wayland/WaylandWindow.h"

using namespace angle;

namespace
{
const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
}

class EGLWaylandTest : public ANGLETest
{
  public:
    std::vector<EGLint> getDisplayAttributes() const
    {
        std::vector<EGLint> attribs;

        attribs.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        attribs.push_back(GetParam().getRenderer());
        attribs.push_back(EGL_NONE);

        return attribs;
    }
};

// Test that a Wayland window can be created and used for rendering
TEST_P(EGLWaylandTest, WaylandWindowRendering)
{
    OSWindow *osWindow = WaylandWindow::New();
    ASSERT_TRUE(osWindow->initialize("EGLWaylandTest", 500, 500));
    setWindowVisible(osWindow, true);

    EGLNativeDisplayType waylandDisplay = osWindow->getNativeDisplay();
    EGLNativeWindowType waylandWindow   = osWindow->getNativeWindow();

    std::vector<EGLint> attribs = getDisplayAttributes();

    EGLDisplay display =
        eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, (void *)waylandDisplay, attribs.data());
    ASSERT_NE(EGL_NO_DISPLAY, display);

    ASSERT_TRUE(EGL_TRUE == eglInitialize(display, nullptr, nullptr));

    int nConfigs = 0;
    ASSERT_TRUE(EGL_TRUE == eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_GE(nConfigs, 1);

    int nReturnedConfigs = 0;
    std::vector<EGLConfig> configs(nConfigs);
    ASSERT_TRUE(EGL_TRUE == eglGetConfigs(display, configs.data(), nConfigs, &nReturnedConfigs));
    ASSERT_EQ(nConfigs, nReturnedConfigs);

    for (EGLConfig config : configs)
    {
        // Finally, try to do a clear on the window.
        EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, context);

        EGLSurface window = eglCreateWindowSurface(display, config, waylandWindow, nullptr);
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

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }

    OSWindow::Delete(&osWindow);

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(display);
}

TEST_P(EGLWaylandTest, SwapBuffers)
{
    OSWindow *osWindow = WaylandWindow::New();
    ASSERT_TRUE(osWindow->initialize("EGLWaylandTest", 500, 500));
    setWindowVisible(osWindow, true);

    EGLNativeDisplayType waylandDisplay = osWindow->getNativeDisplay();
    EGLNativeWindowType waylandWindow   = osWindow->getNativeWindow();

    std::vector<EGLint> attribs = getDisplayAttributes();

    EGLDisplay display =
        eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, (void *)waylandDisplay, attribs.data());
    ASSERT_NE(EGL_NO_DISPLAY, display);

    ASSERT_TRUE(EGL_TRUE == eglInitialize(display, nullptr, nullptr));

    int nConfigs = 0;
    ASSERT_TRUE(EGL_TRUE == eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_GE(nConfigs, 1);

    int nReturnedConfigs = 0;
    std::vector<EGLConfig> configs(nConfigs);
    ASSERT_TRUE(EGL_TRUE == eglGetConfigs(display, configs.data(), nConfigs, &nReturnedConfigs));
    ASSERT_EQ(nConfigs, nReturnedConfigs);

    for (EGLConfig config : configs)
    {
        EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, context);

        EGLSurface surface = eglCreateWindowSurface(display, config, waylandWindow, nullptr);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(display, surface, surface, context);
        ASSERT_EGL_SUCCESS();

        const uint32_t loopcount = 16;
        for (uint32_t i = 0; i < loopcount; i++)
        {
            osWindow->messageLoop();

            glViewport(0, 0, 500, 500);
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ASSERT_GL_NO_ERROR() << "glClear failed";
            EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);

            eglSwapBuffers(display, surface);
            ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
        }

        // Teardown
        eglDestroySurface(display, surface);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(display, context);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }

    OSWindow::Delete(&osWindow);

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(display);
}

ANGLE_INSTANTIATE_TEST(EGLWaylandTest, WithNoFixture(ES2_VULKAN()));
