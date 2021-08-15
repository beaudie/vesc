//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompressedTextureFormatsTest:
//   Tests that only the appropriate entry points are affected after
//   enabling compressed texture extensions.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class CompressedTextureFormatsTest : public ANGLETest
{
  public:
    CompressedTextureFormatsTest(const std::vector<std::string> extNames,
                                 const bool supportsUpdates,
                                 const bool supports2DArray,
                                 const bool supports3D,
                                 const bool alwaysOnES3)
        : mExtNames(extNames),
          mSupportsUpdates(supportsUpdates),
          mSupports2DArray(supports2DArray),
          mSupports3D(supports3D),
          mAlwaysOnES3(alwaysOnES3)
    {
        setExtensionsEnabled(false);
    }

    void testSetUp() override
    {
        // Older Metal versions do not support compressed TEXTURE_3D.
        mDisableTexture3D = IsMetal() && !IsMetalCompressedTexture3DAvailable();
    }

    void check2D(GLenum internalformat, const GLsizei blockSize, const bool extensionEnabled)
    {
        GLubyte data[32];

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

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, internalformat, blockSize,
                                          data);
                EXPECT_GL_ERROR(mSupportsUpdates ? GL_NO_ERROR : GL_INVALID_OPERATION);
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

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, internalformat, blockSize,
                                          data);
                EXPECT_GL_ERROR(mSupportsUpdates ? GL_NO_ERROR : GL_INVALID_OPERATION);
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
        GLubyte data[32];

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

                    glCompressedTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, internalformat,
                                              blockSize, data);
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

                    glCompressedTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, internalformat,
                                              blockSize, data);
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

    void testRun(GLenum format, GLsizei size)
    {
        // ETC2/EAC formats always pass validation on ES3 contexts but in some cases fail in drivers
        // because their emulation is not implemented for OpenGL renderer.
        // https://crbug.com/angleproject/6300
        if (mAlwaysOnES3)
        {
            ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                               !IsGLExtensionRequestable(mExtNames[0]));
        }

        // It's not possible to disable ETC2/EAC support on ES 3.0.
        const bool extensionEnabled = mAlwaysOnES3 && getClientMajorVersion() >= 3;
        check2D(format, size, extensionEnabled);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, format, size, extensionEnabled, mSupports2DArray);
            check3D(GL_TEXTURE_3D, format, size, extensionEnabled,
                    mSupports3D && !mDisableTexture3D);
        }

        for (const std::string &extName : mExtNames)
        {
            if (IsGLExtensionRequestable(extName))
            {
                glRequestExtensionANGLE(extName.c_str());
            }
            ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extName));
        }

        // Repeat all checks after enabling the extensions.
        check2D(format, size, true);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, format, size, true, mSupports2DArray);
            check3D(GL_TEXTURE_3D, format, size, true, mSupports3D && !mDisableTexture3D);
        }
    }

    const std::vector<std::string> mExtNames;
    const bool mSupportsUpdates;
    const bool mSupports2DArray;
    const bool mSupports3D;
    const bool mAlwaysOnES3;

  private:
    bool mDisableTexture3D = false;
};

#define TEST_CLASS(class_name, extensions, supports_updates, supports_2d_array, supports_3d, \
                   always_on_es3)                                                            \
    class CompressedTexture##class_name##Test : public CompressedTextureFormatsTest          \
    {                                                                                        \
      protected:                                                                             \
        CompressedTexture##class_name##Test()                                                \
            : CompressedTextureFormatsTest(extensions,                                       \
                                           supports_updates,                                 \
                                           supports_2d_array,                                \
                                           supports_3d,                                      \
                                           always_on_es3)                                    \
        {}                                                                                   \
    };

#define EXT(...) __VA_ARGS__

// clang-format off
TEST_CLASS(DXT1,     EXT({"GL_EXT_texture_compression_dxt1"}),      true, true, false, false)
TEST_CLASS(DXT3,     EXT({"GL_ANGLE_texture_compression_dxt3"}),    true, true, false, false)
TEST_CLASS(DXT5,     EXT({"GL_ANGLE_texture_compression_dxt5"}),    true, true, false, false)
TEST_CLASS(S3TCSRGB, EXT({"GL_EXT_texture_compression_s3tc_srgb"}), true, true, false, false)
TEST_CLASS(RGTC,     EXT({"GL_EXT_texture_compression_rgtc"}),      true, true, false, false)
TEST_CLASS(BPTC,     EXT({"GL_EXT_texture_compression_bptc"}),      true, true, true,  false)

TEST_CLASS(ETC1,         EXT({"GL_OES_compressed_ETC1_RGB8_texture"}),                                            false, false, false, false)
TEST_CLASS(ETC1SubImage, EXT({"GL_OES_compressed_ETC1_RGB8_texture", "GL_EXT_compressed_ETC1_RGB8_sub_texture"}), true,  false, false, false)

