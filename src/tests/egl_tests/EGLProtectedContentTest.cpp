//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLProtectedContentTest.cpp:
//   EGL extension EGL_EXT_protected_content
//

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>
#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

using namespace std::chrono_literals;

using namespace angle;

class EGLProtectedContentTest : public ANGLETest
{
  public:
    EGLProtectedContentTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
        mMajorVersion       = GetParam().majorVersion;
        mExtensionSupported = IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content");
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            eglReleaseThread();
            mDisplay = EGL_NO_DISPLAY;
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    bool chooseConfig(EGLConfig *config)
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0,
                            // EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
                            EGL_RENDERABLE_TYPE, clientVersion, EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_WINDOW_BIT), EGL_NONE};

        result = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result && (count > 0));
        return result;
    }

    bool createContext(EGLBoolean isProtected, EGLConfig config, EGLContext *context)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion,
                                     EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};

        *context = eglCreateContext(mDisplay, config, nullptr,
                                    (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*context != EGL_NO_CONTEXT);
        EXPECT_TRUE(result);
        return result;
    }

    bool createPbufferSurface(EGLBoolean isProtected, EGLConfig config, EGLSurface *surface)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight,
                                     // EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
                                     EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight,
                                       // EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
                                       EGL_NONE};

        *surface = eglCreatePbufferSurface(mDisplay, config,
                                           (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createWindowSurface(EGLBoolean isProtected,
                             EGLConfig config,
                             EGLNativeWindowType win,
                             EGLSurface *surface)
    {
        bool result                 = false;
        EGLint attribsProtected[]   = {EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_NONE};

        *surface = eglCreateWindowSurface(mDisplay, config, win,
                                          (isProtected ? attribsProtected : attribsUnProtected));
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createImage(EGLBoolean isProtected,
                     EGLContext context,
                     EGLenum target,
                     EGLClientBuffer buffer,
                     EGLImage *image)
    {
        bool result                    = false;
        EGLAttrib attribsProtected[]   = {EGL_PROTECTED_CONTENT_EXT, EGL_TRUE, EGL_NONE};
        EGLAttrib attribsUnProtected[] = {EGL_NONE};

        *image = eglCreateImage(mDisplay, context, target, buffer,
                                (isProtected ? attribsProtected : attribsUnProtected));
        result = (*image != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }
#if 0  // TODO
    bool createTexture(EGLBoolean isProtected, EGLContext context, GLuint *textureId)
    {
        bool result = false;
        *textureId  = 0;

        glGenTextures(1, textureId);
        EXPECT_GL_NO_ERROR();
        result = (*textureId != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createTextureFromImage(EGLBoolean isProtected, EGLContext context, EGLImage image, GLuint *textureId)
    {
        bool result = false;
        *textureId  = 0;

        glGenTextures(1, textureId);
        EXPECT_GL_NO_ERROR();
        result = (*textureId != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createTextureFromPbuffer(EGLBoolean isProtected,
                                EGLContext context,
                                EGLSurface pBuffer,
                                GLuint *textureId)
    {
        bool result = false;
        *textureId  = 0;

        glGenTextures(1, textureId);
        EXPECT_GL_NO_ERROR();
        result = (*textureId != 0);
        EXPECT_TRUE(result);
        return result;
    }
#endif
    void PbufferTest(bool isProtectedContext, bool isProtectedSurface);
    void WindowTest(bool isProtectedContext, bool isProtectedSurface);
    bool RenderTexture(bool isProtectedContext, bool isProtectedTexture, GLuint textureId);
    bool TestTexture(bool isProtectedContext, bool isProtectedTexture, GLuint textureId);

    EGLDisplay mDisplay      = EGL_NO_DISPLAY;
    EGLint mMajorVersion     = 0;
    const EGLint kWidth      = 128;
    const EGLint kHeight     = 128;
    bool mExtensionSupported = false;
};

void EGLProtectedContentTest::PbufferTest(bool isProtectedContext, bool isProtectedSurface)
{
    if (isProtectedContext || isProtectedSurface)
    {
        ANGLE_SKIP_TEST_IF(!mExtensionSupported);
    }

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface pBufferSurface = EGL_NO_SURFACE;
    EXPECT_TRUE(createPbufferSurface(isProtectedSurface, config, &pBufferSurface));
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    // Results
    static const GLColor kTransparentBlack = angle::MakeGLColor(0.0, 0.0, 0.0, 0.0);
    if (isProtectedContext)
    {
        GLColor pixel;
        // Expect crash
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel.R);
        EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
    }
    else
    {
        if (isProtectedSurface)
        {
            GLColor pixel;
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel.R);
            EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
        }
        else
        {
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        }
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, pBufferSurface);
    pBufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with Unprotected PbufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedPbufferSurface)
{
    PbufferTest(false, false);
}

// Protected context with Unprotected PbufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedPbufferSurface)
{
    PbufferTest(true, false);
}

// Unprotected context with Protected PbufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedPbufferSurface)
{
    PbufferTest(false, true);
}

// Protected context with Protected PbufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedPbufferSurface)
{
    PbufferTest(true, true);
}

void EGLProtectedContentTest::WindowTest(bool isProtectedContext, bool isProtectedSurface)
{
    if (isProtectedContext || isProtectedSurface)
    {
        ANGLE_SKIP_TEST_IF(!mExtensionSupported);
    }

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface windowSurface = EGL_NO_SURFACE;
    OSWindow *osWindow       = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    // Red
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    glFinish();
    // Results
    static const GLColor kTransparentBlack = angle::MakeGLColor(0.0, 0.0, 0.0, 0.0);
    if (isProtectedContext)
    {
        GLColor pixel;
        // Expect crash
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel.R);
        EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
    }
    else
    {
        if (isProtectedSurface)
        {
            GLColor pixel;
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel.R);
            EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
        }
        else
        {
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        }
    }
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    std::this_thread::sleep_for(1s);

    // Green
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    std::this_thread::sleep_for(1s);

    // Blue
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    std::this_thread::sleep_for(1s);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with Unprotected WindowSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedWindowSurface)
{
    WindowTest(false, false);
}

// Protected context with Unprotected WindowSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedWindowSurface)
{
    WindowTest(true, false);
}

// Unprotected context with Protected WindowSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedWindowSurface)
{
    WindowTest(false, true);
}

// Protected context with Protected WindowSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedWindowSurface)
{
    WindowTest(true, true);
}

// Render pixels to the texture
bool EGLProtectedContentTest::RenderTexture(bool isProtectedContext,
                                            bool isProtectedTexture,
                                            GLuint textureId)
{
    return true;  // TODO
}

// Check Texture for expected pixels
bool EGLProtectedContentTest::TestTexture(bool isProtectedContext,
                                          bool isProtectedTexture,
                                          GLuint textureId)
{
    return true;  // TODO
}
#if 0
// Unprotected context with unprotected texture
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTexture)
{
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    GLuint texture = 0;
    EXPECT_TRUE(createTexture(false, context, &texture));

    EXPECT_TRUE(RenderTexture(false, false, texture));
    EXPECT_TRUE(TestTexture(false, false, texture));
}

// Protected context with unprotected texture
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTexture)
{
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(true, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    GLuint texture = 0;
    EXPECT_TRUE(createTexture(false, context, &texture));

    EXPECT_TRUE(RenderTexture(true, false, texture));
    EXPECT_TRUE(TestTexture(true, false, texture));
}

// Unprotected context with protected texture
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTexture)
{
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    GLuint texture = 0;
    EXPECT_TRUE(createTexture(true, context, &texture));

    EXPECT_TRUE(RenderTexture(false, true, texture));
    EXPECT_TRUE(TestTexture(false, true, texture));
}

// Protected context with protected texture
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTexture)
{
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(true, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    GLuint texture = 0;
    EXPECT_TRUE(createTexture(true, context, &texture));

    EXPECT_TRUE(RenderTexture(true, true, texture));
    EXPECT_TRUE(TestTexture(true, true, texture));
}
#endif
#if 0  // TODO
// Unprotected context with unprotected texture from EGL image
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromImage)
{
    RenderTexture(false, false, 1);
}

// Protected context with unprotected texture from EGL image
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromImage)
{
    RenderTexture(true, false, 1);
}

// Unprotected context with protected texture from EGL image
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromImage)
{
    RenderTexture(false, true, 1);
}

// Protected context with protected texture from EGL image
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromImage)
{
    RenderTexture(true, true, 1);
}


// Unprotected context with unprotected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromPBuffer)
{
    RenderTexture(false, false, 1);
}

// Protected context with unprotected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromPbuffer)
{
    RenderTexture(true, false, 1);
}

// Unprotected context with protected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromPbuffer)
{
    RenderTexture(false, true, 1);
}

// Protected context with protected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromPbuffer)
{
    RenderTexture(true, true, 1);
}


// Unprotected context with unprotected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromAndroidNativeBuffer)
{
    RenderTexture(false, false, 1);
}

// Protected context with unprotected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromAndroidNativeBuffer)
{
    RenderTexture(true, false, 1);
}

// Unprotected context with protected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromAndroidNativeBuffer)
{
    RenderTexture(false, true, 1);
}

// Protected context with protected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromAndroidNativeBuffer)
{
    RenderTexture(true, true, 1);
}
#endif

ANGLE_INSTANTIATE_TEST(EGLProtectedContentTest,
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
