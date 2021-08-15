//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompressedTextureEntrypointsTest:
//   Tests that only the appropriate entry points are affected after
//   enabling compressed texture extensions.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class CompressedTextureEntrypointsTest : public ANGLETest
{
  protected:
    CompressedTextureEntrypointsTest() { setExtensionsEnabled(false); }

    void check2D(GLenum internalformat, const GLsizei blockSize, const bool extensionEnabled)
    {
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         nullptr);
            EXPECT_GL_ERROR(getClientMajorVersion() >= 3 ? GL_INVALID_OPERATION : GL_INVALID_VALUE);

            glCopyTexImage2D(GL_TEXTURE_2D, 0, internalformat, 0, 0, 4, 4, 0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalformat, 4, 4, 0, blockSize, nullptr);
            if (extensionEnabled)
            {
                EXPECT_GL_NO_ERROR();
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }

        if (getClientMajorVersion() >= 3)
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexStorage2D(GL_TEXTURE_2D, 1, internalformat, 4, 4);
            if (extensionEnabled)
            {
                EXPECT_GL_NO_ERROR();
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }
    }

    void check3D(GLenum target,
                 GLenum internalformat,
                 const GLsizei blockSize,
                 const bool extensionEnabled,
                 const bool supportsTarget)
    {
        {
            GLTexture texture;
            glBindTexture(target, texture);

            glTexImage3D(target, 0, internalformat, 4, 4, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glCompressedTexImage3D(target, 0, internalformat, 4, 4, 1, 0, blockSize, nullptr);
            if (extensionEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();
                    glTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                    nullptr);
                }
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }

        {
            GLTexture texture;
            glBindTexture(target, texture);

            glTexStorage3D(target, 1, internalformat, 4, 4, 1);
            if (extensionEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();
                    glTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                    nullptr);
                }
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }
    }

    void testRun(const std::vector<std::pair<GLenum, GLsizei>> formats,
                 const std::string &extName,
                 const bool supports2DArray = true,
                 const bool supports3D      = false,
                 const bool alwaysOnES3     = false)
    {
        testRun(formats, std::vector<std::string>({extName}), supports2DArray, supports3D,
                alwaysOnES3);
    }

    void testRun(const std::vector<std::pair<GLenum, GLsizei>> formats,
                 const std::vector<std::string> &extNames,
                 const bool supports2DArray = true,
                 const bool supports3D      = false,
                 const bool alwaysOnES3     = false)
    {

        const bool alwaysOn = alwaysOnES3 && getClientMajorVersion() >= 3;
        {
            for (const std::pair<GLenum, GLsizei> &format : formats)
            {
                check2D(format.first, format.second, alwaysOn);
                if (getClientMajorVersion() >= 3)
                {
                    check3D(GL_TEXTURE_2D_ARRAY, format.first, format.second, alwaysOn,
                            supports2DArray);
                    check3D(GL_TEXTURE_3D, format.first, format.second, alwaysOn, supports3D);
                }
            }
        }

        for (const std::string &extName : extNames)
        {
            if (IsGLExtensionRequestable(extName))
            {
                glRequestExtensionANGLE(extName.c_str());
            }
            ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extName));
        }

        for (const std::pair<GLenum, GLsizei> &format : formats)
        {
            check2D(format.first, format.second, true);

            if (getClientMajorVersion() >= 3)
            {
                check3D(GL_TEXTURE_2D_ARRAY, format.first, format.second, true, supports2DArray);
                check3D(GL_TEXTURE_3D, format.first, format.second, true, supports3D);
            }
        }
    }
};

// Tests texImage* with DXT1 format
TEST_P(CompressedTextureEntrypointsTest, S3TC_DXT1_RGB)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 8},
                                                             {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8}};
    testRun(formats, "GL_EXT_texture_compression_dxt1");
}

// Tests texImage* with DXT3
TEST_P(CompressedTextureEntrypointsTest, S3TC_DXT3)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 16}};
    testRun(formats, "GL_ANGLE_texture_compression_dxt3");
}

// Tests texImage* with DXT5
TEST_P(CompressedTextureEntrypointsTest, S3TC_DXT5)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16}};
    testRun(formats, "GL_ANGLE_texture_compression_dxt5");
}

// Tests texImage* with S3TC sRGB
TEST_P(CompressedTextureEntrypointsTest, S3TC_SRGB)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 8},
        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 8},
        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 16},
        {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 16}};
    testRun(formats, "GL_EXT_texture_compression_s3tc_srgb");
}

// Tests texImage* with RGTC
TEST_P(CompressedTextureEntrypointsTest, RGTC)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RED_RGTC1_EXT, 8},
        {GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 8},
        {GL_COMPRESSED_RED_GREEN_RGTC2_EXT, 16},
        {GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 16}};
    testRun(formats, "GL_EXT_texture_compression_rgtc");
}

