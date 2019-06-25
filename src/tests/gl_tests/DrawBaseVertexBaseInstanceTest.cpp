//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DrawBaseVertexBaseInstanceTest: Tests of GL_ANGLE_base_vertex_base_instance

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <numeric>

using namespace angle;

namespace
{

// Create a kWidth * kHeight canvas equally split into kCountX * kCountY tiles
// each containing a quad partially covering each tile
constexpr uint32_t kWidth                  = 256;
constexpr uint32_t kHeight                 = 256;
constexpr uint32_t kCountX                 = 8;
constexpr uint32_t kCountY                 = 8;
constexpr std::array<GLfloat, 2> kTileSize = {
    1.f / static_cast<GLfloat>(kCountX),
    1.f / static_cast<GLfloat>(kCountY),
};
constexpr std::array<uint32_t, 2> kTilePixelSize  = {kWidth / kCountX, kHeight / kCountY};
constexpr std::array<GLfloat, 2> kQuadRadius      = {0.25f * kTileSize[0], 0.25f * kTileSize[1]};
constexpr std::array<uint32_t, 2> kPixelCheckSize = {
    static_cast<uint32_t>(kQuadRadius[0] * kWidth),
    static_cast<uint32_t>(kQuadRadius[1] * kHeight)};

constexpr std::array<GLfloat, 2> getTileCenter(uint32_t x, uint32_t y)
{
    return {
        kTileSize[0] * (0.5f + static_cast<GLfloat>(x)),
        kTileSize[1] * (0.5f + static_cast<GLfloat>(y)),
    };
}
constexpr std::array<std::array<GLfloat, 3>, 4> getQuadVertices(uint32_t x, uint32_t y)
{
    const auto center = getTileCenter(x, y);
    return {
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
    };
}

// TODO: VertexID, InstanceID Option (Has Shader Builtin)

enum class ShaderBuiltinOption
{
    NoShaderBuiltin,
    UseShaderBuiltin
};

using DrawBaseVertexBaseInstanceTestParams =
    std::tuple<angle::PlatformParameters, ShaderBuiltinOption>;

struct PrintToStringParamName
{
    std::string operator()(
        const ::testing::TestParamInfo<DrawBaseVertexBaseInstanceTestParams> &info) const
    {
        ::std::stringstream ss;
        ss << std::get<0>(info.param)
           << (std::get<1>(info.param) == ShaderBuiltinOption::UseShaderBuiltin ? "ShaderBuiltin_"
                                                                                : "");
        return ss.str();
    }
};

// These tests check correctness of the ANGLE_base_vertex_base_instance extension.
// An array of quads is drawn across the screen.
// gl_VertexID, gl_InstanceID, gl_BaseVertex, and gl_BaseInstance
// are checked by using them to select the color of the draw.
class DrawBaseVertexBaseInstanceTest
    : public ANGLETestBase,
      public ::testing::TestWithParam<DrawBaseVertexBaseInstanceTestParams>
{
  protected:
    DrawBaseVertexBaseInstanceTest()
        : ANGLETestBase(std::get<0>(GetParam())),
          mNonIndexedVertexBuffer(0u),
          mVertexBuffer(0u),
          mIndexBuffer(0u),
          mProgram(0u)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    bool HasShaderBuiltin() const
    {
        return std::get<1>(GetParam()) == ShaderBuiltinOption::UseShaderBuiltin;
    }

    std::string VertexShaderSource300(bool isDrawArrays, bool isMultiDraw)
    {
        std::stringstream shader;
        shader << ("#version 300 es\n")
               << (isMultiDraw ? "#extension GL_ANGLE_multi_draw : require\n" : "")
               << ("#extension GL_ANGLE_base_vertex_base_instance : require\n")
               << "#define kCountX " << kCountX << "\n"
               << "#define kCountY " << kCountY << "\n"
               << R"(
in vec2 vPosition;
out vec4 color;
void main()
{
    const float xStep = 1.0 / float(kCountX);
    const float yStep = 1.0 / float(kCountY);
    float x_id = float(gl_InstanceID + gl_BaseInstance);
    float y_id = floor(float(gl_VertexID) / )"
               << (isDrawArrays ? "6.0" : "4.0") << R"( + 0.01);

    color = vec4(
        1.0 - xStep * x_id,
        1.0 - yStep * y_id,
        1.0 - yStep * float(gl_BaseVertex) / 4.0,
        1);

    mat3 transform = mat3(1.0);
    transform[2][0] = x_id * xStep;

    gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
})";

        return shader.str();
    }

