//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class TextureUploadFormatTest : public ANGLETest
{
};

struct TexFormat final
{
    GLenum internalFormat;
    GLenum unpackFormat;
    GLenum unpackType;

    TexFormat() = delete;

    uint8_t bytesPerPixel() const
    {
        uint8_t bytesPerChannel;
        switch (unpackType)
        {
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return 2;

            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT_24_8:
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
            case GL_UNSIGNED_INT_5_9_9_9_REV:
                return 4;

            case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                return 8;

            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                bytesPerChannel = 1;
                break;

            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_HALF_FLOAT:
            case GL_HALF_FLOAT_OES:
                bytesPerChannel = 2;
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_FLOAT:
                bytesPerChannel = 4;
                break;

            default:
                assert(false);
                return 0;
        }

        switch (unpackFormat)
        {
            case GL_RGBA:
            case GL_RGBA_INTEGER:
                return bytesPerChannel * 4;

            case GL_RGB:
            case GL_RGB_INTEGER:
                return bytesPerChannel * 3;

            case GL_RG:
            case GL_RG_INTEGER:
            case GL_LUMINANCE_ALPHA:
                return bytesPerChannel * 2;

            case GL_RED:
            case GL_RED_INTEGER:
            case GL_LUMINANCE:
            case GL_ALPHA:
            case GL_DEPTH_COMPONENT:
                return bytesPerChannel * 1;

            default:
                assert(false);
                return 0;
        }
    }
};

template <const uint8_t bits>
static uint32_t EncodeNormUint(const float val)
{
    const auto max = UINT32_MAX >> (32 - bits);
    return uint32_t(val * max + 0.5);  // round-half-up
}

template <const int signBit, const int eBits, const int mBits>
struct SizedFloat
{
    static const int kSignBit = signBit;
    static const int kEBits   = eBits;
    static const int kMBits   = mBits;

    static uint32_t Assemble(const uint32_t sVal, const uint32_t eVal, const uint32_t mVal)
    {
        return (signBit ? (sVal << (eBits + mBits)) : 0) | (eVal << mBits) | mVal;
    }

    static uint32_t Encode(const float signedV)
    {
        const float v = signBit ? fabsf(signedV) : std::max(0.0f, signedV);

        const int eBias   = (1 << (eBits - 1)) - 1;
        const int eValMax = (1 << eBits) - 1;

        const float eApprox = log2f(v);
        const auto eActual  = int(floorf(eApprox));

        int eVal = eBias + eActual;
        uint32_t mVal;
        if (v != v)
        {  // NaN
            eVal = eValMax;
            mVal = 1;
        }
        else if (eVal < 0)
        {  // underflow to zero
            eVal = 0;
            mVal = 0;
        }
        else if (eVal >= eValMax)
        {  // overfloat to Inf
            eVal = eValMax;
            mVal = 0;
        }
        else
        {
            float mFloat;
            if (eVal == 0)
            {  // denormal
                mFloat = v * powf(2, 1 - eBias);
            }
            else
            {  // standard range
                mFloat = v * powf(2, -eActual) - 1.0;
            }
            mVal = uint32_t(mFloat * (1 << mBits) + 0.5);
        }

        const auto sVal = uint32_t(v < 0.0f);
        return Assemble(sVal, eVal, mVal);
    }
};
typedef SizedFloat<1, 5, 10> Float16;
typedef SizedFloat<0, 5, 6> UFloat11;
typedef SizedFloat<0, 5, 5> UFloat10;