// Tests texImage* with BPTC
TEST_P(CompressedTextureEntrypointsTest, BPTC)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 16},
        {GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, 16},
        {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 16},
        {GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 16}};

    const bool supports3D = !IsMetal() || IsMetalCompressedTexture3DAvailable();
    testRun(formats, "GL_EXT_texture_compression_bptc", true, supports3D);
}

// Tests texImage* with ETC1
TEST_P(CompressedTextureEntrypointsTest, ETC1)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_ETC1_RGB8_OES, 8}};
    testRun(formats, "GL_OES_compressed_ETC1_RGB8_texture", false, false);
}

// Tests texImage* with EAC R11U
TEST_P(CompressedTextureEntrypointsTest, EAC_R11U)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_EAC_R11_unsigned_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_R11_EAC, 8}};
    testRun(formats, "GL_OES_compressed_EAC_R11_unsigned_texture", true, false, true);
}

// Tests texImage* with EAC R11S
TEST_P(CompressedTextureEntrypointsTest, EAC_R11S)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_EAC_R11_signed_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_SIGNED_R11_EAC, 8}};
    testRun(formats, "GL_OES_compressed_EAC_R11_signed_texture", true, false, true);
}

// Tests texImage* with EAC RG11U
TEST_P(CompressedTextureEntrypointsTest, EAC_RG11U)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_EAC_RG11_unsigned_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_RG11_EAC, 16}};
    testRun(formats, "GL_OES_compressed_EAC_RG11_unsigned_texture", true, false, true);
}

// Tests texImage* with EAC RG11S
TEST_P(CompressedTextureEntrypointsTest, EAC_RG11S)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_EAC_RG11_signed_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_SIGNED_RG11_EAC, 16}};
    testRun(formats, "GL_OES_compressed_EAC_RG11_signed_texture", true, false, true);
}

// Tests texImage* with ETC2 RGB8
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGB8)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_ETC2_RGB8_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_RGB8_ETC2, 8}};
    testRun(formats, "GL_OES_compressed_ETC2_RGB8_texture", true, false, true);
}

// Tests texImage* with ETC2 RGB8
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGB8_SRGB)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_ETC2_sRGB8_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_SRGB8_ETC2, 8}};
    testRun(formats, "GL_OES_compressed_ETC2_sRGB8_texture", true, false, true);
}

// Tests texImage* with ETC2 RGB8A1
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGB8A1)
{
    ANGLE_SKIP_TEST_IF(
        getClientMajorVersion() >= 3 &&
        !IsGLExtensionRequestable("GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8}};
    testRun(formats, "GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture", true, false, true);
}

// Tests texImage* with ETC2 RGB8A1 SRGB
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGB8A1_SRGB)
{
    ANGLE_SKIP_TEST_IF(
        getClientMajorVersion() >= 3 &&
        !IsGLExtensionRequestable("GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8}};
    testRun(formats, "GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture", true, false, true);
}

// Tests texImage* with ETC2 RGBA8
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGBA8)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_ETC2_RGBA8_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {{GL_COMPRESSED_RGBA8_ETC2_EAC, 16}};
    testRun(formats, "GL_OES_compressed_ETC2_RGBA8_texture", true, false, true);
}

// Tests texImage* with ETC2 RGBA8
TEST_P(CompressedTextureEntrypointsTest, ETC2_RGBA8_SRGB)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                       !IsGLExtensionRequestable("GL_OES_compressed_ETC2_sRGB8_alpha8_texture"));
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 16}};
    testRun(formats, "GL_OES_compressed_ETC2_sRGB8_alpha8_texture", true, false, true);
}

// Tests texImage* with ASTC
TEST_P(CompressedTextureEntrypointsTest, ASTC)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16},
    };
    testRun(formats, "GL_KHR_texture_compression_astc_ldr");
}

// Tests texImage* with ASTC SLICED 3D
TEST_P(CompressedTextureEntrypointsTest, ASTC_SLICED_3D)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 16},
        {GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 16},
        {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16},
    };

    const std::vector<std::string> extNames = {"GL_KHR_texture_compression_astc_ldr",
                                               "GL_KHR_texture_compression_astc_sliced_3d"};
    testRun(formats, extNames, true, true);
}

// Tests texImage* with PVRTC1
TEST_P(CompressedTextureEntrypointsTest, PVRTC1)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 32},
        {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 32},
        {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 32},
        {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 32}};
    testRun(formats, "GL_IMG_texture_compression_pvrtc", false, false);
}

// Tests texImage* with PVRTC1 SRGB
TEST_P(CompressedTextureEntrypointsTest, PVRTC1_SRGB)
{
    const std::vector<std::pair<GLenum, GLsizei>> formats = {
        {GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, 32},
        {GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, 32},
        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, 32},
        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, 32}};
    const std::vector<std::string> extNames = {"GL_IMG_texture_compression_pvrtc",
                                               "GL_EXT_pvrtc_sRGB"};
    testRun(formats, extNames, false, false);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureEntrypointsTest);
}  // anonymous namespace