TEST_CLASS(EACR11U,  EXT({"GL_OES_compressed_EAC_R11_unsigned_texture"}),  true, true, false, true)
TEST_CLASS(EACR11S,  EXT({"GL_OES_compressed_EAC_R11_signed_texture"}),    true, true, false, true)
TEST_CLASS(EACRG11U, EXT({"GL_OES_compressed_EAC_RG11_unsigned_texture"}), true, true, false, true)
TEST_CLASS(EACRG11S, EXT({"GL_OES_compressed_EAC_RG11_signed_texture"}),   true, true, false, true)

TEST_CLASS(ETC2RGB8,       EXT({"GL_OES_compressed_ETC2_RGB8_texture"}),                      true, true, false, true)
TEST_CLASS(ETC2RGB8SRGB,   EXT({"GL_OES_compressed_ETC2_sRGB8_texture"}),                     true, true, false, true)
TEST_CLASS(ETC2RGB8A1,     EXT({"GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture"}),       true, true, false, true)
TEST_CLASS(ETC2RGB8A1SRGB, EXT({"GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture"}), true, true, false, true)
TEST_CLASS(ETC2RGBA8,      EXT({"GL_OES_compressed_ETC2_RGBA8_texture"}),                     true, true, false, true)
TEST_CLASS(ETC2RGBA8SRGB,  EXT({"GL_OES_compressed_ETC2_sRGB8_alpha8_texture"}),              true, true, false, true)

TEST_CLASS(ASTC,         EXT({"GL_KHR_texture_compression_astc_ldr"}),                                              true, true, false, false)
TEST_CLASS(ASTCSliced3D, EXT({"GL_KHR_texture_compression_astc_ldr", "GL_KHR_texture_compression_astc_sliced_3d"}), true, true, true,  false)

TEST_CLASS(PVRTC1,     EXT({"GL_IMG_texture_compression_pvrtc"}),                      true, false, false, false)
TEST_CLASS(PVRTC1SRGB, EXT({"GL_IMG_texture_compression_pvrtc", "GL_EXT_pvrtc_sRGB"}), true, false, false, false)

#define TEST_CASE(class, name, enum, size) \
    TEST_P(class, name) { testRun(enum, size); }

TEST_CASE(CompressedTextureDXT1Test, RGB_S3TC_DXT1,  GL_COMPRESSED_RGB_S3TC_DXT1_EXT,   8)
TEST_CASE(CompressedTextureDXT1Test, RGBA_S3TC_DXT1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,  8)
TEST_CASE(CompressedTextureDXT3Test, RGBA_S3TC_DXT3, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 16)
TEST_CASE(CompressedTextureDXT5Test, RGBA_S3TC_DXT5, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16)

TEST_CASE(CompressedTextureS3TCSRGBTest, SRGB_S3TC_DXT1,       GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,        8)
TEST_CASE(CompressedTextureS3TCSRGBTest, SRGB_ALPHA_S3TC_DXT1, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  8)
TEST_CASE(CompressedTextureS3TCSRGBTest, SRGB_ALPHA_S3TC_DXT3, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 16)
TEST_CASE(CompressedTextureS3TCSRGBTest, SRGB_ALPHA_S3TC_DXT5, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 16)

TEST_CASE(CompressedTextureRGTCTest, RED_RGTC1,              GL_COMPRESSED_RED_RGTC1_EXT,               8)
TEST_CASE(CompressedTextureRGTCTest, SIGNED_RED_RGTC1,       GL_COMPRESSED_SIGNED_RED_RGTC1_EXT,        8)
TEST_CASE(CompressedTextureRGTCTest, RED_GREEN_RGTC2,        GL_COMPRESSED_RED_GREEN_RGTC2_EXT,        16)
TEST_CASE(CompressedTextureRGTCTest, SIGNED_RED_GREEN_RGTC2, GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 16)

TEST_CASE(CompressedTextureBPTCTest, RGBA_BPTC_UNORM,         GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,         16)
TEST_CASE(CompressedTextureBPTCTest, SRGB_ALPHA_BPTC_UNORM,   GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,   16)
TEST_CASE(CompressedTextureBPTCTest, RGB_BPTC_SIGNED_FLOAT,   GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT,   16)
TEST_CASE(CompressedTextureBPTCTest, RGB_BPTC_UNSIGNED_FLOAT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 16)

TEST_CASE(CompressedTextureETC1Test,         ETC1_RGB8, GL_ETC1_RGB8_OES, 8)
TEST_CASE(CompressedTextureETC1SubImageTest, ETC1_RGB8, GL_ETC1_RGB8_OES, 8)

TEST_CASE(CompressedTextureEACR11UTest,  R11_EAC,         GL_COMPRESSED_R11_EAC,          8)
TEST_CASE(CompressedTextureEACR11STest,  SIGNED_R11_EAC,  GL_COMPRESSED_SIGNED_R11_EAC,   8)
TEST_CASE(CompressedTextureEACRG11UTest, RG11_EAC,        GL_COMPRESSED_RG11_EAC,        16)
TEST_CASE(CompressedTextureEACRG11STest, SIGNED_RG11_EAC, GL_COMPRESSED_SIGNED_RG11_EAC, 16)

