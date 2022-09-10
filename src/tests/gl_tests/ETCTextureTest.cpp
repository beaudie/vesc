//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ETCTextureTest:
//   Tests for ETC lossy decode formats.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class ETCTextureTest : public ANGLETest<>
{
  protected:
    ETCTextureTest() : mTexture(0u)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteTextures(1, &mTexture); }

    GLuint mTexture;
};

// Tests a texture with ETC1 lossy decode format
TEST_P(ETCTextureTest, ETC1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[8] = {0x0, 0x0, 0xf8, 0x2, 0x43, 0xff, 0x4, 0x12};
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 4, 4, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 2, 2, 0,
                               sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 1, 1, 0,
                               sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 RGB8 lossy decode format
TEST_P(ETCTextureTest, ETC2RGB8Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x00, 0x00, 0xf8, 0x02, 0x43, 0xff, 0x04, 0x12,  // Individual/differential block
        0x1c, 0x65, 0xc6, 0x62, 0xff, 0xf0, 0xff, 0x00,  // T block
        0x62, 0xf2, 0xe3, 0x32, 0xff, 0x0f, 0xff, 0x00,  // H block
        0x71, 0x88, 0xfb, 0xee, 0x87, 0x07, 0x11, 0x1f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a cube map array texture with compressed ETC2 RGB8 format
TEST_P(ETCTextureTest, ETC2RGB8_CubeMapValidation)
{
    ANGLE_SKIP_TEST_IF(!(IsGLExtensionEnabled("GL_EXT_texture_cube_map_array") &&
                         (getClientMajorVersion() >= 3 && getClientMinorVersion() > 1)));

    constexpr GLsizei kInvalidTextureWidth  = 8;
    constexpr GLsizei kInvalidTextureHeight = 8;
    constexpr GLsizei kCubemapFaceCount     = 6;
    const std::vector<GLubyte> kInvalidTextureData(
        kInvalidTextureWidth * kInvalidTextureHeight * kCubemapFaceCount, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    EXPECT_GL_NO_ERROR();

    glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, kInvalidTextureWidth,
                           kInvalidTextureHeight, kCubemapFaceCount, 0, kInvalidTextureData.size(),
                           kInvalidTextureData.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    constexpr GLenum kFormat = GL_COMPRESSED_RGB8_ETC2;

    std::vector<GLubyte> arrayData;

    constexpr GLuint kWidth       = 4u;
    constexpr GLuint kHeight      = 4u;
    constexpr GLuint kDepth       = 6u;
    constexpr GLuint kPixelBytes  = 8u;
    constexpr GLuint kBlockWidth  = 4u;
    constexpr GLuint kBlockHeight = 4u;

    constexpr GLuint kNumBlocksWide = (kWidth + kBlockWidth - 1u) / kBlockWidth;
    constexpr GLuint kNumBlocksHigh = (kHeight + kBlockHeight - 1u) / kBlockHeight;
    constexpr GLuint kBytes         = kNumBlocksWide * kNumBlocksHigh * kPixelBytes * kDepth;

    arrayData.reserve(kBytes);

    glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, kFormat, kWidth, kHeight, kDepth, 0,
                           kBytes, arrayData.data());
    EXPECT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, kInvalidTextureWidth,
                              kInvalidTextureHeight, kDepth, GL_RGB, kInvalidTextureData.size(),
                              kInvalidTextureData.data());
    glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, kInvalidTextureWidth,
                              kInvalidTextureHeight, kDepth, GL_RGB, kInvalidTextureData.size(),
                              kInvalidTextureData.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests a texture with ETC2 SRGB8 lossy decode format
TEST_P(ETCTextureTest, ETC2SRGB8Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x00, 0x00, 0xf8, 0x02, 0x43, 0xff, 0x04, 0x12,  // Individual/differential block
        0x1c, 0x65, 0xc6, 0x62, 0xff, 0xf0, 0xff, 0x00,  // T block
        0x62, 0xf2, 0xe3, 0x32, 0xff, 0x0f, 0xff, 0x00,  // H block
        0x71, 0x88, 0xfb, 0xee, 0x87, 0x07, 0x11, 0x1f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, sizeof(pixel),
                                  pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 RGB8 punchthrough A1 lossy decode format
TEST_P(ETCTextureTest, ETC2RGB8A1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x80, 0x98, 0x59, 0x02, 0x6e, 0xe7, 0x44, 0x47,  // Individual/differential block
        0xeb, 0x85, 0x68, 0x30, 0x77, 0x73, 0x44, 0x44,  // T block
        0xb4, 0x05, 0xab, 0x92, 0xf8, 0x8c, 0x07, 0x73,  // H block
        0xbb, 0x90, 0x15, 0xba, 0x8a, 0x8c, 0xd5, 0x5f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0,
                           GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 SRGB8 punchthrough A1 lossy decode format
TEST_P(ETCTextureTest, ETC2SRGB8A1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x80, 0x98, 0x59, 0x02, 0x6e, 0xe7, 0x44, 0x47,  // Individual/differential block
        0xeb, 0x85, 0x68, 0x30, 0x77, 0x73, 0x44, 0x44,  // T block
        0xb4, 0x05, 0xab, 0x92, 0xf8, 0x8c, 0x07, 0x73,  // H block
        0xbb, 0x90, 0x15, 0xba, 0x8a, 0x8c, 0xd5, 0x5f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0,
                           GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 4,
                               4, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 2,
                               2, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 1,
                               1, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

class ETCToBCTextureTest : public ANGLETest<>
{
  protected:
    ETCToBCTextureTest() : mTexture(0u)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteTextures(1, &mTexture); }

    GLuint mTexture;
};

// Tests GPU compute transcode ETC2_RGB8 to BC1
TEST_P(ETCToBCTextureTest, ETC2Rgb8UnormToBC1)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    //! GetParam().isEnabled(Feature::SupportsComputeTranscodeEtcToBc));
    GLuint fboTexture      = 0;
    constexpr int kWidth   = 4;
    constexpr int kHeight  = 4;
    constexpr int kTexSize = 4;
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    glViewport(0, 0, kHeight, kHeight);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_ETC2, kTexSize, kTexSize);
    uint32_t data[2] = {0x14050505, 0x00ffff33};
    // the transcoded BC1 data
    // {0x0000a534, 0x05055555}
    // then decoded RGBA data are
    // 0xff000000 0xff000000 0xff000000 0xff000000
    // 0xff000000 0xff000000 0xff000000 0xff000000
    // 0xff000000 0xff000000 0xffa5a6a5 0xffa5a6a5
    // 0xff000000 0xff000000 0xffa5a6a5 0xffa5a6a5
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, GL_COMPRESSED_RGB8_ETC2,
                              sizeof(data), data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0, 0, 0, 0xff));
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor(0, 0, 0, 0xff));
    EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor(0xa5, 0xa6, 0xa5, 0xff));
    EXPECT_PIXEL_COLOR_EQ(3, 3, GLColor(0xa5, 0xa6, 0xa5, 0xff));
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ETCTextureTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ETCToBCTextureTest);
}  // anonymous namespace
