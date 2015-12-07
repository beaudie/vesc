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
using namespace egl_platform;

namespace
{

float randFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float randFloatNegativeOneToOne()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
}

float f(float t)
{
    float l = t / 2.0f;
    float f = l - floor(l);
    return (f > 0.5f ? 1.0f - f : f) * 4.0f - 1.0f;
}

template <typename T>
size_t VectorSizeBytes(const std::vector<T> &vec)
{
    return sizeof(T) * vec.size();
}

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

std::ostream &operator<<(std::ostream &os, const InstancingPerfParams &params)
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
    std::vector<Vector3> mTranslateData;
    std::vector<float> mSizeData;
    std::vector<Vector3> mColorData;
};

InstancingPerfBenchmark::InstancingPerfBenchmark()
    : ANGLERenderTest("InstancingPerf", GetParam()), mProgram(0), mNumPoints(75000)
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

    std::vector<GLuint> indexData;
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

    for (GLuint pointIndex = 0; pointIndex < mNumPoints; ++pointIndex)
    {
        Vector3 randVec(randFloatNegativeOneToOne(), randFloatNegativeOneToOne(),
                        randFloatNegativeOneToOne());
        mTranslateData.push_back(randVec);
    }

    mSizeData.resize(mNumPoints, 0.012f);
    mColorData.resize(mNumPoints, Vector3(1.0f, 0.0f, 0.0f));

    mBuffers.resize(5, 0);
    glGenBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);

    // Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, VectorSizeBytes(indexData), &indexData[0],
                 GL_STATIC_DRAW);

    // Position Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(positionData), &positionData[0], GL_STATIC_DRAW);
    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 8, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Translate Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mTranslateData), &mTranslateData[0],
                 GL_STATIC_DRAW);
    GLint translateLocation = glGetAttribLocation(mProgram, "aTranslate");
    ASSERT_NE(-1, translateLocation);
    glVertexAttribPointer(translateLocation, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(translateLocation);
    glVertexAttribDivisorANGLE(translateLocation, 1);

    // Scale Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mSizeData), nullptr, GL_DYNAMIC_DRAW);
    GLint scaleLocation = glGetAttribLocation(mProgram, "aScale");
    ASSERT_NE(-1, scaleLocation);
    glVertexAttribPointer(scaleLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(scaleLocation);
    glVertexAttribDivisorANGLE(scaleLocation, 1);

    // Color Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mColorData), nullptr, GL_DYNAMIC_DRAW);
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
    Matrix4 worldMatrix = Matrix4::translate(Vector3(0, 0, -3.0f));
    worldMatrix *= Matrix4::rotate(25.0f, Vector3(0.6f, 1.0f, 0.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix.data[0]);

    GLint projectionMatrixLocation = glGetUniformLocation(mProgram, "uProjectionMatrix");
    ASSERT_NE(-1, projectionMatrixLocation);
    float fov =
        static_cast<float>(getWindow()->getWidth()) / static_cast<float>(getWindow()->getHeight());
    Matrix4 projectionMatrix = Matrix4::perspective(60.0f, fov, 1.0f, 300.0f);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix.data[0]);

    getWindow()->setVisible(true);

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

    // float time = static_cast<float>(mTimer->getElapsedTime());

    // for (size_t pointIndex = 0; pointIndex < mTranslateData.size(); ++pointIndex)
    //{
    //    const Vector3 &translate = mTranslateData[pointIndex];

    //    float tx = translate.x + time;
    //    float ty = translate.y + time;
    //    float tz = translate.z + time;

    //    float scale = f(tx) * 0.01f + 0.01f;
    //    mSizeData[pointIndex] = scale;

    //    Vector3 color;
    //    color.x = f(tx) * 0.5f + 0.5f;
    //    color.y = f(ty) * 0.5f + 0.5f;
    //    color.z = f(tz) * 0.5f + 0.5f;

    //    mColorData[pointIndex] = color;
    //}

    // Update scales and colors.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VectorSizeBytes(mSizeData), &mSizeData[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VectorSizeBytes(mColorData), &mColorData[0]);

    // Render the instances.
    for (unsigned int it = 0; it < params.iterations; it++)
    {
        glDrawElementsInstancedANGLE(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, mNumPoints);
    }

    ASSERT_GL_NO_ERROR();
}

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

struct BillboardPerfParams final : public RenderTestParams
{
    // Common default options
    BillboardPerfParams()
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

std::ostream &operator<<(std::ostream &os, const BillboardPerfParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class BillboardPerfBenchmark : public ANGLERenderTest,
                               public ::testing::WithParamInterface<BillboardPerfParams>
{
  public:
    BillboardPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    std::vector<GLuint> mBuffers;
    GLuint mNumPoints;
    std::vector<Vector3> mTranslateData;
    std::vector<float> mSizeData;
    std::vector<Vector3> mColorData;
};

BillboardPerfBenchmark::BillboardPerfBenchmark()
    : ANGLERenderTest("BillboardingPerf", GetParam()), mProgram(0), mNumPoints(75000)
{
    mRunTimeSeconds = GetParam().runTimeSeconds;
}

void BillboardPerfBenchmark::initializeBenchmark()
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

    GLuint baseIndexData[6]     = {0, 1, 2, 1, 3, 2};
    Vector2 basePositionData[4] = {Vector2(-1.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(-1.0f, -1.0f),
                                   Vector2(1.0f, -1.0f)};

    std::vector<GLuint> indexData;
    std::vector<Vector2> positionData;
    GLuint pointVertexStride = 4;
    for (GLuint pointIndex = 0; pointIndex < mNumPoints; ++pointIndex)
    {
        for (GLuint indexIndex = 0; indexIndex < 6; ++indexIndex)
        {
            indexData.push_back(baseIndexData[indexIndex] + pointIndex * pointVertexStride);
        }

        Vector3 randVec(randFloatNegativeOneToOne(), randFloatNegativeOneToOne(),
                        randFloatNegativeOneToOne());
        for (GLuint vertexIndex = 0; vertexIndex < 4; ++vertexIndex)
        {
            positionData.push_back(basePositionData[vertexIndex]);
            mTranslateData.push_back(randVec);
        }
    }

    mSizeData.resize(mNumPoints * 4, 0.012f);
    mColorData.resize(mNumPoints * 4, Vector3(1.0f, 0.0f, 0.0f));

    mBuffers.resize(5, 0);
    glGenBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);

    // Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, VectorSizeBytes(indexData), &indexData[0],
                 GL_STATIC_DRAW);

    // Position Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(positionData), &positionData[0], GL_STATIC_DRAW);
    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 8, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Translate Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mTranslateData), &mTranslateData[0],
                 GL_STATIC_DRAW);
    GLint translateLocation = glGetAttribLocation(mProgram, "aTranslate");
    ASSERT_NE(-1, translateLocation);
    glVertexAttribPointer(translateLocation, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(translateLocation);

    // Scale Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mSizeData), nullptr, GL_DYNAMIC_DRAW);
    GLint scaleLocation = glGetAttribLocation(mProgram, "aScale");
    ASSERT_NE(-1, scaleLocation);
    glVertexAttribPointer(scaleLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(scaleLocation);

    // Color Data
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferData(GL_ARRAY_BUFFER, VectorSizeBytes(mColorData), nullptr, GL_DYNAMIC_DRAW);
    GLint colorLocation = glGetAttribLocation(mProgram, "aColor");
    ASSERT_NE(-1, colorLocation);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 12, nullptr);
    glEnableVertexAttribArray(colorLocation);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    // Init matrices
    GLint worldMatrixLocation = glGetUniformLocation(mProgram, "uWorldMatrix");
    ASSERT_NE(-1, worldMatrixLocation);
    Matrix4 worldMatrix = Matrix4::translate(Vector3(0, 0, -3.0f));
    worldMatrix *= Matrix4::rotate(25.0f, Vector3(0.6f, 1.0f, 0.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix.data[0]);

    GLint projectionMatrixLocation = glGetUniformLocation(mProgram, "uProjectionMatrix");
    ASSERT_NE(-1, projectionMatrixLocation);
    float fov =
        static_cast<float>(getWindow()->getWidth()) / static_cast<float>(getWindow()->getHeight());
    Matrix4 projectionMatrix = Matrix4::perspective(60.0f, fov, 1.0f, 300.0f);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix.data[0]);

    getWindow()->setVisible(true);

    ASSERT_GL_NO_ERROR();
}

void BillboardPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);

    if (!mBuffers.empty())
    {
        glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);
        mBuffers.clear();
    }
}

void BillboardPerfBenchmark::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void BillboardPerfBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    // This code makes the test more interesting visually, but also eats up many CPU cycles.
    // float time = static_cast<float>(mTimer->getElapsedTime());

    // for (size_t pointIndex = 0; pointIndex < mTranslateData.size(); ++pointIndex)
    //{
    //    const Vector3 &translate = mTranslateData[pointIndex];

    //    float tx = translate.x + time;
    //    float ty = translate.y + time;
    //    float tz = translate.z + time;

    //    float scale = f(tx) * 0.01f + 0.01f;
    //    mSizeData[pointIndex] = scale;

    //    Vector3 color;
    //    color.x = f(tx) * 0.5f + 0.5f;
    //    color.y = f(ty) * 0.5f + 0.5f;
    //    color.z = f(tz) * 0.5f + 0.5f;

    //    mColorData[pointIndex] = color;
    //}

    // Update scales and colors.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VectorSizeBytes(mSizeData), &mSizeData[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[4]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VectorSizeBytes(mColorData), &mColorData[0]);

    // Render the instances.
    for (unsigned int it = 0; it < params.iterations; it++)
    {
        glDrawElements(GL_TRIANGLES, 6 * mNumPoints, GL_UNSIGNED_INT, nullptr);
    }

    ASSERT_GL_NO_ERROR();
}

BillboardPerfParams BillboardPerfD3D11Params(bool useNullDevice)
{
    BillboardPerfParams params;
    params.eglParameters = useNullDevice ? D3D11_NULL() : D3D11();
    return params;
}

BillboardPerfParams BillboardPerfD3D9Params(bool useNullDevice)
{
    BillboardPerfParams params;
    params.eglParameters = useNullDevice ? D3D9_NULL() : D3D9();
    return params;
}

BillboardPerfParams BillboardPerfOpenGLParams(bool useNullDevice)
{
    BillboardPerfParams params;
    params.eglParameters = useNullDevice ? OPENGL_NULL() : OPENGL();
    return params;
}

TEST_P(BillboardPerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(BillboardPerfBenchmark,
                       BillboardPerfD3D11Params(false),
                       BillboardPerfD3D9Params(false),
                       BillboardPerfOpenGLParams(false));

}  // anonymous namespace
