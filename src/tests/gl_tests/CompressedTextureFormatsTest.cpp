//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompressedTextureFormatsTest:
//   Tests that only the appropriate entry points are affected after
//   enabling compressed texture extensions.
//

#include "libANGLE/capture/gl_enum_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class CompressedTextureFormatsTest : public ANGLETestBase, public testing::Test
{
  public:
    CompressedTextureFormatsTest(const PlatformParameters &params,
                                 const std::string ext1,
                                 const std::string ext2,
                                 const GLenum format,
                                 const GLsizei size,
                                 const bool supportsUpdates,
                                 const bool supports2DArray,
                                 const bool supports3D,
                                 const bool alwaysOnES3)
        : ANGLETestBase(params),
          mExtNames({ext1, ext2}),
          mFormat(format),
          mSize(size),
          mSupportsUpdates(supportsUpdates),
          mSupports2DArray(supports2DArray),
          mSupports3D(supports3D),
          mAlwaysOnES3(alwaysOnES3)
    {
        setExtensionsEnabled(false);
    }

    void SetUp() final
    {
        ANGLETestBase::ANGLETestSetUp();

        // Older Metal versions do not support compressed TEXTURE_3D.
        mDisableTexture3D = IsMetal() && !IsMetalCompressedTexture3DAvailable();
    }

    void TearDown() final { ANGLETestBase::ANGLETestTearDown(); }

    void check2D(const bool extensionEnabled)
    {
        GLubyte data[32];

        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(GL_TEXTURE_2D, 0, mFormat, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(getClientMajorVersion() >= 3 ? GL_INVALID_OPERATION : GL_INVALID_VALUE);

            glCopyTexImage2D(GL_TEXTURE_2D, 0, mFormat, 0, 0, 4, 4, 0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glCompressedTexImage2D(GL_TEXTURE_2D, 0, mFormat, 4, 4, 0, mSize, nullptr);
            if (extensionEnabled)
            {
                EXPECT_GL_NO_ERROR();
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, mFormat, mSize, data);
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

            glTexStorage2D(GL_TEXTURE_2D, 1, mFormat, 4, 4);
            if (extensionEnabled)
            {
                EXPECT_GL_NO_ERROR();
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);

                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, mFormat, mSize, data);
                EXPECT_GL_ERROR(mSupportsUpdates ? GL_NO_ERROR : GL_INVALID_OPERATION);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }
    }

    void check3D(GLenum target, const bool extensionEnabled, const bool supportsTarget)
    {
        GLubyte data[32];

        {
            GLTexture texture;
            glBindTexture(target, texture);

            glTexImage3D(target, 0, mFormat, 4, 4, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glCompressedTexImage3D(target, 0, mFormat, 4, 4, 1, 0, mSize, nullptr);
            if (extensionEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();

                    glCompressedTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, mFormat, mSize, data);
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

            glTexStorage3D(target, 1, mFormat, 4, 4, 1);
            if (extensionEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();

                    glCompressedTexSubImage3D(target, 0, 0, 0, 0, 4, 4, 1, mFormat, mSize, data);
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

    void TestBody() override
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
        check2D(extensionEnabled);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, extensionEnabled, mSupports2DArray);
            check3D(GL_TEXTURE_3D, extensionEnabled, mSupports3D && !mDisableTexture3D);
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
        check2D(true);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, true, mSupports2DArray);
            check3D(GL_TEXTURE_3D, true, mSupports3D && !mDisableTexture3D);
        }
    }

  private:
    bool mDisableTexture3D = false;
    const std::vector<std::string> mExtNames;
    const GLenum mFormat;
    const GLsizei mSize;
    const bool mSupportsUpdates;
    const bool mSupports2DArray;
    const bool mSupports3D;
    const bool mAlwaysOnES3;
};

template <char const *ext1,
          char const *ext2,
          bool supports_updates,
          bool supports_2d_array,
          bool supports_3d,
          bool always_on_es3>
