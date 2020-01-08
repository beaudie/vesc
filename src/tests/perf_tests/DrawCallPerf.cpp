//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawCallPerf:
//   Performance tests for ANGLE draw call overhead.
//

#include "ANGLEPerfTest.h"
#include "DrawCallPerfParams.h"
#include "test_utils/draw_call_perf_utils.h"
#include "util/shader_utils.h"

#include <sstream>

namespace
{
enum class TestSubject
{
    NoChange,
    VertexAttribChange,
    VertexBufferChange,
    ManyVertexBuffersChange,
    TextureChange,
    ManyVaryings,
};

struct DrawArraysPerfParams : public DrawCallPerfParams
{
    DrawArraysPerfParams(const DrawCallPerfParams &base) : DrawCallPerfParams(base) {}

    std::string story() const override;

    TestSubject testSubject = TestSubject::NoChange;
};

std::string DrawArraysPerfParams::story() const
{
    std::stringstream strstr;

    strstr << DrawCallPerfParams::story();

    switch (testSubject)
    {
        case TestSubject::VertexAttribChange:
            strstr << "_attrib_change";
            break;
        case TestSubject::VertexBufferChange:
            strstr << "_vbo_change";
            break;
        case TestSubject::ManyVertexBuffersChange:
            strstr << "_manyvbos_change";
            break;
        case TestSubject::ManyVaryings:
            strstr << "_many_varyings";
            break;
        case TestSubject::TextureChange:
            strstr << "_tex_change";
            break;
        default:
            break;
    }

    return strstr.str();
}

std::ostream &operator<<(std::ostream &os, const DrawArraysPerfParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

GLuint CreateSimpleTexture2D(size_t textureSize)
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: textureSize x textureSize Image, 3 bytes per pixel (R, G, B)
    std::vector<GLubyte> pixels(textureSize * textureSize * 3);
    for (size_t i = 0; i < textureSize; ++i)
    {
        for (size_t j = 0; j < textureSize; ++j)
        {
            // Alternate between Red, Green and Blue
            pixels[i * textureSize * 3 + j * 3 + 0] = (i + j) % 4 == 0 ? 255 : 0;
            pixels[i * textureSize * 3 + j * 3 + 1] = (i + j) % 4 == 1 ? 255 : 0;
            pixels[i * textureSize * 3 + j * 3 + 2] = (i + j) % 4 == 2 ? 255 : 0;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 pixels.data());

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

class DrawCallPerfBenchmark : public ANGLERenderTest,
                              public ::testing::WithParamInterface<DrawArraysPerfParams>
{
  public:
    DrawCallPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram    = 0;
    GLuint mBuffer1    = 0;
    GLuint mBuffer2    = 0;
    GLuint mFBO        = 0;
    GLuint mFBOTexture = 0;
    GLuint mTexture1   = 0;
    GLuint mTexture2   = 0;
    int mNumTris       = GetParam().numTris;
};

DrawCallPerfBenchmark::DrawCallPerfBenchmark() : ANGLERenderTest("DrawCallPerf", GetParam()) {}

void DrawCallPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    if (params.testSubject == TestSubject::TextureChange)
    {
        mProgram = SetupSimpleTextureProgram();
    }
    else if (params.testSubject == TestSubject::ManyVertexBuffersChange)
    {
        constexpr char kVS[] = R"(attribute vec2 vPosition;
attribute vec2 v0;
attribute vec2 v1;
attribute vec2 v2;
attribute vec2 v3;
const float scale = 0.5;
const float offset = -0.5;

varying vec2 v;

void main()
{
    gl_Position = vec4(vPosition * vec2(scale) + vec2(offset), 0, 1);
    v = (v0 + v1 + v2 + v3) * 0.25;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec2 v;
void main()
{
    gl_FragColor = vec4(v, 0, 1);
})";

        mProgram = CompileProgram(kVS, kFS);
        glBindAttribLocation(mProgram, 1, "v0");
        glBindAttribLocation(mProgram, 2, "v1");
        glBindAttribLocation(mProgram, 3, "v2");
        glBindAttribLocation(mProgram, 4, "v3");
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }
    if (params.testSubject == TestSubject::ManyVaryings)
    {
        GLint maxVaryings = 0;
        glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);
        ASSERT_GL_NO_ERROR();

        // TODO(syoussefi): remove when http://anglebug.com/4273 is fixed.
        --maxVaryings;

        std::ostringstream vs, fs;

        fs << "precision mediump float;\n";

        for (GLint varyingIndex = 0; varyingIndex < maxVaryings; ++varyingIndex)
        {
            vs << "varying vec4 v" << varyingIndex << ";\n";
            fs << "varying vec4 v" << varyingIndex << ";\n";
        }

        // Make the vertex shader also substantially heavy, so it doesn't near-instantaneously
        // terminate and keep memory reserved for varying export.
        vs << "uniform sampler2D sampler0;\n";
        vs << "uniform sampler2D sampler1;\n";
        vs << "uniform float nonConstZero;\n";

        vs << R"(
void main()
{
    // Output zero position so the fragment shader is not called at all.
    gl_Position = vec4(0);
)";
        fs << R"(
void main()
{
    vec4 sum = vec4(0);
)";

        // Make only a portion of the varyings active.  If ANGLE or driver don't optimize away the
        // inactive varyings, there could be a performance penalty.
        const GLint activeVaryingCount = maxVaryings / 2;
        for (GLint varyingIndex = 0; varyingIndex < maxVaryings; ++varyingIndex)
        {
            GLint index        = varyingIndex % activeVaryingCount;
            const char *assign = varyingIndex < activeVaryingCount ? "=" : "+=";
            vs << "v" << index << " " << assign << " texture2D(sampler" << (index % 2)
               << ", vec2(mod(nonConstZero + " << index << ".0, 15.0) / " << maxVaryings
               << ".0, mod(nonConstZero + " << index << ".0, 17.0) / " << maxVaryings << ".0));\n";
            fs << "sum += v" << index << ";\n";
        }

        vs << "}";
        fs << R"(
    gl_FragColor = sum;
}
)";

