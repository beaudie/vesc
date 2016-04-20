//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CopyTextureTest.cpp: Tests of the GL_CHROMIUM_copy_texture extension

#include "test_utils/ANGLETest.h"

namespace angle
{

class CopyTextureTest : public ANGLETest
{
  protected:
    CopyTextureTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenTextures(2, mTextures);
        glBindTexture(GL_TEXTURE_2D, mTextures[1]);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1],
                               0);

        if (extensionEnabled("GL_CHROMIUM_copy_texture"))
        {
            glCopyTextureCHROMIUM = reinterpret_cast<PFNGLCOPYTEXTURECHROMIUMPROC>(
                eglGetProcAddress("glCopyTextureCHROMIUM"));
            glCopySubTextureCHROMIUM = reinterpret_cast<PFNGLCOPYSUBTEXTURECHROMIUMPROC>(
                eglGetProcAddress("glCopySubTextureCHROMIUM"));
        }
    }

    void TearDown() override
    {
        glDeleteTextures(2, mTextures);
        glDeleteFramebuffers(1, &mFramebuffer);

        ANGLETest::TearDown();
    }

    GLuint mTextures[2];
    GLuint mFramebuffer = 0;

    typedef void(GL_APIENTRYP PFNGLCOPYTEXTURECHROMIUMPROC)(GLuint sourceId,
                                                            GLuint destId,
                                                            GLint internalFormat,
                                                            GLenum destType,
                                                            GLboolean unpackFlipY,
                                                            GLboolean unpackPremultiplyAlpha,
                                                            GLboolean unpackUnmultiplyAlpha);
    PFNGLCOPYTEXTURECHROMIUMPROC glCopyTextureCHROMIUM = nullptr;

    typedef void(GL_APIENTRYP PFNGLCOPYSUBTEXTURECHROMIUMPROC)(GLuint sourceId,
                                                               GLuint destId,
                                                               GLint xoffset,
                                                               GLint yoffset,
                                                               GLint x,
                                                               GLint y,
                                                               GLsizei width,
                                                               GLsizei height,
                                                               GLboolean unpackFlipY,
                                                               GLboolean unpackPremultiplyAlpha,
                                                               GLboolean unpackUnmultiplyAlpha);
    PFNGLCOPYSUBTEXTURECHROMIUMPROC glCopySubTextureCHROMIUM = nullptr;
};

// Test to ensure that the basic functionality of the extension works.
TEST_P(CopyTextureTest, BasicCopyTexture)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, false, false,
                          false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(0, 0, pixels[0], pixels[1], pixels[2], pixels[3]);

    EXPECT_GL_NO_ERROR();
}

// Test to ensure that the basic functionality of the extension works.
TEST_P(CopyTextureTest, BasicCopySubTexture)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 0, 0, 0, 1, 1, false, false, false);

    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_EQ(0, 0, pixels[0], pixels[1], pixels[2], pixels[3]);

    EXPECT_GL_NO_ERROR();
}

// Test that CopyTexture cannot redefine an immutable texture and CopySubTexture can copy data to
// immutable textures
TEST_P(CopyTextureTest, ImmutableTexture)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    if (getClientVersion() < 3 &&
        (!extensionEnabled("GL_EXT_texture_storage") || !extensionEnabled("GL_OES_rgb8_rgba8")))
    {
        std::cout
            << "Test skipped due to missing ES3 or GL_EXT_texture_storage or GL_OES_rgb8_rgba8"
            << std::endl;
        return;
    }

    uint8_t pixels[1 * 4] = {255u, 0u, 0u, 255u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, 1, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    EXPECT_GL_NO_ERROR();

    // Should generate an error when the texture is redefined
    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, false, false,
                          false);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Should succeed when using CopySubTexture
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 0, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_EQ(0, 0, pixels[0], pixels[1], pixels[2], pixels[3]);

    EXPECT_GL_NO_ERROR();
}