    std::string FragmentShaderSource300()
    {
        return
            R"(#version 300 es
            precision mediump float;
            in vec4 color;
            out vec4 o_color;
            void main()
            {
                o_color = color;
            })";
    }

    void SetupProgram(bool isDrawArrays = true, bool isMultiDraw = false)
    {
        mProgram = CompileProgram(VertexShaderSource300(isDrawArrays, isMultiDraw).c_str(),
                                  FragmentShaderSource300().c_str());

        EXPECT_GL_NO_ERROR();
        ASSERT_GE(mProgram, 1u);
        glUseProgram(mProgram);
        mPositionLoc = glGetAttribLocation(mProgram, "vPosition");
    }

    void SetupBuffers()
    {
        // Rects in the same column are within a vertex array, testing gl_VertexID, gl_BaseVertex
        // Rects in the same row are drawn by instancing, testing gl_InstanceID, gl_BaseInstance

        mIndices = {0, 1, 2, 0, 2, 3};

        for (uint32_t y = 0; y < kCountY; ++y)
        {
            // v3 ---- v2
            // |       |
            // |       |
            // v0 ---- v1

            const auto vs = getQuadVertices(0, y);

            for (const auto &v : vs)
            {
                mVertices.insert(mVertices.end(), v.begin(), v.end());
            }

            for (GLushort i : mIndices)
            {
                mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(), vs[i].end());
            }
        }

        glGenBuffers(1, &mNonIndexedVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     GL_STATIC_DRAW);

        ASSERT_GL_NO_ERROR();
    }

    void DoDrawCommons()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    void DoVertexAttribDivisor(GLint location, GLuint divisor)
    {
        if (getClientMajorVersion() <= 2)
        {
            ASSERT_TRUE(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
            glVertexAttribDivisorANGLE(location, divisor);
        }
        else
        {
            glVertexAttribDivisor(location, divisor);
        }
    }

    void DoDrawArraysInstancedBaseInstance()
    {
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        DoDrawCommons();

        const uint32_t countPerDraw = kCountY * 6 * 2;

        for (uint32_t i = 0; i < kCountX; i += 2)
        {
            glDrawArraysInstancedBaseInstanceANGLE(GL_TRIANGLES, 0, countPerDraw, 2, i);
        }
    }

    void DoMultiDrawArraysInstancedBaseInstance()
    {
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        DoDrawCommons();

        const uint32_t countPerDraw = kCountY * 6 * 2;
        const std::vector<GLsizei> counts(kCountX, countPerDraw);
        const std::vector<GLsizei> instanceCounts(kCountX, 2);
        const std::vector<GLsizei> firsts(kCountX, 0);
        std::vector<GLuint> baseInstances(kCountX);
        for (size_t i = 0; i < kCountX; i++)
        {
            baseInstances[i] = i * 2;
        }

        glMultiDrawArraysInstancedBaseInstanceANGLE(GL_TRIANGLES, kCountX, counts.data(),
                                                    instanceCounts.data(), firsts.data(),
                                                    baseInstances.data());
    }

    void DoDrawElementsInstancedBaseVertexBaseInstance()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        DoDrawCommons();

        const uint32_t countPerDraw = 6;

        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                glDrawElementsInstancedBaseVertexBaseInstanceANGLE(
                    GL_TRIANGLES, countPerDraw, GL_UNSIGNED_SHORT,
                    reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 2, v * 4, i);
            }
        }
    }

    void DoMultiDrawElementsInstancedBaseVertexBaseInstance()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        DoDrawCommons();

        const GLsizei drawCount = kCountX * kCountY / 2;
        const std::vector<GLsizei> counts(drawCount, 6);
        const std::vector<GLsizei> instanceCounts(drawCount, 2);
        const std::vector<GLvoid *> indices(drawCount, 0);
        std::vector<GLint> baseVertices(drawCount);
        std::vector<GLuint> baseInstances(drawCount);

        GLsizei b = 0;
        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                baseVertices[b]  = v * 4;
                baseInstances[b] = i;
                b++;
            }
        }

        glMultiDrawElementsInstancedBaseVertexBaseInstanceANGLE(
            GL_TRIANGLES, GL_UNSIGNED_SHORT, drawCount, counts.data(), instanceCounts.data(),
            indices.data(), baseVertices.data(), baseInstances.data());
    }

    void CheckDrawResult(bool hasBaseVertex)
    {
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                uint32_t center_x = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;

                EXPECT_PIXEL_NEAR(
                    center_x - kPixelCheckSize[0] / 2, center_y - kPixelCheckSize[1] / 2,
                    256.0 * (1.0 - (float)x / (float)kCountX),
                    256.0 * (1.0 - (float)y / (float)kCountY),
                    hasBaseVertex ? 256.0 * (1.0 - (float)y / (float)kCountY) : 255, 255, 3);
            }
        }
    }

    void TearDown() override
    {
        if (mNonIndexedVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mNonIndexedVertexBuffer);
        }
        if (mVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mIndexBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndexBuffer);
        }
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        ANGLETestBase::ANGLETestTearDown();
    }

    bool requestDrawBaseVertexBaseInstanceExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_base_vertex_base_instance"))
        {
            glRequestExtensionANGLE("GL_ANGLE_base_vertex_base_instance");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance"))
        {
            return false;
        }

        return true;
    }

    bool requestInstancedExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_instanced_arrays"))
        {
            glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"))
        {
            return false;
        }

        return true;
    }

    bool requestExtensions()
    {
        if (getClientMajorVersion() <= 2)
        {
            if (!requestInstancedExtension())
            {
                return false;
            }
        }
        return requestDrawBaseVertexBaseInstanceExtension();
    }

    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    GLuint mNonIndexedVertexBuffer;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    GLuint mProgram;
    GLint mPositionLoc;
};

