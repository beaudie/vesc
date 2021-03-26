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
        mMajorVersion = GetParam().majorVersion;
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
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            clientVersion,
                            EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_WINDOW_BIT),
                            EGL_BIND_TO_TEXTURE_RGBA,
                            EGL_TRUE,
                            EGL_NONE};

        EGLint count = 0;
        bool result  = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result);
        EXPECT_EGL_TRUE(count > 0);
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
        EGLint attribsProtected[]   = {EGL_WIDTH,
                                     kWidth,
                                     EGL_HEIGHT,
                                     kHeight,
                                     EGL_TEXTURE_FORMAT,
                                     EGL_TEXTURE_RGBA,
                                     EGL_TEXTURE_TARGET,
                                     EGL_TEXTURE_2D,
                                     EGL_PROTECTED_CONTENT_EXT,
                                     EGL_TRUE,
                                     EGL_NONE};
        EGLint attribsUnProtected[] = {EGL_WIDTH,
                                       kWidth,
                                       EGL_HEIGHT,
                                       kHeight,
                                       EGL_TEXTURE_FORMAT,
                                       EGL_TEXTURE_RGBA,
                                       EGL_TEXTURE_TARGET,
                                       EGL_TEXTURE_2D,
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
        EXPECT_EGL_SUCCESS();
        result = (*image != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    bool createTexture(EGLBoolean isProtected,
                       EGLContext context,
                       GLenum textureUnit,
                       GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (isProtected)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PROTECTED_EXT, GL_TRUE);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);
        EXPECT_GL_NO_ERROR();
        result = (texture != 0);
        EXPECT_TRUE(result);
        *textureId = texture;
        return result;
    }

    bool createTextureFromImage(EGLBoolean isProtected,
                                EGLContext context,
                                EGLImage image,
                                GLenum textureUnit,
                                GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (isProtected)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PROTECTED_EXT, GL_TRUE);
        }
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        EXPECT_GL_NO_ERROR();
        result = (texture != 0);
        EXPECT_TRUE(result);
        *textureId = texture;
        return result;
    }

    bool createTextureFromPbuffer(EGLBoolean isProtected,
                                  EGLSurface pBuffer,
                                  GLenum textureUnit,
                                  GLuint *textureId)
    {
        bool result    = false;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (isProtected)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PROTECTED_EXT, GL_TRUE);
        }
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(texture != 0);
        result     = eglBindTexImage(mDisplay, pBuffer, EGL_BACK_BUFFER);
        *textureId = texture;
        return result;
    }

    bool fillTexture(GLuint textureId)
    {
        GLuint pixels[kWidth * kHeight];
        for (uint32_t i = 0; i < (kWidth * kHeight); i++)
        {
            pixels[i] = 0xFF0000FF;  // RED
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        // This may not work since it is not protected data going to possibly protected texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        (void *)pixels);
        EXPECT_GL_NO_ERROR();
        return true;
    }

    bool renderTexture(GLuint textureId)
    {
        const char *vertexShader   = R"(
            precision mediump float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            }
        )";
        const char *fragmentShader = R"(
            precision mediump float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        )";

        GLuint program = CompileProgram(vertexShader, fragmentShader);
        glUseProgram(program);
        GLint texture2DUniformLocation = glGetUniformLocation(program, "tex");
        glUniform1i(texture2DUniformLocation, 0);
        drawQuad(program, "position", 0.5f);

        EXPECT_GL_NO_ERROR();

        return true;
    }

    bool createRenderbuffer(EGLBoolean isProtected, EGLContext context, GLuint *renderbufferId)
    {
        bool result     = false;
        *renderbufferId = 0;
        glGenRenderbuffers(1, renderbufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, kWidth, kHeight);
        // Note, there is no way to mark this protected, but attached a FBO needs to be - TBD
        EXPECT_GL_NO_ERROR();
        result = (*renderbufferId != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createRenderbufferFromImage(EGLBoolean isProtected, EGLImage image, GLuint *renderbufferId)
    {
        bool result     = false;
        *renderbufferId = 0;
        glGenRenderbuffers(1, renderbufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbufferId);
        glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);
        EXPECT_GL_NO_ERROR();
        result = (*renderbufferId != 0);
        EXPECT_TRUE(result);
        return result;
    }

    bool createAndroidClientBuffer(EGLBoolean useProtected,
                                   bool useRenderbuffer,
                                   bool useTexture,
                                   EGLClientBuffer *clientBuffer)
    {
        bool result = false;
        EGLint nativeBufferUsage =
            0 | (useProtected ? EGL_NATIVE_BUFFER_USAGE_PROTECTED_BIT_ANDROID : 0) |
            (useRenderbuffer ? EGL_NATIVE_BUFFER_USAGE_RENDERBUFFER_BIT_ANDROID : 0) |
            (useTexture ? EGL_NATIVE_BUFFER_USAGE_TEXTURE_BIT_ANDROID : 0);

        EGLint attribs[] = {EGL_WIDTH,
                            kWidth,
                            EGL_HEIGHT,
                            kHeight,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_NATIVE_BUFFER_USAGE_ANDROID,
                            nativeBufferUsage,
                            EGL_NONE};

        *clientBuffer = eglCreateNativeClientBufferANDROID(attribs);
        EXPECT_EGL_SUCCESS();
        result = (*clientBuffer != nullptr);
        EXPECT_TRUE(result);
        return result;
    }

    void pbufferTest(bool isProtectedContext, bool isProtectedSurface);
    void windowTest(bool isProtectedContext, bool isProtectedSurface);
    void textureTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromImageTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromPbufferTest(bool isProtectedContext, bool isProtectedTexture);
    void textureFromAndroidNativeBufferTest(bool isProtectedContext, bool isProtectedTexture);
    void renderbufferTest(bool isProtectedContext, bool isProtectedTexture);

    bool renderRenderbuffer(GLuint renderbufferId);

    void checkReadPixelsResult(bool isProtectedContext, bool isProtectedSurface)
    {
        // Results
        static const GLColor kTransparentBlack = angle::MakeGLColor(0.0, 0.0, 0.0, 0.0);
        if (isProtectedContext)
        {
            if (isProtectedSurface)
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
            }
        }
        else
        {
            if (isProtectedSurface)
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, kTransparentBlack);
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
            }
        }
    }

    void checkSwapBuffersResult(const char *color, bool isProtectedContext, bool isProtectedSurface)
    {
        std::this_thread::sleep_for(1s);
        if (isProtectedContext)
        {
            if (isProtectedSurface)
            {
                std::cout << "Operator should see color: " << color << std::endl;
            }
            else
            {
                std::cout << "Operator should see color BLACK" << std::endl;
            }
        }
        else
        {
            if (isProtectedSurface)
            {
                std::cout << "Operator should see color BLACK" << std::endl;
            }
            else
            {
                std::cout << "Operator should see color: " << color << std::endl;
            }
        }
    }

    EGLDisplay mDisplay         = EGL_NO_DISPLAY;
    EGLint mMajorVersion        = 0;
    static const EGLint kWidth  = 16;
    static const EGLint kHeight = 16;
};

