//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlendFuncExtendedTest
//   Test EXT_blend_func_extended

#include "test_utils/ANGLETest.h"
#include "shader_utils.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <string>

using namespace angle;

namespace
{

// Partial implementation of weight function for GLES 2 blend equation that
// is dual-source aware.
template <int factor, int index>
float Weight(const float /*dst*/[4], const float src[4], const float src1[4])
{
    if (factor == GL_SRC_COLOR)
        return src[index];
    if (factor == GL_SRC_ALPHA)
        return src[3];
    if (factor == GL_SRC1_COLOR_EXT)
        return src1[index];
    if (factor == GL_SRC1_ALPHA_EXT)
        return src1[3];
    if (factor == GL_ONE_MINUS_SRC1_COLOR_EXT)
        return 1.0f - src1[index];
    if (factor == GL_ONE_MINUS_SRC1_ALPHA_EXT)
        return 1.0f - src1[3];
    return 0.0f;
}

// Implementation of GLES 2 blend equation that is dual-source aware.
template <int RGBs, int RGBd, int As, int Ad>
void BlendEquationFuncAdd(const float dst[4],
                          const float src[4],
                          const float src1[4],
                          uint8_t result[4])
{
    float r[4];
    r[0] = src[0] * Weight<RGBs, 0>(dst, src, src1) +
           dst[0] * Weight<RGBd, 0>(dst, src, src1);
    r[1] = src[1] * Weight<RGBs, 1>(dst, src, src1) +
           dst[1] * Weight<RGBd, 1>(dst, src, src1);
    r[2] = src[2] * Weight<RGBs, 2>(dst, src, src1) +
           dst[2] * Weight<RGBd, 2>(dst, src, src1);
    r[3] = src[3] * Weight<As, 3>(dst, src, src1) +
           dst[3] * Weight<Ad, 3>(dst, src, src1);

    for (int i = 0; i < 4; ++i)
    {
        result[i] = static_cast<uint8_t>(
            std::floor(std::max(0.0f, std::min(1.0f, r[i])) * 255.0f));
    }
}

bool checkPixels(GLint x,
                 GLint y,
                 GLsizei width,
                 GLsizei height,
                 GLint tolerance,
                 const uint8_t* color)
{
    const GLsizei size = width * height * 4;

    std::vector<uint8_t> pixels;
    pixels.resize(size);

    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

    int bad_count = 0;
    for (GLint yy = 0; yy < height; ++yy)
    {
        for (GLint xx = 0; xx < width; ++xx)
        {
            int offset = yy * width * 4 + xx * 4;
            for (int jj = 0; jj < 4; ++jj)
            {
                uint8_t actual = pixels[offset + jj];
                uint8_t expected = color[jj];
                int diff = actual - expected;
                diff = diff < 0 ? -diff: diff;
                if (diff > tolerance)
                {
                    EXPECT_EQ(expected, actual) << " at " << (xx + x) << ", " << (yy + y)
                                      << " channel " << jj;
                    ++bad_count;
                    // Exit early just so we don't spam the log but we print enough
                    // to hopefully make it easy to diagnose the issue.
                    if (bad_count > 16)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return bad_count == 0;
}

const GLuint kWidth = 100;
const GLuint kHeight = 100;

class EXTBlendFuncExtendedTest : public ANGLETest
{
protected:
    bool isApplicable() const
    {
        return extensionEnabled("GL_EXT_blend_func_extended");
    }
};

class EXTBlendFuncExtendedDrawTest : public ANGLETest
{
protected:
    EXTBlendFuncExtendedDrawTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    bool isApplicable() const
    {
        return extensionEnabled("GL_EXT_blend_func_extended");
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        static const float vertices[] =
        {
            1.0f,  1.0f,
           -1.0f,  1.0f,
           -1.0f, -1.0f,
            1.0f,  1.0f,
           -1.0f, -1.0f,
            1.0f, -1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    void TearDown() override
    {
        glDeleteBuffers(1, &mVBO);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    void MakeProgram(const char* vertSource, const char* fragSource)
    {
        mProgram = CompileProgram(vertSource, fragSource);
    }

    void DrawTest()
    {
        glUseProgram(mProgram);

        GLint  position = glGetAttribLocation(mProgram, "position");
        GLint  src0     = glGetUniformLocation(mProgram, "src0");
        GLint  src1     = glGetUniformLocation(mProgram, "src1");
        ASSERT_GL_NO_ERROR();

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, 0);
        ASSERT_GL_NO_ERROR();

        static const float kDst[4]  = {0.5f, 0.5f, 0.5f, 0.5f};
        static const float kSrc0[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        static const float kSrc1[4] = {0.3f, 0.6f, 0.9f, 0.7f};

        glUniform4f(src0, kSrc0[0], kSrc0[1], kSrc0[2], kSrc0[3]);
        glUniform4f(src1, kSrc1[0], kSrc1[1], kSrc1[2], kSrc1[3]);
        ASSERT_GL_NO_ERROR();

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glViewport(0, 0, kWidth, kHeight);
        glClearColor(kDst[0], kDst[1], kDst[2], kDst[3]);
        ASSERT_GL_NO_ERROR();

        {
            glBlendFuncSeparate(GL_SRC1_COLOR_EXT, GL_SRC_ALPHA,
                                GL_ONE_MINUS_SRC1_COLOR_EXT,
                                GL_ONE_MINUS_SRC1_ALPHA_EXT);

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ASSERT_GL_NO_ERROR();

            // verify
            uint8_t color[4];
            BlendEquationFuncAdd<GL_SRC1_COLOR_EXT, GL_SRC_ALPHA,
                                 GL_ONE_MINUS_SRC1_COLOR_EXT,
                                 GL_ONE_MINUS_SRC1_ALPHA_EXT>(kDst, kSrc0, kSrc1, color);

            EXPECT_TRUE(checkPixels(kWidth / 4, (3 * kHeight) / 4, 1, 1,
                                          1, color));
            EXPECT_TRUE(checkPixels(kWidth - 1, 0, 1, 1, 1, color));
        }

        {
            glBlendFuncSeparate(GL_ONE_MINUS_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC_ALPHA,
                                GL_ONE_MINUS_SRC_COLOR,
                                GL_SRC1_ALPHA_EXT);

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ASSERT_GL_NO_ERROR();

            // verify
            uint8_t color[4];
            BlendEquationFuncAdd<GL_ONE_MINUS_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC_ALPHA,
                                 GL_ONE_MINUS_SRC_COLOR,
                                 GL_SRC1_ALPHA_EXT>(kDst, kSrc0, kSrc1, color);

            EXPECT_TRUE(checkPixels(kWidth / 4, (3 * kHeight) / 4, 1, 1,
                                          1, color));
            EXPECT_TRUE(checkPixels(kWidth - 1, 0, 1, 1, 1, color));
        }
    }

    GLuint mVBO;
    GLuint mProgram;
};

} // namespace

TEST_P(EXTBlendFuncExtendedTest, TestMaxDualSourceDrawBuffers)
{
    if (!isApplicable())
        return;

    GLint maxDualSourceDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT, &maxDualSourceDrawBuffers);
    EXPECT_GT(maxDualSourceDrawBuffers, 0);

    ASSERT_GL_NO_ERROR();
}

TEST_P(EXTBlendFuncExtendedDrawTest, FragColor)
{
    if (!isApplicable())
        return;

    static const char* kVertexShader =
        "attribute vec4 position;\n"
        "void main() {\n"
        "  gl_Position = position;\n"
        "}\n";

    static const char* kFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "precision mediump float;\n"
        "uniform vec4 src0;\n"
        "uniform vec4 src1;\n"
        "void main() {\n"
        "  gl_FragColor = src0;\n"
        "  gl_SecondaryFragColorEXT = src1;\n"
        "}\n";

    MakeProgram(kVertexShader, kFragColorShader);

    DrawTest();

}

TEST_P(EXTBlendFuncExtendedDrawTest, FragData)
{
    if (!isApplicable())
        return;

    static const char* kVertexShader =
        "attribute vec4 position;\n"
        "void main() {\n"
        "  gl_Position = position;\n"
        "}\n";

    static const char* kFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "precision mediump float;\n"
        "uniform vec4 src0;\n"
        "uniform vec4 src1;\n"
        "void main() {\n"
        "  gl_FragData[0] = src0;\n"
        "  gl_SecondaryFragDataEXT[0] = src1;\n"
        "}\n";

    MakeProgram(kVertexShader, kFragColorShader);

    DrawTest();
}

ANGLE_INSTANTIATE_TEST(EXTBlendFuncExtendedTest,         ES2_OPENGL(), ES2_OPENGLES(), ES3_OPENGL(), ES3_OPENGLES());
ANGLE_INSTANTIATE_TEST(EXTBlendFuncExtendedDrawTest,     ES2_OPENGL(), ES2_OPENGLES());
