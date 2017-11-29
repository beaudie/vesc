//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LinkAndRelinkFailureTest:
//   Link and relink failure tests for rendering pipeline and compute pipeline.

#include <vector>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class LinkAndRelinkFailureTest : public ANGLETest
{
  protected:
    LinkAndRelinkFailureTest() {}
};

class LinkAndRelinkFailureTestES31 : public ANGLETest
{
  protected:
    LinkAndRelinkFailureTestES31() {}
};

// Link and relink fails, then try to install the unsuccessfully linked
// program and start rendering or dispatch compute. The result depends on
// the previous state: Whether a valid program has been installed in pipeline.

TEST_P(LinkAndRelinkFailureTest, RenderingProgramLinkFailureTest)
{
    // Link failure, and no valid program installed in the pipeline before.
    // It should report error for UseProgram and DrawArrays/DrawElements.
    glUseProgram(0);
    GLuint program = glCreateProgram();

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_FALSE, linkStatus);

    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(LinkAndRelinkFailureTest, RenderingProgramRelinkFailureTest)
{
    // Install a render program in the pipeline via UseProgram, then render.
    // It should success.
    const std::string vsSource =
        R"(#version 310 es
        void main()
        {
        })";

    const std::string fsSource =
        R"(#version 310 es
        void main()
        {
        })";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_TRUE, linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // Relink failure, but a valid program has been installed in the pipeline.

    // No vertex shader, relink fails.
    glDetachShader(program, vs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_FALSE, linkStatus);
    EXPECT_GL_NO_ERROR();

    // To start rendering should success.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // To use the unsuccessfully linked program, report error.
    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // To use the unsuccessfully linked program, that program should not
    // replace the program binary resided in pipeline. It will not make the
    // installed program invalid, like what UseProgram(0) can do.
    // So, to start rendering should success.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

TEST_P(LinkAndRelinkFailureTestES31, ComputeProgramLinkFailureTest)
{
    // Link failure, and no valid program installed in the pipeline before.
    // It should report error for UseProgram and DispatchCompute.
    glUseProgram(0);
    GLuint program = glCreateProgram();

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_FALSE, linkStatus);

    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(LinkAndRelinkFailureTestES31, ComputeProgramRelinkFailureTest)
{
    // Install a compute program in the pipeline via UseProgram, then dispatch
    // compute. It should success.
    const std::string csSource =
        R"(#version 310 es
        layout(local_size_x=1) in;
        void main()
        {
        })";

    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, csSource);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_TRUE, linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    // Relink failure, but a valid program has been installed in the pipeline.

    // No compute shader, relink fails.
    glDetachShader(program, cs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_FALSE, linkStatus);
    EXPECT_GL_NO_ERROR();

    // To dispatch compute should success.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    // To use the unsuccessfully linked program, report error.
    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // To use the unsuccessfully linked program, that program should not
    // replace the program binary resided in pipeline. It will not make the
    // installed program invalid, like what UseProgram(0) can do.
    // So, to dispatch compute should success.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(LinkAndRelinkFailureTest, ES2_OPENGL(), ES2_OPENGLES(), ES2_D3D9(), ES2_D3D11());
ANGLE_INSTANTIATE_TEST(LinkAndRelinkFailureTestES31, ES31_OPENGL(), ES31_OPENGLES());

}  // namespace