        mProgram = CompileProgram(vs.str().c_str(), fs.str().c_str());
    }
    else
    {
        mProgram = SetupSimpleDrawProgram();
    }

    ASSERT_NE(0u, mProgram);

    // Re-link program to ensure the attrib bindings are used.
    glBindAttribLocation(mProgram, 0, "vPosition");
    glLinkProgram(mProgram);
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    mBuffer1 = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);
    mBuffer2 = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    if (params.offscreen)
    {
        CreateColorFBO(getWindow()->getWidth(), getWindow()->getHeight(), &mFBOTexture, &mFBO);
    }

    const size_t textureSize = params.testSubject == TestSubject::ManyVaryings ? 2048 : 2;
    mTexture1                = CreateSimpleTexture2D(textureSize);
    mTexture2                = CreateSimpleTexture2D(textureSize);

    if (params.testSubject == TestSubject::ManyVaryings)
    {
        GLint uniformLocation = glGetUniformLocation(mProgram, "nonConstZero");
        GLint sampler0        = glGetUniformLocation(mProgram, "sampler0");
        GLint sampler1        = glGetUniformLocation(mProgram, "sampler0");
        ASSERT_NE(uniformLocation, -1);
        ASSERT_NE(sampler0, -1);
        ASSERT_NE(sampler1, -1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mTexture2);

        glUniform1f(uniformLocation, 0.0f);
        glUniform1i(sampler0, 0);
        glUniform1i(sampler1, 0);
    }

    ASSERT_GL_NO_ERROR();
}

void DrawCallPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer1);
    glDeleteBuffers(1, &mBuffer2);
    glDeleteTextures(1, &mFBOTexture);
    glDeleteTextures(1, &mTexture1);
    glDeleteTextures(1, &mTexture2);
    glDeleteFramebuffers(1, &mFBO);
}