// Test validation of internal formats in CopyTexture and CopySubTexture
TEST_P(CopyTextureTest, InternalFormat)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    std::vector<GLint> sourceFormats;
    sourceFormats.push_back(GL_ALPHA);
    sourceFormats.push_back(GL_RGB);
    sourceFormats.push_back(GL_RGBA);
    sourceFormats.push_back(GL_LUMINANCE);
    sourceFormats.push_back(GL_LUMINANCE_ALPHA);

    std::vector<GLint> destFormats;
    destFormats.push_back(GL_RGB);
    destFormats.push_back(GL_RGBA);

    if (extensionEnabled("GL_EXT_texture_format_BGRA8888"))
    {
        sourceFormats.push_back(GL_BGRA_EXT);
        destFormats.push_back(GL_BGRA_EXT);
    }

    // Test with glCopyTexture
    for (GLint sourceFormat : sourceFormats)
    {
        for (GLint destFormat : destFormats)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, sourceFormat, 1, 1, 0, sourceFormat, GL_UNSIGNED_BYTE,
                         nullptr);
            EXPECT_GL_NO_ERROR();

            glCopyTextureCHROMIUM(mTextures[0], mTextures[1], destFormat, GL_UNSIGNED_BYTE, false,
                                  false, false);

            EXPECT_GL_NO_ERROR();
        }
    }

    // Test with glCopySubTexture
    for (GLint sourceFormat : sourceFormats)
    {
        for (GLint destFormat : destFormats)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, sourceFormat, 1, 1, 0, sourceFormat, GL_UNSIGNED_BYTE,
                         nullptr);
            EXPECT_GL_NO_ERROR();

            glBindTexture(GL_TEXTURE_2D, mTextures[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, destFormat, 1, 1, 0, destFormat, GL_UNSIGNED_BYTE,
                         nullptr);
            EXPECT_GL_NO_ERROR();

            glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 0, 0, 0, 1, 1, false, false,
                                     false);

            EXPECT_GL_NO_ERROR();
        }
    }
}

TEST_P(CopyTextureTest, InternalFormatNotSupported)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    std::vector<GLint> unsupportedDestFormats;
    unsupportedDestFormats.push_back(GL_ALPHA);
    unsupportedDestFormats.push_back(GL_LUMINANCE);
    unsupportedDestFormats.push_back(GL_LUMINANCE_ALPHA);

    if (!extensionEnabled("GL_EXT_texture_format_BGRA8888"))
    {
        unsupportedDestFormats.push_back(GL_BGRA_EXT);
    }

    // Check unsupported format reports an error.
    for (GLint unsupportedDestFormat : unsupportedDestFormats)
    {
        glCopyTextureCHROMIUM(mTextures[0], mTextures[1], unsupportedDestFormat, GL_UNSIGNED_BYTE,
                              false, false, false);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    for (GLint unsupportedDestFormat : unsupportedDestFormats)
    {
        glBindTexture(GL_TEXTURE_2D, mTextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, unsupportedDestFormat, 1, 1, 0, unsupportedDestFormat,
                     GL_UNSIGNED_BYTE, nullptr);
        glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 0, 0, 0, 1, 1, false, false, false);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

// Test to ensure that the destination texture is redefined if the properties are different.
TEST_P(CopyTextureTest, RedefineDestinationTexture)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t pixels[4 * 4] = {255u, 0u, 0u, 255u, 255u, 0u, 0u, 255u,
                             255u, 0u, 0u, 255u, 255u, 0u, 0u, 255u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_NO_ERROR();

    // GL_INVALID_OPERATION due to "intrinsic format" != "internal format".
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // GL_INVALID_VALUE due to bad dimensions.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // If the dest texture has different properties, glCopyTextureCHROMIUM()
    // redefines them.
    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, false, false,
                          false);
    EXPECT_GL_NO_ERROR();

    // glTexSubImage2D() succeeds because mTextures[1] is redefined into 2x2
    // dimension and GL_RGBA format.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_EQ(1, 1, pixels[12], pixels[13], pixels[14], pixels[15]);
    EXPECT_GL_NO_ERROR();
}

// Test that invalid dimensions in CopySubTexture are validated
TEST_P(CopyTextureTest, CopySubTextureDimension)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();

    // xoffset < 0
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], -1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // x < 0
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 1, 1, -1, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // xoffset + width > dest_width
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 2, 2, 0, 0, 2, 2, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // x + width > source_width
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 0, 1, 1, 2, 2, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that invalid IDs in CopyTexture are validated
TEST_P(CopyTextureTest, CopyTextureInvalidTextureIds)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[0], 99993, GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(99994, mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(99995, 99996, GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, false, false,
                          false);
    EXPECT_GL_NO_ERROR();
}

