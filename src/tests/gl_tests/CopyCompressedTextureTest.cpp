//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CopyCompressedTextureTest.cpp: Tests of the GL_CHROMIUM_copy_compressed_texture extension

#include "test_utils/ANGLETest.h"

namespace angle
{

class CopyCompressedTextureTest : public ANGLETest
{
  protected:
    CopyCompressedTextureTest()
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

        mProgram = CompileProgram(
            "attribute vec2 a_position;\n"
            "varying vec2 v_texcoord;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(a_position, 0.0, 1.0);\n"
            "   v_texcoord = (a_position + 1.0) * 0.5;\n"
            "}\n",
            "precision mediump float;\n"
            "uniform sampler2D u_texture;\n"
            "varying vec2 v_texcoord;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(u_texture, v_texcoord);\n"
            "}\n");
        EXPECT_NE(0u, mProgram);

        if (extensionEnabled("GL_CHROMIUM_copy_compressed_texture"))
        {
            glCompressedCopyTextureCHROMIUM =
                reinterpret_cast<PFNGLCOMPRESSEDCOPYTEXTURECHROMIUMPROC>(
                    eglGetProcAddress("glCompressedCopyTextureCHROMIUM"));
        }
    }

    void TearDown() override
    {
        glDeleteTextures(2, mTextures);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram     = 0;
    GLuint mTextures[2] = {0, 0};

    PFNGLCOMPRESSEDCOPYTEXTURECHROMIUMPROC glCompressedCopyTextureCHROMIUM = nullptr;
};

namespace
{

const uint8_t CompressedImageColor[4] = {255u, 0u, 0u, 255u};

// Single compressed ATC block of source pixels all set to:
// CompressedImageColor.
const uint8_t CompressedImageATC[8] = {0x0, 0x7c, 0x0, 0xf8, 0x55, 0x55, 0x55, 0x55};

// Single compressed ATCIA block of source pixels all set to:
// CompressedImageColor.
const uint8_t CompressedImageATCIA[16] = {0xff, 0xff, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0,
                                          0x0,  0x7c, 0x0, 0xf8, 0x55, 0x55, 0x55, 0x55};

// Single compressed DXT1 block of source pixels all set to:
// CompressedImageColor.
const uint8_t CompressedImageDXT1[8] = {0x00, 0xf8, 0x00, 0xf8, 0xaa, 0xaa, 0xaa, 0xaa};

// Single compressed DXT5 block of source pixels all set to:
// CompressedImageColor.
const uint8_t CompressedImageDXT5[16] = {0xff, 0xff, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0,
                                         0x0,  0xf8, 0x0, 0xf8, 0xaa, 0xaa, 0xaa, 0xaa};

// Single compressed DXT1 block of source pixels all set to:
// CompressedImageColor.
const uint8_t CompressedImageETC1[8] = {0x0, 0x0, 0xf8, 0x2, 0xff, 0xff, 0x0, 0x0};

}  // anonymous namespace

// Test to ensure that the basic functionality of the extension works.
TEST_P(CopyCompressedTextureTest, Basic)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_compressed_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_compressed_texture is not available."
                  << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCompressedCopyTextureCHROMIUM);

    if (!extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        std::cout << "Test skipped because GL_EXT_texture_compression_dxt1 is not available."
                  << std::endl;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glCompressedCopyTextureCHROMIUM(mTextures[0], mTextures[1]);
    EXPECT_GL_NO_ERROR();

    // Load texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    GLint textureLoc = glGetUniformLocation(mProgram, "u_texture");
    glUseProgram(mProgram);
    glUniform1i(textureLoc, 0);

    // Draw.
    drawQuad(mProgram, "a_position", 0.5f);

    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, CompressedImageColor[0],
                    CompressedImageColor[1], CompressedImageColor[2], CompressedImageColor[3]);
    EXPECT_GL_NO_ERROR();
}

// Test validation of compressed formats
TEST_P(CopyCompressedTextureTest, InternalFormat)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_compressed_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_compressed_texture is not available."
                  << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCompressedCopyTextureCHROMIUM);

    struct Data
    {
        GLint format;
        const uint8_t *data;
        GLsizei dataSize;

        Data() : Data(GL_NONE, nullptr, 0) {}
        Data(GLint format, const uint8_t *data, GLsizei dataSize)
            : format(format), data(data), dataSize(dataSize)
        {
        }
    };
    std::vector<Data> supportedFormats;

    if (extensionEnabled("GL_AMD_compressed_ATC_texture"))
    {
        supportedFormats.push_back(
            Data(GL_ATC_RGB_AMD, CompressedImageATC, sizeof(CompressedImageATC)));
        supportedFormats.push_back(Data(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD, CompressedImageATCIA,
                                        sizeof(CompressedImageATCIA)));
    }
    if (extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        supportedFormats.push_back(Data(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, CompressedImageDXT1,
                                        sizeof(CompressedImageDXT1)));
    }
    if (extensionEnabled("GL_ANGLE_texture_compression_dxt5"))
    {
        supportedFormats.push_back(Data(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, CompressedImageDXT5,
                                        sizeof(CompressedImageDXT5)));
    }
    if (extensionEnabled("GL_OES_compressed_ETC1_RGB8_texture"))
    {
        supportedFormats.push_back(
            Data(GL_ETC1_RGB8_OES, CompressedImageETC1, sizeof(CompressedImageETC1)));
    }

    for (const auto &supportedFormat : supportedFormats)
    {
        glBindTexture(GL_TEXTURE_2D, mTextures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, supportedFormat.format, 4, 4, 0,
                               supportedFormat.dataSize, supportedFormat.data);
        EXPECT_GL_NO_ERROR();

        glBindTexture(GL_TEXTURE_2D, mTextures[1]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glCompressedCopyTextureCHROMIUM(mTextures[0], mTextures[1]);
        EXPECT_GL_NO_ERROR();
    }
}

// Test that uncompressed textures generate errors when copying
TEST_P(CopyCompressedTextureTest, InternalFormatNotSupported)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_compressed_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_compressed_texture is not available."
                  << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCompressedCopyTextureCHROMIUM);

    const uint8_t uncompressedPixels[1 * 4] = {255u, 0u, 0u, 255u};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, uncompressedPixels);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Check that the GL_RGBA format reports an error.
    glCompressedCopyTextureCHROMIUM(mTextures[0], mTextures[1]);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test validation of texture IDs
TEST_P(CopyCompressedTextureTest, InvalidTextureIds)
{
    if (!extensionEnabled("GL_CHROMIUM_copy_compressed_texture"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_copy_compressed_texture is not available."
                  << std::endl;
        return;
    }
    ASSERT_NE(nullptr, glCompressedCopyTextureCHROMIUM);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glCompressedCopyTextureCHROMIUM(mTextures[0], 99993);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedCopyTextureCHROMIUM(99994, mTextures[1]);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedCopyTextureCHROMIUM(99995, 99996);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedCopyTextureCHROMIUM(mTextures[0], mTextures[1]);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(CopyCompressedTextureTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES2_OPENGLES());

}  // namespace angle
