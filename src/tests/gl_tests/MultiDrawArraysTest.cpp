//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultiDrawArraysTest: Tests of GL_WEBGL_multi_draw_arrays

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Create a width * height canvas equally split into x_count * y_count tiles
// each containing a quad partially coverting each tile
constexpr uint32_t width                  = 256;
constexpr uint32_t height                 = 256;
constexpr uint32_t x_count                = 16;
constexpr uint32_t y_count                = 16;
constexpr uint32_t quad_count             = x_count * y_count;
constexpr uint32_t tri_count              = quad_count * 2;
constexpr std::array<GLfloat, 2> tileSize = {
    1.f / static_cast<GLfloat>(x_count),
    1.f / static_cast<GLfloat>(y_count),
};
constexpr std::array<uint32_t, 2> tilePixelSize  = {width / x_count, height / y_count};
constexpr std::array<GLfloat, 2> quadRadius      = {0.25f * tileSize[0], 0.25f * tileSize[1]};
constexpr std::array<uint32_t, 2> pixelCheckSize = {quadRadius[0] * width, quadRadius[1] * height};

constexpr std::array<GLfloat, 2> getTileCenter(uint32_t x, uint32_t y)
{
    return {
        tileSize[0] * (0.5f + static_cast<GLfloat>(x)),
        tileSize[1] * (0.5f + static_cast<GLfloat>(y)),
    };
}
constexpr std::array<std::array<GLfloat, 3>, 4> getQuadVertices(uint32_t x, uint32_t y)
{
    const auto center = getTileCenter(x, y);
    return {
        std::array<GLfloat, 3>{center[0] - quadRadius[0], center[1] - quadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + quadRadius[0], center[1] - quadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + quadRadius[0], center[1] + quadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] - quadRadius[0], center[1] + quadRadius[1], 0.0f},
    };
}

const char *VertexShaderNoDrawIDSource()
{
    return
        R"(attribute vec2 vPosition;
        varying vec4 color;
        void main()
        {
            color = vec4(1.0, 0.0, 0.0, 1.0);
            gl_Position = vec4(vPosition * 2.0 - 1.0, 0, 1);
        })";
}

const char *VertexShaderWithDrawIDSource()
{
    return
        R"(#extension GL_WEBGL_multi_draw_arrays : require
        attribute vec2 vPosition;
        varying vec4 color;
        void main()
        {
            float quad_id = float(gl_DrawID / 2);
            float color_id = quad_id - (3.0 * floor(quad_id / 3.0));
            if (color_id == 0.0) {
              color = vec4(1, 0, 0, 1);
            } else if (color_id == 1.0) {
              color = vec4(0, 1, 0, 1);
            } else {
              color = vec4(0, 0, 1, 1);
            }
            gl_Position = vec4(vPosition * 2.0 - 1.0, 0, 1);
        })";
}

const char *FragmentShaderSource()
{
    return
        R"(precision mediump float;
        varying vec4 color;
        void main()
        {
            gl_FragColor = color;
        })";
}

void CheckDrawNoDrawIDResult()
{
    for (uint32_t y = 0; y < y_count; ++y)
    {
        for (uint32_t x = 0; x < x_count; ++x)
        {
            uint32_t center_x = x * tilePixelSize[0] + tilePixelSize[0] / 2;
            uint32_t center_y = y * tilePixelSize[1] + tilePixelSize[1] / 2;
            EXPECT_PIXEL_RECT_EQ(center_x - pixelCheckSize[0] / 2, center_y - pixelCheckSize[1] / 2,
                                 pixelCheckSize[0], pixelCheckSize[1], GLColor(255, 0, 0, 255));
        }
    }
}

void CheckDrawWithDrawIDResult()
{
    for (uint32_t y = 0; y < y_count; ++y)
    {
        for (uint32_t x = 0; x < x_count; ++x)
        {
            uint32_t center_x = x * tilePixelSize[0] + tilePixelSize[0] / 2;
            uint32_t center_y = y * tilePixelSize[1] + tilePixelSize[1] / 2;
            uint32_t quadID   = y * x_count + x;
            uint32_t colorID  = quadID % 3u;
            std::array<GLColor, 3> colors = {GLColor(255, 0, 0, 255),
                                             GLColor(0, 255, 0, 255),
                                             GLColor(0, 0, 255, 255)};
            GLColor expected = colors[colorID];
            EXPECT_PIXEL_RECT_EQ(center_x - pixelCheckSize[0] / 2, center_y - pixelCheckSize[1] / 2,
                                 pixelCheckSize[0], pixelCheckSize[1], expected);
        }
    }
}

