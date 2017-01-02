//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultiviewSideBySidePerf:
//   Performance test for side-by-side multiview rendering.
//

#include "ANGLEPerfTest.h"
#include "DrawCallPerfParams.h"
#include "shader_utils.h"
#include "test_utils/draw_call_perf_utils.h"

namespace
{

class MultiviewSideBySidePerfBenchmark : public ANGLERenderTest,
                                         public ::testing::WithParamInterface<DrawCallPerfParams>
{
  public:
    MultiviewSideBySidePerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram = 0;
    GLuint mBuffer  = 0;
    GLuint mFBO     = 0;
    GLuint mTexture = 0;
    int mNumTris    = GetParam().numTris;
};

MultiviewSideBySidePerfBenchmark::MultiviewSideBySidePerfBenchmark()
    : ANGLERenderTest("MultiviewSideBySidePerf", GetParam())
{
    mRunTimeSeconds = GetParam().runTimeSeconds;
}

void MultiviewSideBySidePerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_LT(0u, params.iterations);

    const std::string vs =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "in vec2 vPosition;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vPosition * 0.5 - 0.5, 0.0, 1.0);\n"
        "}\n";
    const std::string fs =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(gl_ViewID_OVR, 0.0, 0.0, 1.0);\n"
        "}\n";
    mProgram = CompileProgram(vs, fs);
    ASSERT_NE(0u, mProgram);

    // Use the program object
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    mBuffer = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth() / 2, getWindow()->getHeight());

    if (params.useFBO)
    {
        CreateColorFBO(getWindow()->getWidth(), getWindow()->getHeight(), &mTexture, &mFBO);
    }

    glDrawBufferSideBySideANGLE(GL_BACK);

    ASSERT_GL_NO_ERROR();
}

void MultiviewSideBySidePerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
    glDeleteTextures(1, &mTexture);
    glDeleteFramebuffers(1, &mFBO);
}

void MultiviewSideBySidePerfBenchmark::drawBenchmark()
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
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(3 * mNumTris));
    }

    ASSERT_GL_NO_ERROR();
}

TEST_P(MultiviewSideBySidePerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(MultiviewSideBySidePerfBenchmark,
                       DrawCallPerfOpenGLParamsGLES3(false, false),
                       DrawCallPerfOpenGLParamsGLES3(true, false),
                       DrawCallPerfOpenGLParamsGLES3(true, true));

}  // namespace