// Test that invalid IDs in CopySubTexture are validated
TEST_P(CopyTextureTest, CopySubTextureInvalidTextureIds)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopySubTextureCHROMIUM(mTextures[0], 99993, 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(99994, mTextures[1], 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(99995, 99996, 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();
}

// Test that using an offset in CopySubTexture works correctly
TEST_P(CopyTextureTest, CopySubTextureOffset)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopySubTextureCHROMIUM);

    uint8_t rgbaPixels[4 * 4] = {255u, 0u, 0u,   255u, 0u, 255u, 0u, 255u,
                                 0u,   0u, 255u, 255u, 0u, 0u,   0u, 255u};
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    uint8_t transparentPixels[4 * 4] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                                        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, transparentPixels);

    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 1, 1, 0, 0, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 1, 0, 1, 0, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();
    glCopySubTextureCHROMIUM(mTextures[0], mTextures[1], 0, 1, 0, 1, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);      // transparent
    EXPECT_PIXEL_EQ(1, 1, 255, 0, 0, 255);  // red
    EXPECT_PIXEL_EQ(1, 0, 0, 255, 0, 255);  // green
    EXPECT_PIXEL_EQ(0, 1, 0, 0, 255, 255);  // blue
    EXPECT_GL_NO_ERROR();
}

// Test that flipping the Y component works correctly
TEST_P(CopyTextureTest, FlipY)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t rgbaPixels[4 * 4] = {255u, 255u, 255u, 255u, 127u, 127u, 127u, 127u,
                                 63u,  63u,  63u,  127u, 255u, 255u, 255u, 0u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, GL_TRUE, GL_FALSE,
                          GL_FALSE);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_EQ(0, 0, 63, 63, 63, 127);
    EXPECT_PIXEL_EQ(1, 0, 255, 255, 255, 0);
    EXPECT_PIXEL_EQ(0, 1, 255, 255, 255, 255);
    EXPECT_PIXEL_EQ(1, 1, 127, 127, 127, 127);
    EXPECT_GL_NO_ERROR();
}

// Test that premultipying the alpha on copy works correctly
TEST_P(CopyTextureTest, PremultiplyAlpha)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t rgbaPixels[4 * 4] = {255u, 255u, 255u, 255u, 255u, 255u, 255u, 127u,
                                 127u, 127u, 127u, 127u, 255u, 255u, 255u, 0u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, GL_FALSE, GL_TRUE,
                          GL_FALSE);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_NEAR(0, 0, 255, 255, 255, 255, 1.0);
    EXPECT_PIXEL_NEAR(1, 0, 127, 127, 127, 127, 1.0);
    EXPECT_PIXEL_NEAR(0, 1, 63, 63, 63, 127, 1.0);
    EXPECT_PIXEL_NEAR(1, 1, 0, 0, 0, 0, 1.0);
    EXPECT_GL_NO_ERROR();
}

// Test that unemultipying the alpha on copy works correctly
TEST_P(CopyTextureTest, UnmultiplyAlpha)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t rgbaPixels[4 * 4] = {255u, 255u, 255u, 255u, 127u, 127u, 127u, 127u,
                                 63u,  63u,  63u,  127u, 255u, 255u, 255u, 0u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, GL_FALSE, GL_FALSE,
                          GL_TRUE);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_NEAR(0, 0, 255, 255, 255, 255, 1.0);
    EXPECT_PIXEL_NEAR(1, 0, 255, 255, 255, 127, 1.0);
    EXPECT_PIXEL_NEAR(0, 1, 127, 127, 127, 127, 1.0);
    EXPECT_PIXEL_NEAR(1, 1, 255, 255, 255, 0, 1.0);
    EXPECT_GL_NO_ERROR();
}

// Test that unemultipying and premultiplying the alpha is the same as doing neither
TEST_P(CopyTextureTest, UnmultiplyAndPremultplyAlpha)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available." << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCopyTextureCHROMIUM);

    uint8_t rgbaPixels[4 * 4] = {255u, 255u, 255u, 255u, 127u, 127u, 127u, 127u,
                                 63u,  63u,  63u,  127u, 255u, 255u, 255u, 0u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    glCopyTextureCHROMIUM(mTextures[0], mTextures[1], GL_RGBA, GL_UNSIGNED_BYTE, GL_FALSE, GL_TRUE,
                          GL_TRUE);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
              glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_NEAR(0, 0, 255, 255, 255, 255, 1.0);
    EXPECT_PIXEL_NEAR(1, 0, 127, 127, 127, 127, 1.0);
    EXPECT_PIXEL_NEAR(0, 1, 63, 63, 63, 127, 1.0);
    EXPECT_PIXEL_NEAR(1, 1, 255, 255, 255, 0, 1.0);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(CopyTextureTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL(), ES2_OPENGLES());

}  // namespace angle