static uint32_t EncodeRGB9_E5_Rev(const float signedR, const float signedG, const float signedB)
{
    const float r       = std::max(0.0f, signedR);
    const float g       = std::max(0.0f, signedG);
    const float b       = std::max(0.0f, signedB);
    const int eBits     = 5;
    const int eBias     = (1 << (eBits - 1)) - 1;  // 15
    const int eMax      = (1 << eBits) - 1;
    const int mBits     = 9;
    const uint32_t mMax = (1 << mBits) - 1;
    // Maximize mVal for one channel
    // => Find the lowest viable exponent
    int minViableActualExp                    = 1 << eBits;
    const auto fnMinimizeViableActualExponent = [&](const float v) {
        const auto cur = int(ceil(log2f(v / mMax)));
        if (cur < minViableActualExp)
        {
            minViableActualExp = cur;
        }
    };
    fnMinimizeViableActualExponent(r);
    fnMinimizeViableActualExponent(g);
    fnMinimizeViableActualExponent(b);
    const int eVal = std::max(0, std::min(minViableActualExp + eBias + mBits, eMax));

    const auto fnM = [&](const float v) {
        const auto m = uint32_t(v * powf(2, mBits + eBias - eVal));
        return std::min(m, mMax);
    };

    const auto mR = fnM(r);
    const auto mG = fnM(g);
    const auto mB = fnM(b);
    return (mR << 0) | (mG << 9) | (mB << 18) | (eVal << 27);
}

TEST_P(TextureUploadFormatTest, WeirdFloats)
{
    EXPECT_EQ(Float16::Assemble(0, 0x0f, 0), Float16::Encode(1.0));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 1, 0), Float16::Encode(1.0 / 2));

    EXPECT_EQ(Float16::Assemble(0, 0x0f - 3, 0), Float16::Encode(1.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 2, 0), Float16::Encode(2.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 2, 1 << (Float16::kMBits - 1)), Float16::Encode(3.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 1, 1 << (Float16::kMBits - 2)), Float16::Encode(5.0 / 8));
}

template <typename DestT, typename SrcT, size_t SrcN>
static void ZeroAndCopy(DestT &dest, const SrcT (&src)[SrcN])
{
    dest.fill(0);
    memcpy(dest.data(), src, sizeof(SrcT) * SrcN);
}

static std::string EnumStr(const GLenum v)
{
#define _(X)     \
    case GL_##X: \
        return #X;
    switch (v)
    {
        _(UNSIGNED_SHORT_5_6_5)
        _(UNSIGNED_SHORT_4_4_4_4)
        _(UNSIGNED_SHORT_5_5_5_1)
        _(UNSIGNED_INT_2_10_10_10_REV)
        _(UNSIGNED_INT_24_8)
        _(UNSIGNED_INT_10F_11F_11F_REV)
        _(UNSIGNED_INT_5_9_9_9_REV)
        _(FLOAT_32_UNSIGNED_INT_24_8_REV)
        _(UNSIGNED_BYTE)
        _(BYTE)
        _(UNSIGNED_SHORT)
        _(SHORT)
        _(HALF_FLOAT)
        _(HALF_FLOAT_OES)
        _(UNSIGNED_INT)
        _(INT)
        _(FLOAT)
        _(RGBA)
        _(RGBA_INTEGER)
        _(RGB)
        _(RGB_INTEGER)
        _(RG)
        _(RG_INTEGER)
        _(LUMINANCE_ALPHA)
        _(RED)
        _(RED_INTEGER)
        _(LUMINANCE)
        _(ALPHA)
        _(DEPTH_COMPONENT)

        _(RGBA8)
        _(RGB8)
        _(RG8)
        _(R8)
        _(RGBA8_SNORM)
        _(RGB8_SNORM)
        _(RG8_SNORM)
        _(R8_SNORM)
        _(RGBA8I)
        _(RGB8I)
        _(RG8I)
        _(R8I)
        _(RGBA8UI)
        _(RGB8UI)
        _(RG8UI)
        _(R8UI)

        _(RGBA16F)
        _(RGB16F)
        _(RG16F)
        _(R16F)
        _(RGBA16I)
        _(RGB16I)
        _(RG16I)
        _(R16I)
        _(RGBA16UI)
        _(RGB16UI)
        _(RG16UI)
        _(R16UI)

        _(RGBA32F)
        _(RGB32F)
        _(RG32F)
        _(R32F)
        _(RGBA32I)
        _(RGB32I)
        _(RG32I)
        _(R32I)
        _(RGBA32UI)
        _(RGB32UI)
        _(RG32UI)
        _(R32UI)

        _(RGBA4)
        _(RGB565)
        _(RGB5_A1)
        _(RGB10_A2)
        _(RGB9_E5)
        _(R11F_G11F_B10F)
        _(RGB10_A2UI)

        _(DEPTH_COMPONENT16)
        _(DEPTH_COMPONENT24)
        _(DEPTH_COMPONENT32F)
        _(DEPTH24_STENCIL8)
        _(DEPTH32F_STENCIL8)

        default:
            assert(false);
            std::stringstream ret;
            ret << "0x" << std::hex << v;
            return ret.str();
    }
#undef _
}