void EGLProtectedContentTest::pbufferTest(bool isProtectedContext, bool isProtectedSurface)
{
    if (isProtectedContext || isProtectedSurface)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
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
    checkReadPixelsResult(isProtectedContext, isProtectedSurface);

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
    pbufferTest(false, false);
}
#if 0
// Unprotected context with Protected PbufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedPbufferSurface)
{
    pbufferTest(false, true);
}

// Protected context with Unprotected PbufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedPbufferSurface)
{
    pbufferTest(true, false);
}
#endif
// Protected context with Protected PbufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedPbufferSurface)
{
    pbufferTest(true, true);
}

void EGLProtectedContentTest::windowTest(bool isProtectedContext, bool isProtectedSurface)
{
    if (isProtectedContext || isProtectedSurface)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
    }

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(isProtectedContext, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
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
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("Red", isProtectedContext, isProtectedSurface);

    // Green
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("Green", isProtectedContext, isProtectedSurface);

    // Blue
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";
    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("Blue", isProtectedContext, isProtectedSurface);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}
#
// Unprotected context with Unprotected WindowSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedWindowSurface)
{
    windowTest(false, false);
}
#if 0
// Unprotected context with Protected WindowSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedWindowSurface)
{
    WindowTest(false, true);
}

// Protected context with Unprotected WindowSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedWindowSurface)
{
    windowTest(true, false);
}
#endif
// Protected context with Protected WindowSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedWindowSurface)
{
    windowTest(true, true);
}

void EGLProtectedContentTest::textureTest(bool isProtectedContext, bool isProtectedTexture)
{
    if (isProtectedContext || isProtectedTexture)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_protected_textures"));
    }

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    GLuint texture = 0;
    EXPECT_TRUE(createTexture(isProtectedTexture, context, GL_TEXTURE0, &texture));
    EXPECT_TRUE(fillTexture(texture));
    EXPECT_TRUE(renderTexture(texture));

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    checkReadPixelsResult(isProtectedContext, isProtectedSurface);

    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("RED", isProtectedContext, isProtectedSurface);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    glDeleteTextures(1, &texture);

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Unprotected context with unprotected texture
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTexture)
{
    textureTest(false, false);
}
#if 0
// Unprotected context with protected texture
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTexture)
{
    textureTest(false, true);
}

// Protected context with unprotected texture
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTexture)
{
    textureTest(true, false);
}

// Protected context with protected texture
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTexture)
{
    textureTest(true, true);
}
#endif