class _Test : public CompressedTextureFormatsTest
{
  public:
    _Test(const PlatformParameters &params, const GLenum format, const GLsizei size)
        : CompressedTextureFormatsTest(params,
                                       ext1,
                                       ext2,
                                       format,
                                       size,
                                       supports_updates,
                                       supports_2d_array,
                                       supports_3d,
                                       always_on_es3)
    {}
};

const char kDXT1[]     = "GL_EXT_texture_compression_dxt1";
const char kDXT3[]     = "GL_ANGLE_texture_compression_dxt3";
const char kDXT5[]     = "GL_ANGLE_texture_compression_dxt5";
const char kS3TCSRGB[] = "GL_EXT_texture_compression_s3tc_srgb";
const char kRGTC[]     = "GL_EXT_texture_compression_rgtc";
const char kBPTC[]     = "GL_EXT_texture_compression_bptc";

const char kETC1[]    = "GL_OES_compressed_ETC1_RGB8_texture";
const char kETC1Sub[] = "GL_EXT_compressed_ETC1_RGB8_sub_texture";

const char kEACR11U[]  = "GL_OES_compressed_EAC_R11_unsigned_texture";
const char kEACR11S[]  = "GL_OES_compressed_EAC_R11_signed_texture";
const char kEACRG11U[] = "GL_OES_compressed_EAC_RG11_unsigned_texture";
const char kEACRG11S[] = "GL_OES_compressed_EAC_RG11_signed_texture";

const char kETC2RGB8[]       = "GL_OES_compressed_ETC2_RGB8_texture";
const char kETC2RGB8SRGB[]   = "GL_OES_compressed_ETC2_sRGB8_texture";
const char kETC2RGB8A1[]     = "GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture";
const char kETC2RGB8A1SRGB[] = "GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture";
const char kETC2RGBA8[]      = "GL_OES_compressed_ETC2_RGBA8_texture";
const char kETC2RGBA8SRGB[]  = "GL_OES_compressed_ETC2_sRGB8_alpha8_texture";

const char kASTC[]         = "GL_KHR_texture_compression_astc_ldr";
const char kASTCSliced3D[] = "GL_KHR_texture_compression_astc_sliced_3d";

const char kPVRTC1[]    = "GL_IMG_texture_compression_pvrtc";
const char kPVRTCSRGB[] = "GL_EXT_pvrtc_sRGB";

const char kEmpty[] = "";

// clang-format off
using _DXT1Test     = _Test<kDXT1,     kEmpty, true, true, false, false>;
using _DXT3Test     = _Test<kDXT3,     kEmpty, true, true, false, false>;
using _DXT5Test     = _Test<kDXT5,     kEmpty, true, true, false, false>;
using _S3TCSRGBTest = _Test<kS3TCSRGB, kEmpty, true, true, false, false>;
using _RGTCTest     = _Test<kRGTC,     kEmpty, true, true, false, false>;
using _BPTCTest     = _Test<kBPTC,     kEmpty, true, true, true,  false>;

using _ETC1Test    = _Test<kETC1, kEmpty,   false, false, false, false>;
using _ETC1SubTest = _Test<kETC1, kETC1Sub, true,  false, false, false>;

using _EACR11UTest  = _Test<kEACR11U,  kEmpty, true, true, false, true>;
using _EACR11STest  = _Test<kEACR11S,  kEmpty, true, true, false, true>;
using _EACRG11UTest = _Test<kEACRG11U, kEmpty, true, true, false, true>;
using _EACRG11STest = _Test<kEACRG11S, kEmpty, true, true, false, true>;

using _ETC2RGB8Test       = _Test<kETC2RGB8,       kEmpty, true, true, false, true>;
using _ETC2RGB8SRGBTest   = _Test<kETC2RGB8SRGB,   kEmpty, true, true, false, true>;
using _ETC2RGB8A1Test     = _Test<kETC2RGB8A1,     kEmpty, true, true, false, true>;
using _ETC2RGB8A1SRGBTest = _Test<kETC2RGB8A1SRGB, kEmpty, true, true, false, true>;
using _ETC2RGBA8Test      = _Test<kETC2RGBA8,      kEmpty, true, true, false, true>;
using _ETC2RGBA8SRGBTest  = _Test<kETC2RGBA8SRGB,  kEmpty, true, true, false, true>;

