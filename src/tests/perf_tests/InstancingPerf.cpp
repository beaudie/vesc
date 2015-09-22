//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InstancingPerf:
//   Performance tests for ANGLE instanced draw calls.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "Matrix.h"
#include "shader_utils.h"
#include "Vector.h"

using namespace angle;

namespace
{

struct InstancingPerfParams final : public RenderTestParams
{
    // Common default options
    InstancingPerfParams()
    {
        majorVersion   = 2;
        minorVersion   = 0;
        windowWidth    = 256;
        windowHeight   = 256;
        iterations     = 1;
        runTimeSeconds = 10.0;
    }

    std::string suffix() const override
    {
        std::stringstream strstr;

        strstr << RenderTestParams::suffix();

        return strstr.str();
    }

    unsigned int iterations;
    double runTimeSeconds;
};

inline std::ostream &operator<<(std::ostream &os, const InstancingPerfParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class InstancingPerfBenchmark : public ANGLERenderTest,
                                public ::testing::WithParamInterface<InstancingPerfParams>
{
  public:
    InstancingPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    std::vector<GLuint> mBuffers;
    GLuint mNumPoints;
    std::vector<float> mSizeData;
    std::vector<Vector3> mColorData;
};

InstancingPerfBenchmark::InstancingPerfBenchmark()
    : ANGLERenderTest("InstancingPerf", GetParam()), mProgram(0), mNumPoints(50000)
{
    mRunTimeSeconds = GetParam().runTimeSeconds;
}

void InstancingPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_TRUE(params.iterations > 0);
    mDrawIterations = params.iterations;

    const std::string vs =
        "attribute vec2 aPosition;\n"
        "attribute vec3 aTranslate;\n"
        "attribute float aScale;\n"
        "attribute vec3 aColor;\n"
        "uniform mat4 uWorldMatrix;\n"
        "uniform mat4 uProjectionMatrix;\n"
        "varying vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    vec4 position = uWorldMatrix * vec4(aTranslate, 1.0);\n"
        "    position.xy += aPosition * aScale;\n"
        "    gl_Position = uProjectionMatrix * position;\n"
        "    vColor = aColor;\n"
        "}\n";

    const std::string fs =
        "precision mediump float;\n"
        "varying vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n";

    mProgram = CompileProgram(vs, fs);
    ASSERT_TRUE(mProgram != 0);

    // Use the program object
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    std::vector<GLushort> indexData;
    indexData.push_back(0);
    indexData.push_back(1);
    indexData.push_back(2);
    indexData.push_back(1);
    indexData.push_back(3);
    indexData.push_back(2);

    std::vector<GLfloat> positionData;
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(-1.0f);

    std::vector<GLfloat> translateData;
    for (GLuint pointIndex = 0; pointIndex < mNumPoints; ++pointIndex)
    {
        for (int dim = 0; dim < 3; ++dim)
        {
            float randFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            translateData.push_back(randFloat * 2.0f - 1.0f);
        }
    }

    mBuffers.resize(5, 0);
    glGenBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);

    // Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indexData.size(), &indexData[0],
                 GL_STATIC_DRAW);

    // Position Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 8, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Translate Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * translateData.size(), &translateData[0],
                 GL_STATIC_DRAW);
    GLint translateLocation = glGetAttribLocation(mProgram, "aTranslate");
    ASSERT_NE(-1, translateLocation);
    glVertexAttribPointer(translateLocation, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(translateLocation);
    glVertexAttribDivisorANGLE(translateLocation, 1);

    // Scale Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNumPoints * 1, nullptr, GL_DYNAMIC_DRAW);
    GLint scaleLocation = glGetAttribLocation(mProgram, "aScale");
    ASSERT_NE(-1, scaleLocation);
    glVertexAttribPointer(scaleLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(scaleLocation);
    glVertexAttribDivisorANGLE(scaleLocation, 1);

    // Color Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNumPoints * 3, nullptr, GL_DYNAMIC_DRAW);
    GLint colorLocation = glGetAttribLocation(mProgram, "aColor");
    ASSERT_NE(-1, colorLocation);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribDivisorANGLE(colorLocation, 1);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    // Init matrices
    GLint worldMatrixLocation = glGetUniformLocation(mProgram, "uWorldMatrix");
    ASSERT_NE(-1, worldMatrixLocation);
    Matrix4 worldMatrix = Matrix4::identity();
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix.data[0]);

    GLint projectionMatrixLocation = glGetUniformLocation(mProgram, "uProjectionMatrix");
    ASSERT_NE(-1, projectionMatrixLocation);
    Matrix4 projectionMatrix = Matrix4::perspective(0.5f, 1.0f, 1.0f, 1000.0f);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix.data[0]);

    mSizeData.resize(mNumPoints, 1.0f);
    mColorData.resize(mNumPoints, Vector3(1.0f, 0.0f, 0.0f));

    ASSERT_GL_NO_ERROR();
}

void InstancingPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);

    if (!mBuffers.empty())
    {
        glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);
        mBuffers.clear();
    }
}

void InstancingPerfBenchmark::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void InstancingPerfBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    // Update scales and colors.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * mSizeData.size(), &mSizeData[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3) * mColorData.size(), &mColorData[0]);

    // Render the instances.
    for (unsigned int it = 0; it < params.iterations; it++)
    {
        glDrawElementsInstancedANGLE(GL_TRIANGLES, 4, GL_UNSIGNED_SHORT, nullptr, mNumPoints);
    }

    ASSERT_GL_NO_ERROR();
}

using namespace egl_platform;

InstancingPerfParams InstancingPerfD3D11Params(bool useNullDevice)
{
    InstancingPerfParams params;
    params.eglParameters = useNullDevice ? D3D11_NULL() : D3D11();
    return params;
}

InstancingPerfParams InstancingPerfD3D9Params(bool useNullDevice)
{
    InstancingPerfParams params;
    params.eglParameters = useNullDevice ? D3D9_NULL() : D3D9();
    return params;
}

InstancingPerfParams InstancingPerfOpenGLParams(bool useNullDevice)
{
    InstancingPerfParams params;
    params.eglParameters = useNullDevice ? OPENGL_NULL() : OPENGL();
    return params;
}

TEST_P(InstancingPerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(InstancingPerfBenchmark,
                       InstancingPerfD3D11Params(false),
                       InstancingPerfD3D9Params(false),
                       InstancingPerfOpenGLParams(false));

}  // anonymous namespace
