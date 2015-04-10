//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackUnpackTest:
//   Tests the correctness of eglImage.
//

#include "test_utils/ANGLETest.h"

namespace angle
{

class ImageTest : public ANGLETest
{
protected:
    ImageTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string vsSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            }
        );

        const std::string textureFSSource = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        mTextureProgram = CompileProgram(vsSource, textureFSSource);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        ANGLETest::TearDown();

        glDeleteProgram(mTextureProgram);
    }

    void createEGLImage2DTextureSource(size_t width, size_t height, GLenum format, GLenum type, void *data,
                                       GLuint* outSourceTexture, EGLImageKHR* outSourceImage)
    {
        // Create a source texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_2D, source);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(),
                                              EGL_GL_TEXTURE_2D_KHR, reinterpret_cast<EGLClientBuffer>(source),
                                              nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage = image;
    }

    void createEGLImageCubemapTextureSource(size_t width, size_t height, GLenum format, GLenum type, uint8_t *data, size_t dataStride,
                                            EGLenum imageTarget, GLuint* outSourceTexture, EGLImageKHR* outSourceImage)
    {

         // Create a source texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_CUBE_MAP, source);

        for (size_t faceIdx = 0; faceIdx < 6; faceIdx++)
        {
            glTexImage2D(faceIdx + GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, width, height, 0,
                         format, type, data + (faceIdx * dataStride));
        }

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(),
                                              imageTarget, reinterpret_cast<EGLClientBuffer>(source),
                                              nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage = image;
    }

    void createEGLImage3DTextureSource(size_t width, size_t height, size_t depth, GLenum format, GLenum type, void *data,
                                       size_t imageLayer, GLuint* outSourceTexture, EGLImageKHR* outSourceImage)
    {
        // Create a source texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_3D, source);

        glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, 0, format, type, data);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLint attribs[] =
        {
            EGL_GL_TEXTURE_ZOFFSET_KHR, imageLayer,
            EGL_NONE,
        };
        EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(),
                                              EGL_GL_TEXTURE_3D_KHR, reinterpret_cast<EGLClientBuffer>(source),
                                              attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage = image;
    }

    void createEGLImageRenderbufferSource(size_t width, size_t height, GLenum internalFormat, GLubyte data[4],
                                          GLuint* outSourceRenderbuffer, EGLImageKHR* outSourceImage)
    {
         // Create a source texture
        GLuint source;
        glGenRenderbuffers(1, &source);
        glBindRenderbuffer(GL_RENDERBUFFER, source);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);

        // Create a framebuffer and clear it to set the data
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, source);

        glClearColor(data[0] / 255.0f, data[1] / 255.0f, data[2] / 255.0f, data[3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDeleteFramebuffers(1, &framebuffer);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(),
                                              EGL_GL_RENDERBUFFER_KHR, reinterpret_cast<EGLClientBuffer>(source),
                                              nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceRenderbuffer = source;
        *outSourceImage = image;
    }

    void createEGLImageTargetTexture2D(EGLImageKHR image, GLuint *outTargetTexture)
    {
        // Create a target texture from the image
        GLuint target;
        glGenTextures(1, &target);
        glBindTexture(GL_TEXTURE_2D, target);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

        ASSERT_GL_NO_ERROR();

        *outTargetTexture = target;
    }

    void createEGLImageTargetRenderbuffer(EGLImageKHR image, GLuint *outTargetRenderbuffer)
    {
        // Create a target texture from the image
        GLuint target;
        glGenRenderbuffers(1, &target);
        glBindRenderbuffer(GL_RENDERBUFFER, target);
        glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);

        ASSERT_GL_NO_ERROR();

        *outTargetRenderbuffer = target;
    }

    void verifyResults2D(GLuint texture, GLubyte data[4])
    {
        // Draw a quad with the target texture
        glUseProgram(mTextureProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(mTextureUniformLocation, 0);

        drawQuad(mTextureProgram, "position", 0.5f);

        EXPECT_GL_NO_ERROR();

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_EQ(0, 0, data[0], data[1], data[2], data[3]);
    }

    void verifyResultsRenderbuffer(GLuint renderbuffer, GLubyte data[4])
    {
        // Bind the renderbuffer to a framebuffer
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

        EXPECT_GL_NO_ERROR();

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_EQ(0, 0, data[0], data[1], data[2], data[3]);

        glDeleteFramebuffers(1, &framebuffer);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

TEST_P(ImageTest, Source2DTarget2D)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                      "EGL_KHR_gl_texture_2D_image is not available." << std::endl;
        return;
    }

    GLubyte data[4] = { 255, 0, 255, 255 };

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, data);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