class MultiDrawArraysTest : public ANGLETest
{
  protected:
    MultiDrawArraysTest()
        : mVertexBuffer(0u), mIndexBuffer(0u), mSimpleProgram(0u), mDrawIDProgram(0u)
    {
        setWindowWidth(width);
        setWindowHeight(height);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        for (uint32_t y = 0; y < y_count; ++y)
        {
            for (uint32_t x = 0; x < x_count; ++x)
            {
                // v3 ---- v2
                // |       |
                // |       |
                // v0 ---- v1
                uint32_t quadIndex         = y * x_count + x;
                GLushort starting_index    = static_cast<GLushort>(4 * quadIndex);
                std::array<GLushort, 6> is = {0, 1, 2, 0, 2, 3};
                const auto vs              = getQuadVertices(x, y);
                for (GLushort i : is)
                {
                    mIndices.push_back(starting_index + i);
                }

                for (const auto &v : vs)
                {
                    mVertices.insert(mVertices.end(), v.begin(), v.end());
                }

                for (GLushort i : is)
                {
                    mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(),
                                               vs[i].end());
                }
            }
        }
    }

    void SetupProgramNoDrawID()
    {
        mSimpleProgram = CompileProgram(VertexShaderNoDrawIDSource(), FragmentShaderSource());
        ASSERT_GE(mSimpleProgram, 1u);
        glUseProgram(mSimpleProgram);
    }

    void SetupProgramWithDrawID()
    {
        mDrawIDProgram = CompileProgram(VertexShaderWithDrawIDSource(), FragmentShaderSource());
        ASSERT_GE(mDrawIDProgram, 1u);
        glUseProgram(mDrawIDProgram);
    }

    void DoDrawArrays()
    {
        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        ASSERT_GL_NO_ERROR();

        std::vector<GLint> firsts(tri_count);
        std::vector<GLsizei> counts(tri_count, 3);
        for (uint32_t i = 0; i < tri_count; ++i)
            firsts[i] = i * 3;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMultiDrawArraysWEBGL(GL_TRIANGLES, firsts.data(), counts.data(), tri_count);
        EXPECT_GL_NO_ERROR();
    }

    void DoDrawElements()
    {
        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        ASSERT_GL_NO_ERROR();

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     GL_STATIC_DRAW);
        ASSERT_GL_NO_ERROR();

        std::vector<GLsizei> counts(tri_count, 3);
        std::vector<GLsizei> offsets(tri_count);
        for (uint32_t i = 0; i < tri_count; ++i)
            offsets[i] = i * 3 * 2;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMultiDrawElementsWEBGL(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT, offsets.data(),
                                 tri_count);
    }

    void TearDown() override
    {
        if (mVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mIndexBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndexBuffer);
        }
        if (mSimpleProgram != 0u)
        {
            glDeleteProgram(mSimpleProgram);
        }
        if (mDrawIDProgram != 0)
        {
            glDeleteProgram(mDrawIDProgram);
        }
        ANGLETest::TearDown();
    }

    bool requestExtension()
    {
        if (extensionRequestable("GL_WEBGL_multi_draw_arrays"))
        {
            glRequestExtensionANGLE("GL_WEBGL_multi_draw_arrays");
        }

        if (!extensionEnabled("GL_WEBGL_multi_draw_arrays"))
        {
            return false;
        }
        return true;
    }

    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    GLuint mSimpleProgram;
    GLuint mDrawIDProgram;
};

// glMultiDraw*WEBGL are emulated and should always be available
TEST_P(MultiDrawArraysTest, RequestExtension)
{
    EXPECT_TRUE(requestExtension());
}

// Tests basic functionality of glMultiDrawArrays
TEST_P(MultiDrawArraysTest, ValidateMultiDrawArrays)
{
    ANGLE_SKIP_TEST_IF(!requestExtension());
    SetupProgramNoDrawID();
    DoDrawArrays();
    CheckDrawNoDrawIDResult();
}

// Tests basic functionality of glMultiDrawArrays with gl_DrawID
TEST_P(MultiDrawArraysTest, ValidateMultiDrawArraysDrawID)
{
    ANGLE_SKIP_TEST_IF(!requestExtension());
    SetupProgramWithDrawID();
    DoDrawArrays();
    CheckDrawWithDrawIDResult();
}

// Tests basic functionality of glMultiDrawElements
TEST_P(MultiDrawArraysTest, ValidateMultiDrawElements)
{
    ANGLE_SKIP_TEST_IF(!requestExtension());
    SetupProgramNoDrawID();
    DoDrawElements();
    CheckDrawNoDrawIDResult();
}

// Tests basic functionality of glMultiDrawElements with gl_DrawID
TEST_P(MultiDrawArraysTest, ValidateMultiDrawElementsDrawID)
{
    ANGLE_SKIP_TEST_IF(!requestExtension());
    SetupProgramWithDrawID();
    DoDrawElements();
    CheckDrawWithDrawIDResult();
}

ANGLE_INSTANTIATE_TEST(MultiDrawArraysTest,
                       ES2_D3D9(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_D3D11(),
                       ES3_OPENGL(),
                       ES3_OPENGLES());
}  // namespace
