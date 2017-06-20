//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawElementsPerf:
//   Performance tests for ANGLE DrawElements call overhead.
//

#include "ANGLEPerfTest.h"
#include "DrawCallPerfParams.h"
#include "test_utils/draw_call_perf_utils.h"

namespace
{

class DrawElementsPerfBenchmark : public ANGLERenderTest,
                                  public ::testing::WithParamInterface<DrawCallPerfParams>
{
  public:
    DrawElementsPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram     = 0;
    GLuint mBuffer      = 0;
    GLuint mIndexBuffer = 0;
    GLuint mFBO         = 0;
    GLuint mTexture     = 0;
    int mNumTris        = GetParam().numTris * 2;
    GLenum mType        = GetParam().type;
};

DrawElementsPerfBenchmark::DrawElementsPerfBenchmark()
    : ANGLERenderTest("DrawElementsPerf", GetParam())
{
    mRunTimeSeconds = GetParam().runTimeSeconds;
}

void DrawElementsPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_LT(0u, params.iterations);

    mProgram = SetupSimpleDrawProgram();
    ASSERT_NE(0u, mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    mBuffer      = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);
    mIndexBuffer = CreateIndexBuffer(mNumTris, mType, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    if (params.useFBO)
    {
        CreateColorFBO(getWindow()->getWidth(), getWindow()->getHeight(), &mTexture, &mFBO);
    }

    ASSERT_GL_NO_ERROR();
}

void DrawElementsPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &mIndexBuffer);
    glDeleteTextures(1, &mTexture);
    glDeleteFramebuffers(1, &mFBO);
}

void DrawElementsPerfBenchmark::drawBenchmark()
{
    // This workaround fixes a huge queue of graphics commands accumulating on the GL
    // back-end. The GL back-end doesn't have a proper NULL device at the moment.
    // TODO(jmadill): Remove this when/if we ever get a proper OpenGL NULL device.
    const auto &eglParams = GetParam().eglParameters;
    if (eglParams.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE ||
        (eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE &&
         eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE))
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    const auto &params = GetParam();

    for (unsigned int it = 0; it < params.iterations; it++)
    {
        std::vector<GLuint> indexData;
        int count = 3 * mNumTris;
        for (int i = 0; i < count; i++)
        {
            indexData.push_back(rand() % count);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(mType) * indexData.size(),
                        indexData.data());
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), mType, 0);
    }

    ASSERT_GL_NO_ERROR();
}

TEST_P(DrawElementsPerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(DrawElementsPerfBenchmark,
                       DrawCallPerfD3D11Params(false, false),
                       DrawCallPerfD3D11Params(true, false),
                       DrawCallPerfD3D11Params(true, true));

}  // namespace
