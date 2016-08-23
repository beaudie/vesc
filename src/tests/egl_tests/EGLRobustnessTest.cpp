//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLRobustnessTest.cpp: tests for EGL_EXT_create_context_robustness

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLRobustnessTest : public ::testing::TestWithParam<angle::PlatformParameters>
{
  public:
    void SetUp() override
    {
        mOSWindow = CreateOSWindow();
        mOSWindow->initialize("EGLRobustnessTest", 500, 500);
        mOSWindow->setVisible(true);

        auto eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));

        const auto& platform = GetParam().eglParameters;

        std::vector<EGLint> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platform.renderer);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(platform.majorVersion);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(platform.minorVersion);

        if (platform.deviceType != EGL_DONT_CARE)
        {
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
            displayAttributes.push_back(platform.deviceType);
        }

        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                            &displayAttributes[0]);
        ASSERT_NE(EGL_NO_DISPLAY, mDisplay);

        ASSERT_EQ(EGL_TRUE, eglInitialize(mDisplay, nullptr, nullptr));

        const char* extensions = eglQueryString(mDisplay, EGL_EXTENSIONS);
        if (strstr(extensions, "EGL_EXT_create_context_robustness") == nullptr)
        {
            std::cout << "Test skipped due to missing EGL_EXT_create_context_robustness" << std::endl;
            return;
        }

        int nConfigs = 0;
        ASSERT_EQ(EGL_TRUE, eglGetConfigs(mDisplay, nullptr, 0, &nConfigs));
        ASSERT_LE(1, nConfigs);

        int nReturnedConfigs = 0;
        ASSERT_EQ(EGL_TRUE, eglGetConfigs(mDisplay, &mConfig, 1, &nReturnedConfigs));
        ASSERT_EQ(1, nReturnedConfigs);

        mWindow = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        mInitialized = true;
    }

    void TearDown() override
    {
        eglDestroySurface(mDisplay, mWindow);
        eglDestroyContext(mDisplay, mContext);
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        EXPECT_EGL_SUCCESS();

        SafeDelete(mOSWindow);
    }

    void createContext(EGLint resetStrategy)
    {
        const EGLint contextAttribs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT, resetStrategy,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, mContext);

        eglMakeCurrent(mDisplay, mWindow, mWindow, mContext);
        ASSERT_EGL_SUCCESS();
    }

    void forceContextReset()
    {
        GLuint program = CompileProgram(
            "attribute vec4 pos;\n"
            "void main() {gl_Position = pos;}\n",
            "precision mediump float;\n"
            "uniform float target;"
            "void main() {"
            "   float i = 0.0;"
            "   float j = 0.0;"
            "   while (i <= target) {\n"
            "       while(j <= i) j++;\n"
            "       i++; j = 0.0;\n"
            "   }\n"
            "   gl_FragColor = vec4(i);\n"
            "}\n"
        );
        ASSERT_NE(0, program);
        glUseProgram(program);
        glUniform1f(glGetUniformLocation(program, "target"), 1000000000.0f);

        GLfloat vertices[] =
        {
            -1.0f, -1.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f,
        };

        glBindAttribLocation(program, 0, "pos");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        glViewport(0, 0, mOSWindow->getWidth(), mOSWindow->getHeight());
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glFinish();
    }

  protected:
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mWindow = EGL_NO_SURFACE;
    bool mInitialized = false;

  private:
    EGLContext mContext = EGL_NO_CONTEXT;
    EGLConfig mConfig = 0;
    OSWindow *mOSWindow = nullptr;
};

// Check glGetGraphicsResetStatusEXT returns GL_NO_ERROR if we did nothing
TEST_P(EGLRobustnessTest, NoErrorByDefault)
{
    if (!mInitialized)
    {
        return;
    }
    ASSERT_EQ(GL_NO_ERROR, glGetGraphicsResetStatusEXT());
}

// Checks that the application gets notified of a context loss with LOSE_CONTEXT_ON_RESET
TEST_P(EGLRobustnessTest, LoseContextOnReset)
{
    if (!mInitialized)
    {
        return;
    }

    if (!IsWindows())
    {
        std::cout << "Test disabled on non Windows platforms because drivers can't recover. "
                  << "See " << __FILE__ << ":" << __LINE__ << std::endl;
        return;
    }

    createContext(EGL_LOSE_CONTEXT_ON_RESET_EXT);
    forceContextReset();
    ASSERT_NE(GL_NO_ERROR, glGetGraphicsResetStatusEXT());
}

// Checks that the application gets no loss with NO_RESET_NOTIFICATION
TEST_P(EGLRobustnessTest, NoResetNotification)
{
    if (!mInitialized)
    {
        return;
    }

    if (!IsWindows())
    {
        std::cout << "Test disabled on non Windows platforms because drivers can't recover. "
                  << "See " << __FILE__ << ":" << __LINE__ << std::endl;
        return;
    }

    createContext(EGL_NO_RESET_NOTIFICATION_EXT);
    forceContextReset();
    ASSERT_EQ(GL_NO_ERROR, glGetGraphicsResetStatusEXT());
}

// Checks that resetting the ANGLE display allows to get rid of the context loss.
TEST_P(EGLRobustnessTest, ResettingDisplayWorks)
{
    if (!mInitialized)
    {
        return;
    }

    if (!IsWindows())
    {
        std::cout << "Test disabled on non Windows platforms because drivers can't recover. "
                  << "See " << __FILE__ << ":" << __LINE__ << std::endl;
        return;
    }

    createContext(EGL_LOSE_CONTEXT_ON_RESET_EXT);
    forceContextReset();
    ASSERT_NE(GL_NO_ERROR, glGetGraphicsResetStatusEXT());

    TearDown();
    SetUp();
    ASSERT_EQ(GL_NO_ERROR, glGetGraphicsResetStatusEXT());
}

ANGLE_INSTANTIATE_TEST(EGLRobustnessTest, ES2_OPENGL(), ES2_D3D9(), ES2_D3D11());