TEST_P(TextureUploadFormatTest, All)
{
    const char kVertShader[] = R"(
        void main()
        {
            gl_PointSize = 1.0;
            gl_Position = vec4(0, 0, 0, 1);
        })";

    const char kFragShader_Floats[] = R"(
        precision mediump float;
        uniform sampler2D uTex;

        void main()
        {
            gl_FragColor = texture2D(uTex, vec2(0,0));
        })";
    const char kFragShader_Ints[]   = R"(
        precision mediump float;
        uniform sampler2D uTex;

        void main()
        {
            gl_FragColor = texture2D(uTex, vec2(0,0)) / 8.0;
        })";
    ANGLE_GL_PROGRAM(floatsProg, kVertShader, kFragShader_Floats);
    ANGLE_GL_PROGRAM(intsProg, kVertShader, kFragShader_Ints);

    auto uTex = glGetUniformLocation(floatsProg, "uTex");
    glUseProgram(floatsProg);
    glUniform1i(uTex, 0);

    uTex = glGetUniformLocation(intsProg, "uTex");
    glUseProgram(intsProg);
    glUniform1i(uTex, 0);

    glDisable(GL_DITHER);

    ASSERT_GL_NO_ERROR();

    // --

    GLRenderbuffer backbufferRB;
    glBindRenderbuffer(GL_RENDERBUFFER, backbufferRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    GLFramebuffer backbufferFB;
    glBindFramebuffer(GL_FRAMEBUFFER, backbufferFB);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, backbufferRB);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glViewport(0, 0, 1, 1);

    // --

    GLTexture testTex;
    glBindTexture(GL_TEXTURE_2D, testTex);

    ASSERT_GL_NO_ERROR();

    // --

    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    const bool hasSubrectUploads = !glGetError();

    const int srcIntVals[4] = {1, 2, 5, 3};
    const float srcVals[4]  = {
        srcIntVals[0] / 8.0, srcIntVals[1] / 8.0, srcIntVals[2] / 8.0, srcIntVals[3] / 8.0,
    };
    const uint8_t refVals[4] = {EncodeNormUint<8>(srcVals[0]), EncodeNormUint<8>(srcVals[1]),
                                EncodeNormUint<8>(srcVals[2]), EncodeNormUint<8>(srcVals[3])};

    // --

    const auto fnTestData = [&](const TexFormat &format, const void *const data, const GLColor &err,
                                const char *const info) {
        ASSERT_GL_NO_ERROR();
        glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, 1, 1, 0, format.unpackFormat,
                     format.unpackType, data);
        const auto uploadErr = glGetError();
        if (uploadErr)
            return;

        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);

        const auto actual = ReadColor(0, 0);

        GLColor expected;
        switch (format.unpackFormat)
        {
            case GL_RGBA:
            case GL_RGBA_INTEGER:
                expected = {refVals[0], refVals[1], refVals[2], refVals[3]};
                break;
            case GL_RGB:
                expected = {refVals[0], refVals[1], refVals[2], 255};
                break;
            case GL_RG:
                expected = {refVals[0], refVals[1], 0, 255};
                break;
            case GL_RED:
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_STENCIL:
                expected = {refVals[0], 0, 0, 255};
                break;

            case GL_RGB_INTEGER:
                expected = {refVals[0], refVals[1], refVals[2], refVals[0]};
                break;
            case GL_RG_INTEGER:
                expected = {refVals[0], refVals[1], 0, refVals[0]};
                break;
            case GL_RED_INTEGER:
                expected = {refVals[0], 0, 0, refVals[0]};
                break;

            case GL_LUMINANCE_ALPHA:
                expected = {refVals[0], refVals[0], refVals[0], refVals[1]};
                break;
            case GL_LUMINANCE:
                expected = {refVals[0], refVals[0], refVals[0], 255};
                break;
            case GL_ALPHA:
                expected = {0, 0, 0, refVals[0]};
                break;

            default:
                assert(false);
        }

        ASSERT_GL_NO_ERROR();
        auto result = actual.ExpectNear(expected, err);
        if (!result)
        {
            result << " [" << EnumStr(format.internalFormat) << "/" << EnumStr(format.unpackFormat)
                   << "/" << EnumStr(format.unpackType) << " " << info << "]";
        }
        EXPECT_TRUE(result);
    };

    // --

    std::array<uint8_t, sizeof(float) * 4> srcBuffer;

    std::array<uint8_t, srcBuffer.size() * 2> subrectBuffer;
    const auto fnTest = [&](const TexFormat &format, const GLColor &err) {
        fnTestData(format, srcBuffer.data(), err, "simple");

        if (!hasSubrectUploads)
            return;

        const auto bytesPerPixel = format.bytesPerPixel();

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1);

        subrectBuffer.fill(0);
        memcpy(subrectBuffer.data() + bytesPerPixel, srcBuffer.data(), bytesPerPixel);
        fnTestData(format, subrectBuffer.data(), err, "subrect");

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    };

    // ---------------------------------

    glUseProgram(floatsProg);

    // RGBA+UNSIGNED_BYTE
    {
        const uint8_t src[] = {EncodeNormUint<8>(srcVals[0]), EncodeNormUint<8>(srcVals[1]),
                               EncodeNormUint<8>(srcVals[2]), EncodeNormUint<8>(srcVals[3])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE}, {8, 8, 8, 255});
        fnTest({GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE}, {16, 16, 16, 16});

        fnTest({GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}, {1, 1, 1, 0});
        fnTest({GL_RGB565, GL_RGB, GL_UNSIGNED_BYTE}, {8, 4, 8, 0});

        fnTest({GL_RG8, GL_RG, GL_UNSIGNED_BYTE}, {1, 1, 0, 0});

        fnTest({GL_R8, GL_RED, GL_UNSIGNED_BYTE}, {1, 0, 0, 0});

        fnTest({GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RGB, GL_RGB, GL_UNSIGNED_BYTE}, {1, 1, 1, 0});
        fnTest({GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE}, {1, 1, 1, 0});
        fnTest({GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE}, {0, 0, 0, 1});
    }

    // RGBA+BYTE
    {
        const uint8_t src[] = {EncodeNormUint<7>(srcVals[0]), EncodeNormUint<7>(srcVals[1]),
                               EncodeNormUint<7>(srcVals[2]), EncodeNormUint<7>(srcVals[3])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA8_SNORM, GL_RGBA, GL_BYTE}, {2, 2, 2, 2});
        fnTest({GL_RGB8_SNORM, GL_RGB, GL_BYTE}, {2, 2, 2, 0});
        fnTest({GL_RG8_SNORM, GL_RG, GL_BYTE}, {2, 2, 0, 0});
        fnTest({GL_R8_SNORM, GL_RED, GL_BYTE}, {2, 0, 0, 0});
    }

    // RGBA+UNSIGNED_SHORT_4_4_4_4
    {
        const uint16_t src[] = {
            (EncodeNormUint<4>(srcVals[0]) << 12) | (EncodeNormUint<4>(srcVals[1]) << 8) |
            (EncodeNormUint<4>(srcVals[2]) << 4) | (EncodeNormUint<4>(srcVals[3]) << 0)};
        ZeroAndCopy(srcBuffer, src);

        // fnTest({GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4}, {16,16,16,16});
        fnTest({GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4}, {16, 16, 16, 16});
    }

    // RGBA+UNSIGNED_SHORT_5_5_5_1
    {
        const uint16_t src[] = {
            (EncodeNormUint<5>(srcVals[0]) << 11) | (EncodeNormUint<5>(srcVals[1]) << 6) |
            (EncodeNormUint<5>(srcVals[2]) << 1) | (EncodeNormUint<1>(srcVals[3]) << 0)};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1}, {8, 8, 8, 255});
        fnTest({GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1}, {8, 8, 8, 255});
    }

    // RGBA+UNSIGNED_INT_2_10_10_10_REV
    {
        const uint32_t src[] = {
            (EncodeNormUint<10>(srcVals[0]) << 0) | (EncodeNormUint<10>(srcVals[1]) << 10) |
            (EncodeNormUint<10>(srcVals[2]) << 20) | (EncodeNormUint<2>(srcVals[3]) << 30)};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV}, {1, 1, 1, 128});
        fnTest({GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV}, {8, 8, 8, 255});
    }

    // RGBA+HALF_FLOAT
    {
        const uint16_t src[] = {Float16::Encode(srcVals[0]), Float16::Encode(srcVals[1]),
                                Float16::Encode(srcVals[2]), Float16::Encode(srcVals[3])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT}, {1, 1, 1, 1});

        fnTest({GL_RGB16F, GL_RGB, GL_HALF_FLOAT}, {1, 1, 1, 0});
        fnTest({GL_R11F_G11F_B10F, GL_RGB, GL_HALF_FLOAT}, {1, 1, 1, 0});
        fnTest({GL_RGB9_E5, GL_RGB, GL_HALF_FLOAT}, {1, 1, 1, 0});

        fnTest({GL_RG16F, GL_RG, GL_HALF_FLOAT}, {1, 1, 0, 0});

        fnTest({GL_R16F, GL_RED, GL_HALF_FLOAT}, {1, 0, 0, 0});

        fnTest({GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES}, {1, 1, 1, 1});
        fnTest({GL_RGB, GL_RGB, GL_HALF_FLOAT_OES}, {1, 1, 1, 0});
        fnTest({GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES}, {1, 1, 1, 1});
        fnTest({GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT_OES}, {1, 1, 1, 0});
        fnTest({GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT_OES}, {0, 0, 0, 1});
    }

    // RGBA+FLOAT
    {
        ZeroAndCopy(srcBuffer, srcVals);

        fnTest({GL_RGBA32F, GL_RGBA, GL_FLOAT}, {1, 1, 1, 1});
        fnTest({GL_RGBA16F, GL_RGBA, GL_FLOAT}, {1, 1, 1, 1});

        fnTest({GL_RGB32F, GL_RGB, GL_FLOAT}, {1, 1, 1, 0});
        fnTest({GL_RGB16F, GL_RGB, GL_FLOAT}, {1, 1, 1, 0});
        fnTest({GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT}, {1, 1, 1, 0});
        fnTest({GL_RGB9_E5, GL_RGB, GL_FLOAT}, {1, 1, 1, 0});

        fnTest({GL_RG32F, GL_RG, GL_FLOAT}, {1, 1, 0, 0});
        fnTest({GL_RG16F, GL_RG, GL_FLOAT}, {1, 1, 0, 0});

        fnTest({GL_R32F, GL_RED, GL_FLOAT}, {1, 0, 0, 0});
        fnTest({GL_R16F, GL_RED, GL_FLOAT}, {1, 0, 0, 0});
    }

    // ---------------------------------

    glUseProgram(intsProg);

    // RGBA_INTEGER+UNSIGNED_BYTE
    {
        const uint8_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});
        fnTest({GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE}, {1, 1, 1, 1});

        fnTest({GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RGB8I, GL_RGB_INTEGER, GL_BYTE}, {1, 1, 1, 1});
        fnTest({GL_RG8I, GL_RG_INTEGER, GL_BYTE}, {1, 1, 1, 1});
        fnTest({GL_R8I, GL_RED_INTEGER, GL_BYTE}, {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_SHORT
    {
        const uint16_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT}, {1, 1, 1, 1});
        fnTest({GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT}, {1, 1, 1, 1});
        fnTest({GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT}, {1, 1, 1, 1});
        fnTest({GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT}, {1, 1, 1, 1});

        fnTest({GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT}, {1, 1, 1, 1});
        fnTest({GL_RGB16I, GL_RGB_INTEGER, GL_SHORT}, {1, 1, 1, 1});
        fnTest({GL_RG16I, GL_RG_INTEGER, GL_SHORT}, {1, 1, 1, 1});
        fnTest({GL_R16I, GL_RED_INTEGER, GL_SHORT}, {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_INT
    {
        const uint32_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT}, {1, 1, 1, 1});
        fnTest({GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT}, {1, 1, 1, 1});
        fnTest({GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT}, {1, 1, 1, 1});
        fnTest({GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT}, {1, 1, 1, 1});

        fnTest({GL_RGBA32I, GL_RGBA_INTEGER, GL_INT}, {1, 1, 1, 1});
        fnTest({GL_RGB32I, GL_RGB_INTEGER, GL_INT}, {1, 1, 1, 1});
        fnTest({GL_RG32I, GL_RG_INTEGER, GL_INT}, {1, 1, 1, 1});
        fnTest({GL_R32I, GL_RED_INTEGER, GL_INT}, {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_INT_2_10_10_10_REV
    {
        const uint32_t src[] = {(srcIntVals[0] << 0) | (srcIntVals[1] << 10) |
                                (srcIntVals[2] << 20) | (srcIntVals[3] << 30)};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV}, {1, 1, 1, 1});
    }

    // ---------------------------------

    glUseProgram(floatsProg);

    // RGB+UNSIGNED_SHORT_5_6_5
    {
        const uint16_t src[] = {(EncodeNormUint<5>(srcVals[0]) << 11) |
                                (EncodeNormUint<6>(srcVals[1]) << 5) |
                                (EncodeNormUint<5>(srcVals[2]) << 0)};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5}, {8, 4, 8, 0});
        fnTest({GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5}, {8, 4, 8, 0});
    }

    // UNSIGNED_INT_10F_11F_11F_REV
    {
        const uint32_t src[] = {(UFloat11::Encode(srcVals[0]) << 0) |
                                (UFloat11::Encode(srcVals[1]) << 11) |
                                (UFloat10::Encode(srcVals[2]) << 22)};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV}, {1, 1, 1, 0});
    }

    // UNSIGNED_INT_5_9_9_9_REV
    {
        const uint32_t src[] = {EncodeRGB9_E5_Rev(srcVals[0], srcVals[1], srcVals[2])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV}, {1, 1, 1, 0});
    }

    // DEPTH_COMPONENT+UNSIGNED_SHORT
    {
        const uint16_t src[] = {EncodeNormUint<16>(srcVals[0])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}, {1, 0, 0, 0});
    }

    // DEPTH_COMPONENT+UNSIGNED_INT
    {
        const uint32_t src[] = {EncodeNormUint<32>(srcVals[0])};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}, {1, 0, 0, 0});
        fnTest({GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}, {1, 0, 0, 0});
    }

    // DEPTH_COMPONENT+FLOAT
    {
        // Skip stencil.
        const float src[] = {srcVals[0], 0};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}, {1, 0, 0, 0});
        fnTest({GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV},
               {1, 0, 0, 0});
    }

    // DEPTH_STENCIL+UNSIGNED_INT_24_8
    {
        // Drop stencil.
        const uint32_t src[] = {EncodeNormUint<24>(srcVals[0]) << 8};
        ZeroAndCopy(srcBuffer, src);

        fnTest({GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8}, {1, 0, 0, 0});
    }

    // --

    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(TextureUploadFormatTest,
                       ES2_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_D3D9(),
                       ES2_OPENGL(),
                       ES2_OPENGLES());

}  // anonymous namespace