TEST_CASE(CompressedTextureETC2RGB8Test,       RGB8_ETC2,                      GL_COMPRESSED_RGB8_ETC2,                      8)
TEST_CASE(CompressedTextureETC2RGB8SRGBTest,   SRGB8_ETC2,                     GL_COMPRESSED_SRGB8_ETC2,                     8)
TEST_CASE(CompressedTextureETC2RGB8A1Test,     RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  8)
TEST_CASE(CompressedTextureETC2RGB8A1SRGBTest, SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8)
TEST_CASE(CompressedTextureETC2RGBA8Test,      RGBA8_ETC2_EAC,                 GL_COMPRESSED_RGBA8_ETC2_EAC,                16)
TEST_CASE(CompressedTextureETC2RGBA8SRGBTest,  SRGB8_ALPHA8_ETC2_EAC,          GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,         16)

TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_4x4,   GL_COMPRESSED_RGBA_ASTC_4x4_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_5x4,   GL_COMPRESSED_RGBA_ASTC_5x4_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_5x5,   GL_COMPRESSED_RGBA_ASTC_5x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_6x5,   GL_COMPRESSED_RGBA_ASTC_6x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_6x6,   GL_COMPRESSED_RGBA_ASTC_6x6_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_8x5,   GL_COMPRESSED_RGBA_ASTC_8x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_8x6,   GL_COMPRESSED_RGBA_ASTC_8x6_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_8x8,   GL_COMPRESSED_RGBA_ASTC_8x8_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_10x5,  GL_COMPRESSED_RGBA_ASTC_10x5_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_10x6,  GL_COMPRESSED_RGBA_ASTC_10x6_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_10x8,  GL_COMPRESSED_RGBA_ASTC_10x8_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_10x10, GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_12x10, GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 16)
TEST_CASE(CompressedTextureASTCTest, RGBA_ASTC_12x12, GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 16)

TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_4x4,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_5x4,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_5x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_6x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_6x6,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_8x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_8x6,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_8x8,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,   16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_10x5,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_10x6,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_10x8,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_10x10, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_12x10, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 16)
TEST_CASE(CompressedTextureASTCTest, SRGB8_ALPHA8_ASTC_12x12, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16)

TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_4x4,   GL_COMPRESSED_RGBA_ASTC_4x4_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_5x4,   GL_COMPRESSED_RGBA_ASTC_5x4_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_5x5,   GL_COMPRESSED_RGBA_ASTC_5x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_6x5,   GL_COMPRESSED_RGBA_ASTC_6x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_6x6,   GL_COMPRESSED_RGBA_ASTC_6x6_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_8x5,   GL_COMPRESSED_RGBA_ASTC_8x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_8x6,   GL_COMPRESSED_RGBA_ASTC_8x6_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_8x8,   GL_COMPRESSED_RGBA_ASTC_8x8_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_10x5,  GL_COMPRESSED_RGBA_ASTC_10x5_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_10x6,  GL_COMPRESSED_RGBA_ASTC_10x6_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_10x8,  GL_COMPRESSED_RGBA_ASTC_10x8_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_10x10, GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_12x10, GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 16)
TEST_CASE(CompressedTextureASTCSliced3DTest, RGBA_ASTC_12x12, GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 16)

TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_4x4,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_5x4,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_5x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_6x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_6x6,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_8x5,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_8x6,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_8x8,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,   16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_10x5,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_10x6,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_10x8,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_10x10, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_12x10, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 16)
TEST_CASE(CompressedTextureASTCSliced3DTest, SRGB8_ALPHA8_ASTC_12x12, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16)

TEST_CASE(CompressedTexturePVRTC1Test, RGB_PVRTC_4BPPV1,  GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,  32)
TEST_CASE(CompressedTexturePVRTC1Test, RGB_PVRTC_2BPPV1,  GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,  32)
TEST_CASE(CompressedTexturePVRTC1Test, RGBA_PVRTC_4BPPV1, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 32)
TEST_CASE(CompressedTexturePVRTC1Test, RGBA_PVRTC_2BPPV1, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 32)

TEST_CASE(CompressedTexturePVRTC1SRGBTest, SRGB_PVRTC_2BPPV1,       GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,       32)
TEST_CASE(CompressedTexturePVRTC1SRGBTest, SRGB_PVRTC_4BPPV1,       GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,       32)
TEST_CASE(CompressedTexturePVRTC1SRGBTest, SRGB_ALPHA_PVRTC_2BPPV1, GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, 32)
TEST_CASE(CompressedTexturePVRTC1SRGBTest, SRGB_ALPHA_PVRTC_4BPPV1, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, 32)
// clang-format on

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureDXT1Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureDXT3Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureDXT5Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureS3TCSRGBTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureRGTCTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureBPTCTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC1Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC1SubImageTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureEACR11UTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureEACR11STest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureEACRG11UTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureEACRG11STest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGB8Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGB8SRGBTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGB8A1Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGB8A1SRGBTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGBA8Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureETC2RGBA8SRGBTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureASTCTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTextureASTCSliced3DTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTexturePVRTC1Test);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CompressedTexturePVRTC1SRGBTest);
}  // anonymous namespace