TEST_P(ImageTest, Source2DTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
            "EGL_KHR_gl_texture_2D_image is not available." << std::endl;
        return;
    }

    GLubyte data[4] = { 255, 0, 255, 255 };

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetRenderbuffer(image, &target);

    // Expect that the target renderbuffer has the same color as the source texture
    verifyResultsRenderbuffer(target, data);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteRenderbuffers(1, &target);
}

TEST_P(ImageTest, SourceCubeTarget2D)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_cubemap_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_cubemap_image is not available." << std::endl;
        return;
    }

    GLubyte data[24] =
    {
        255,   0, 255, 255,
        255, 255, 255, 255,
        255,   0,   0, 255,
          0,   0, 255, 255,
          0, 255,   0, 255,
          0,   0,   0, 255,
    };

    for (size_t faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8_t*>(data), sizeof(GLubyte) * 4,
                                           EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetTexture2D(image, &target);

        // Expect that the target texture has the same color as the source texture
        verifyResults2D(target, &data[faceIdx * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, SourceCubeTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_cubemap_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_cubemap_image is not available." << std::endl;
        return;
    }

    GLubyte data[24] =
    {
        255,   0, 255, 255,
        255, 255, 255, 255,
        255,   0,   0, 255,
          0,   0, 255, 255,
          0, 255,   0, 255,
          0,   0,   0, 255,
    };

    for (size_t faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8_t*>(data), sizeof(GLubyte) * 4,
                                           EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetRenderbuffer(image, &target);

        // Expect that the target texture has the same color as the source texture
        verifyResultsRenderbuffer(target, &data[faceIdx * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteRenderbuffers(1, &target);
    }
}

TEST_P(ImageTest, Source3DTargetTexture)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_3D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available." << std::endl;
        return;
    }

    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_3D"))
    {
        std::cout << "Test skipped because 3D textures are not available." << std::endl;
        return;
    }

    const size_t depth = 2;
    GLubyte data[4 * depth] =
    {
        255,   0, 255, 255,
        255, 255,   0, 255,
    };

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, data, layer, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetTexture2D(image, &target);

        // Expect that the target renderbuffer has the same color as the source texture
        verifyResults2D(target, &data[layer * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, Source3DTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_3D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available." << std::endl;
        return;
    }

    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_3D"))
    {
        std::cout << "Test skipped because 3D textures are not available." << std::endl;
        return;
    }

    const size_t depth = 2;
    GLubyte data[4 * depth] =
    {
        255,   0, 255, 255,
        255, 255,   0, 255,
    };

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, data, layer, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetRenderbuffer(image, &target);

        // Expect that the target renderbuffer has the same color as the source texture
        verifyResultsRenderbuffer(target, &data[layer * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, SourceRenderbufferTargetTexture)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_renderbuffer_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available." << std::endl;
        return;
    }

    GLubyte data[4] = { 255, 0, 255, 255 };

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Expect that the target renderbuffer has the same color as the source texture
    verifyResults2D(target, data);

    // Clean up
    glDeleteRenderbuffers(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

TEST_P(ImageTest, SourceRenderbufferTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_renderbuffer_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available." << std::endl;
        return;
    }

    GLubyte data[4] = { 255, 0, 255, 255 };

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetRenderbuffer(image, &target);

    // Expect that the target renderbuffer has the same color as the source texture
    verifyResultsRenderbuffer(target, data);

    // Clean up
    glDeleteRenderbuffers(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteRenderbuffers(1, &target);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(ImageTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());

}