TEST_P(DrawBaseVertexBaseInstanceTest, RequestExtension)
{
    // TODO:? check backend version?
    // *BaseVertex native function support should be present
}

// Test that compile a program with the extension succeeds
TEST_P(DrawBaseVertexBaseInstanceTest, CanCompile)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupProgram();
}

// Tests basic functionality of glDrawArraysInstancedBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, DrawArraysInstancedBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram(true);
    DoDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(false);
}

// Tests basic functionality of glMultiDrawArraysInstancedBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, MultiDrawArraysInstancedBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram(true, true);
    DoMultiDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(false);
}

// Tests basic functionality of glDrawElementsInstancedBaseVertexBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, DrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram(false);
    DoDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(true);
}

// Tests basic functionality of glMultiDrawElementsInstancedBaseVertexBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, MultiDrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram(false, true);
    DoMultiDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(true);
}

const angle::PlatformParameters platforms[] = {
    ES3_D3D11(),
    ES3_OPENGL(),
    ES3_OPENGLES(),
    ES3_VULKAN(),
};

INSTANTIATE_TEST_SUITE_P(
    ,
    DrawBaseVertexBaseInstanceTest,
    testing::Combine(testing::ValuesIn(::angle::FilterTestParams(platforms, ArraySize(platforms))),
                     testing::Values(ShaderBuiltinOption::NoShaderBuiltin)),
    PrintToStringParamName());

}  // namespace