using _ASTCTest         = _Test<kASTC, kEmpty,        true, true, false, false>;
using _ASTCSliced3DTest = _Test<kASTC, kASTCSliced3D, true, true, true,  false>;

using _PVRTC1Test     = _Test<kPVRTC1, kEmpty,     true, false, false, false>;
using _PVRTC1SRGBTest = _Test<kPVRTC1, kPVRTCSRGB, true, false, false, false>;
// clang-format on

const std::string GetTestName(const GLenum format, const PlatformParameters &params)
{
    std::string name = gl::GLinternalFormatToString(format);
    name.erase(0, 3);  // Remove GL_
    if (name.find("COMPRESSED_") == 0)
    {
        name.erase(0, 11);
    }
    for (std::string str : {"_KHR", "_EXT", "_IMG"})
    {
        if (name.find(str) != std::string::npos)
        {
            name.erase(name.length() - 4, 4);
        }
    }
    std::stringstream nameStr;
    nameStr << name << "/" << params;
    return nameStr.str();
}

using FormatDesc = std::pair<GLenum, GLsizei>;

void Run(const std::string name,
         const std::vector<FormatDesc> &formats,
         const PlatformParameters &params,
         std::function<CompressedTextureFormatsTest *(GLenum format, GLsizei size)> factory)
{
    for (FormatDesc pair : formats)
    {
        testing::RegisterTest(("CompressedTexture" + name + "Test").c_str(),
                              GetTestName(pair.first, params).c_str(), nullptr, nullptr, __FILE__,
                              __LINE__, [factory, pair]() -> CompressedTextureFormatsTest * {
                                  return factory(pair.first, pair.second);
                              });
    }
}
}  // namespace