void ClearThenDraw(unsigned int iterations, GLsizei numElements)
{
    glClear(GL_COLOR_BUFFER_BIT);

    for (unsigned int it = 0; it < iterations; it++)
    {
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void JustDraw(unsigned int iterations, GLsizei numElements)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

template <int kArrayBufferCount>
void ChangeVertexAttribThenDraw(unsigned int iterations, GLsizei numElements, GLuint buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    for (unsigned int it = 0; it < iterations; it++)
    {
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}
template <int kArrayBufferCount>
void ChangeArrayBuffersThenDraw(unsigned int iterations,
                                GLsizei numElements,
                                GLuint buffer1,
                                GLuint buffer2)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer1);
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        glBindBuffer(GL_ARRAY_BUFFER, buffer2);
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void ChangeTextureThenDraw(unsigned int iterations,
                           GLsizei numElements,
                           GLuint texture1,
                           GLuint texture2)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glBindTexture(GL_TEXTURE_2D, texture1);
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        glBindTexture(GL_TEXTURE_2D, texture2);
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void DrawCallPerfBenchmark::drawBenchmark()
{
    // This workaround fixes a huge queue of graphics commands accumulating on the GL
    // back-end. The GL back-end doesn't have a proper NULL device at the moment.
    // TODO(jmadill): Remove this when/if we ever get a proper OpenGL NULL device.
    const auto &eglParams = GetParam().eglParameters;
    const auto &params    = GetParam();
    GLsizei numElements   = static_cast<GLsizei>(3 * mNumTris);

    startGpuTimer();

    switch (params.testSubject)
    {
        case TestSubject::VertexAttribChange:
            ChangeVertexAttribThenDraw<1>(params.iterationsPerStep, numElements, mBuffer1);
            break;
        case TestSubject::VertexBufferChange:
            ChangeArrayBuffersThenDraw<1>(params.iterationsPerStep, numElements, mBuffer1,
                                          mBuffer2);
            break;
        case TestSubject::ManyVertexBuffersChange:
            ChangeArrayBuffersThenDraw<5>(params.iterationsPerStep, numElements, mBuffer1,
                                          mBuffer2);
            break;
        case TestSubject::TextureChange:
            ChangeTextureThenDraw(params.iterationsPerStep, numElements, mTexture1, mTexture2);
            break;
        case TestSubject::NoChange:
        case TestSubject::ManyVaryings:
            if (eglParams.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE ||
                (eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE &&
                 eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE))
            {
                ClearThenDraw(params.iterationsPerStep, numElements);
            }
            else
            {
                JustDraw(params.iterationsPerStep, numElements);
            }
            break;
    }

    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

TEST_P(DrawCallPerfBenchmark, Run)
{
    run();
}

DrawArraysPerfParams DrawArrays(const DrawCallPerfParams &base, TestSubject testSubject)
{
    DrawArraysPerfParams params(base);
    params.testSubject = testSubject;
    if (testSubject == TestSubject::ManyVaryings)
    {
        // Increase the number of triangles to fully occupy GPU cores, putting pressure on the
        // shader export cache.
        params.numTris      = 1000;
        params.trackGpuTime = true;
    }
    return params;
}

using namespace params;

ANGLE_INSTANTIATE_TEST(
    DrawCallPerfBenchmark,
    DrawArrays(DrawCallD3D9(), TestSubject::NoChange),
    DrawArrays(NullDevice(DrawCallD3D9()), TestSubject::NoChange),
    DrawArrays(DrawCallD3D11(), TestSubject::NoChange),
    DrawArrays(NullDevice(DrawCallD3D11()), TestSubject::NoChange),
    DrawArrays(NullDevice(Offscreen(DrawCallD3D11())), TestSubject::NoChange),
    DrawArrays(DrawCallD3D11(), TestSubject::VertexAttribChange),
    DrawArrays(NullDevice(DrawCallD3D11()), TestSubject::VertexAttribChange),
    DrawArrays(DrawCallD3D11(), TestSubject::VertexBufferChange),
    DrawArrays(NullDevice(DrawCallD3D11()), TestSubject::VertexBufferChange),
    DrawArrays(DrawCallD3D11(), TestSubject::TextureChange),
    DrawArrays(NullDevice(DrawCallD3D11()), TestSubject::TextureChange),
    DrawArrays(DrawCallD3D11(), TestSubject::ManyVaryings),
    DrawArrays(NullDevice(DrawCallD3D11()), TestSubject::ManyVaryings),
    DrawArrays(DrawCallOpenGL(), TestSubject::NoChange),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::NoChange),
    DrawArrays(NullDevice(Offscreen(DrawCallOpenGL())), TestSubject::NoChange),
    DrawArrays(DrawCallOpenGL(), TestSubject::VertexAttribChange),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::VertexAttribChange),
    DrawArrays(DrawCallOpenGL(), TestSubject::VertexBufferChange),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::VertexBufferChange),
    DrawArrays(DrawCallOpenGL(), TestSubject::ManyVertexBuffersChange),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::ManyVertexBuffersChange),
    DrawArrays(DrawCallOpenGL(), TestSubject::TextureChange),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::TextureChange),
    DrawArrays(DrawCallOpenGL(), TestSubject::ManyVaryings),
    DrawArrays(NullDevice(DrawCallOpenGL()), TestSubject::ManyVaryings),
    DrawArrays(DrawCallValidation(), TestSubject::NoChange),
    DrawArrays(DrawCallVulkan(), TestSubject::NoChange),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::NoChange),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::NoChange),
    DrawArrays(DrawCallVulkan(), TestSubject::VertexAttribChange),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::VertexAttribChange),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::VertexAttribChange),
    DrawArrays(DrawCallVulkan(), TestSubject::VertexBufferChange),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::VertexBufferChange),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::VertexBufferChange),
    DrawArrays(DrawCallVulkan(), TestSubject::ManyVertexBuffersChange),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::ManyVertexBuffersChange),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::ManyVertexBuffersChange),
    DrawArrays(DrawCallVulkan(), TestSubject::TextureChange),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::TextureChange),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::TextureChange),
    DrawArrays(DrawCallVulkan(), TestSubject::ManyVaryings),
    DrawArrays(Offscreen(DrawCallVulkan()), TestSubject::ManyVaryings),
    DrawArrays(NullDevice(DrawCallVulkan()), TestSubject::ManyVaryings),
    DrawArrays(DrawCallWGL(), TestSubject::NoChange),
    DrawArrays(Offscreen(DrawCallWGL()), TestSubject::NoChange),
    DrawArrays(DrawCallWGL(), TestSubject::VertexAttribChange),
    DrawArrays(Offscreen(DrawCallWGL()), TestSubject::VertexAttribChange),
    DrawArrays(DrawCallWGL(), TestSubject::VertexBufferChange),
    DrawArrays(Offscreen(DrawCallWGL()), TestSubject::VertexBufferChange),
    DrawArrays(DrawCallWGL(), TestSubject::TextureChange),
    DrawArrays(Offscreen(DrawCallWGL()), TestSubject::TextureChange),
    DrawArrays(DrawCallWGL(), TestSubject::ManyVaryings),
    DrawArrays(Offscreen(DrawCallWGL()), TestSubject::ManyVaryings));

}  // anonymous namespace