void EGLProtectedContentTest::textureFromImageTest(bool isProtectedContext, bool isProtectedTexture)
{
    if (isProtectedContext || isProtectedTexture)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_protected_textures"));
    }

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    GLuint srcTexture = 0;
    EXPECT_TRUE(createTexture(isProtectedTexture, context, GL_TEXTURE0, &srcTexture));
    EXPECT_TRUE(fillTexture(srcTexture));

    EGLImage image               = EGL_NO_IMAGE;
    EGLClientBuffer clientBuffer = (void *)(static_cast<intptr_t>(srcTexture));
    EXPECT_TRUE(createImage(isProtectedTexture, context, EGL_GL_TEXTURE_2D, clientBuffer, &image));

    GLuint texture = 0;
    EXPECT_TRUE(createTextureFromImage(isProtectedTexture, context, image, GL_TEXTURE0, &texture));
    EXPECT_TRUE(renderTexture(texture));

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    checkReadPixelsResult(isProtectedContext, isProtectedSurface);

    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("RED", isProtectedContext, isProtectedSurface);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    glDeleteTextures(1, &texture);

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}
#if 0
// Unprotected context with unprotected texture from EGL image
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromImage)
{
    textureFromImageTest(false, false);
}

// Unprotected context with protected texture from EGL image
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromImage)
{
    textureFromImageTest(false, true);
}

// Protected context with unprotected texture from EGL image
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromImage)
{
    textureFromImageTest(true, false);
}

// Protected context with protected texture from EGL image
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromImage)
{
    textureFromImageTest(true, true);
}
#endif

void EGLProtectedContentTest::textureFromPbufferTest(bool isProtectedContext,
                                                     bool isProtectedTexture)
{
    if (isProtectedContext || isProtectedTexture)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_protected_textures"));
    }

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
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
    checkReadPixelsResult(isProtectedContext, isProtectedSurface);

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    GLuint texture = 0;
    EXPECT_TRUE(
        createTextureFromPbuffer(isProtectedTexture, pBufferSurface, GL_TEXTURE0, &texture));
    EXPECT_TRUE(renderTexture(texture));

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    checkReadPixelsResult(isProtectedContext, isProtectedTexture);

    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("RED", isProtectedContext, isProtectedTexture);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglReleaseTexImage(mDisplay, pBufferSurface, EGL_BACK_BUFFER);

    glDeleteTextures(1, &texture);

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroySurface(mDisplay, pBufferSurface);
    pBufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}
#if 0
// Unprotected context with unprotected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromPBuffer)
{
    textureFromPbufferTest(false, false);
}

// Unprotected context with protected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromPbuffer)
{
    textureFromPbufferTest(false, true);
}

// Protected context with unprotected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromPbuffer)
{
    textureFromPbufferTest(true, false);
}

// Protected context with protected texture from BindTex of PBufferSurface
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromPbuffer)
{
    textureFromPbufferTest(true, true);
}
#endif

void EGLProtectedContentTest::textureFromAndroidNativeBufferTest(bool isProtectedContext,
                                                                 bool isProtectedTexture)
{
    if (isProtectedContext || isProtectedTexture)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_protected_content"));
        ANGLE_SKIP_TEST_IF(
            !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_get_native_client_buffer"));
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_protected_textures"));
    }

    bool isProtectedSurface = isProtectedTexture;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(false, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("WindowTest", kWidth, kHeight);
    EGLSurface windowSurface          = EGL_NO_SURFACE;
    EGLBoolean createWinSurfaceResult = createWindowSurface(
        isProtectedSurface, config, osWindow->getNativeWindow(), &windowSurface);
    EXPECT_TRUE(createWinSurfaceResult);
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EGLClientBuffer clientBuffer = nullptr;
    EXPECT_TRUE(createAndroidClientBuffer(isProtectedTexture, false, true, &clientBuffer));

    EGLImage image = EGL_NO_IMAGE;
    EXPECT_TRUE(
        createImage(isProtectedTexture, context, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, &image));

    GLuint texture = 0;
    EXPECT_TRUE(createTextureFromImage(isProtectedTexture, context, image, GL_TEXTURE0, &texture));
    EXPECT_TRUE(fillTexture(texture));
    EXPECT_TRUE(renderTexture(texture));

    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";
    checkReadPixelsResult(isProtectedContext, isProtectedTexture);

    eglSwapBuffers(mDisplay, windowSurface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    checkSwapBuffersResult("RED", isProtectedContext, isProtectedTexture);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    glDeleteTextures(1, &texture);

    eglDestroyImage(mDisplay, image);
    image = EGL_NO_IMAGE;

    eglDestroySurface(mDisplay, windowSurface);
    windowSurface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}
#if 0
// Unprotected context with unprotected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, UnprotectedContextWithUnprotectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(false, false);
}

// Unprotected context with protected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, UnprotectedContextWithProtectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(false, true);
}

// Protected context with unprotected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, ProtectedContextWithUnprotectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(true, false);
}

// Protected context with protected texture from EGL image from Android native buffer
TEST_P(EGLProtectedContentTest, ProtectedContextWithProtectedTextureFromAndroidNativeBuffer)
{
    textureFromAndroidNativeBufferTest(true, true);
}
#endif

ANGLE_INSTANTIATE_TEST(EGLProtectedContentTest,
                       //                       WithNoFixture(ES2_OPENGLES()),
                       //                       WithNoFixture(ES3_OPENGLES()),
                       //                       WithNoFixture(ES2_OPENGL()),
                       //                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