void RegisterCompressedTextureFormatsTests()
{
    std::vector<PlatformParameters> platforms = {ANGLE_ALL_TEST_PLATFORMS_ES2,
                                                 ANGLE_ALL_TEST_PLATFORMS_ES3};

    const std::vector<FormatDesc> kDXT1 = {
        {GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 8},
        {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8},
    };

    const std::vector<FormatDesc> kDXT3 = {{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 16}};

    const std::vector<FormatDesc> kDXT5 = {{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16}};

    const std::vector<FormatDesc> kS3TCSRGB = {{GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 8},
                                               {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 8},
                                               {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 16},
                                               {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 16}};

    const std::vector<FormatDesc> kRGTC = {{GL_COMPRESSED_RED_RGTC1_EXT, 8},
                                           {GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 8},
                                           {GL_COMPRESSED_RED_GREEN_RGTC2_EXT, 16},
                                           {GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 16}};

    const std::vector<FormatDesc> kBPTC = {{GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 16},
                                           {GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, 16},
                                           {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 16},
                                           {GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 16}};

    const std::vector<FormatDesc> kETC1 = {{GL_ETC1_RGB8_OES, 8}};

    const std::vector<FormatDesc> kEACR11U      = {{GL_COMPRESSED_R11_EAC, 8}};
    const std::vector<FormatDesc> kEACR11S      = {{GL_COMPRESSED_SIGNED_R11_EAC, 8}};
    const std::vector<FormatDesc> kEACRG11U     = {{GL_COMPRESSED_RG11_EAC, 16}};
    const std::vector<FormatDesc> kEACRG11S     = {{GL_COMPRESSED_SIGNED_RG11_EAC, 16}};
    const std::vector<FormatDesc> kETC2RGB8     = {{GL_COMPRESSED_RGB8_ETC2, 8}};
    const std::vector<FormatDesc> kETC2RGB8SRGB = {{GL_COMPRESSED_SRGB8_ETC2, 8}};
    const std::vector<FormatDesc> kETC2RGB8A1 = {{GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8}};
    const std::vector<FormatDesc> kETC2RGB8A1SRGB = {
        {GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8}};
    const std::vector<FormatDesc> kETC2RGBA8     = {{GL_COMPRESSED_RGBA8_ETC2_EAC, 16}};
    const std::vector<FormatDesc> kETC2RGBA8SRGB = {{GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 16}};

    const std::vector<FormatDesc> kASTC = {{GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 16},
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
                                           {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16}};

    const std::vector<FormatDesc> kPVRTC1 = {{GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 32},
                                             {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 32},
                                             {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 32},
                                             {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 32}};

    const std::vector<FormatDesc> kPVRTC1SRGB = {{GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, 32},
                                                 {GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, 32},
                                                 {GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, 32},
                                                 {GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, 32}};

    for (PlatformParameters &p : platforms)
    {
        if (!IsPlatformAvailable(p))
            continue;

        using CTF = CompressedTextureFormatsTest;
        // clang-format off
        Run("DXT1",     kDXT1,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _DXT1Test     (p, f, s); });
        Run("DXT3",     kDXT3,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _DXT3Test     (p, f, s); });
        Run("DXT5",     kDXT5,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _DXT5Test     (p, f, s); });
        Run("S3TCSRGB", kS3TCSRGB, p, [p](GLenum f, GLsizei s) -> CTF * { return new _S3TCSRGBTest (p, f, s); });
        Run("RGTC",     kRGTC,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _RGTCTest     (p, f, s); });
        Run("BPTC",     kBPTC,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _BPTCTest     (p, f, s); });

        Run("ETC1",    kETC1, p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC1Test   (p, f, s); });
        Run("ETC1Sub", kETC1, p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC1SubTest(p, f, s); });

        Run("EACR11U",  kEACR11U,  p, [p](GLenum f, GLsizei s) -> CTF * { return new _EACR11UTest  (p, f, s); });
        Run("EACR11S",  kEACR11S,  p, [p](GLenum f, GLsizei s) -> CTF * { return new _EACR11STest  (p, f, s); });
        Run("EACRG11U", kEACRG11U, p, [p](GLenum f, GLsizei s) -> CTF * { return new _EACRG11UTest (p, f, s); });
        Run("EACRG11S", kEACRG11S, p, [p](GLenum f, GLsizei s) -> CTF * { return new _EACRG11STest (p, f, s); });

        Run("ETC2RGB8",       kETC2RGB8,       p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGB8Test      (p, f, s); });
        Run("ETC2RGB8SRGB",   kETC2RGB8SRGB,   p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGB8SRGBTest  (p, f, s); });
        Run("ETC2RGB8A1",     kETC2RGB8A1,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGB8A1Test    (p, f, s); });
        Run("ETC2RGB8A1SRGB", kETC2RGB8A1SRGB, p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGB8A1SRGBTest(p, f, s); });
        Run("ETC2RGBA8",      kETC2RGBA8,      p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGBA8Test     (p, f, s); });
        Run("ETC2RGBA8SRGB",  kETC2RGBA8SRGB,  p, [p](GLenum f, GLsizei s) -> CTF * { return new _ETC2RGBA8SRGBTest (p, f, s); });

        Run("ASTC",         kASTC, p, [p](GLenum f, GLsizei s) -> CTF * { return new _ASTCTest        (p, f, s); });
        Run("ASTCSliced3D", kASTC, p, [p](GLenum f, GLsizei s) -> CTF * { return new _ASTCSliced3DTest(p, f, s); });

        Run("PVRTC1",     kPVRTC1,     p, [p](GLenum f, GLsizei s) -> CTF * { return new _PVRTC1Test    (p, f, s); });
        Run("PVRTC1SRGB", kPVRTC1SRGB, p, [p](GLenum f, GLsizei s) -> CTF * { return new _PVRTC1SRGBTest(p, f, s); });
        // clang-format on
    }
}
