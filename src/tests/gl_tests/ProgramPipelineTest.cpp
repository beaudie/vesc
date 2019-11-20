//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineTest:
//   Various tests related to Program Pipeline.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ProgramPipelineTest : public ANGLETest
{
  protected:
    ProgramPipelineTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void drawQuad(const std::string &positionAttribName,
                  const GLfloat positionAttribZ,
                  const GLfloat positionAttribXYScale);

    GLuint vertProg;
    GLuint fragProg;
};

// Verify that program pipeline is not supported in version lower than ES31.
TEST_P(ProgramPipelineTest, GenerateProgramPipelineObject)
{
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_NO_ERROR();

        glDeleteProgramPipelines(1, &pipeline);
        EXPECT_GL_NO_ERROR();
    }
}

class ProgramPipelineTest31 : public ProgramPipelineTest
{};

// Test generate or delete program pipeline.
TEST_P(ProgramPipelineTest31, GenOrDeleteProgramPipelineTest)
{
    GLuint pipeline;
    glGenProgramPipelines(-1, &pipeline);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glGenProgramPipelines(0, &pipeline);
    EXPECT_GL_NO_ERROR();

    glDeleteProgramPipelines(-1, &pipeline);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDeleteProgramPipelines(0, &pipeline);
    EXPECT_GL_NO_ERROR();
}

// Test BindProgramPipeline.
TEST_P(ProgramPipelineTest31, BindProgramPipelineTest)
{
    glBindProgramPipeline(0);
    EXPECT_GL_NO_ERROR();

    glBindProgramPipeline(2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    glDeleteProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test IsProgramPipeline
TEST_P(ProgramPipelineTest31, IsProgramPipelineTest)
{
    EXPECT_GL_FALSE(glIsProgramPipeline(0));
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(glIsProgramPipeline(2));
    EXPECT_GL_NO_ERROR();

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_TRUE(glIsProgramPipeline(pipeline));
    EXPECT_GL_NO_ERROR();

    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
    EXPECT_GL_FALSE(glIsProgramPipeline(pipeline));
    EXPECT_GL_NO_ERROR();
}

#if 0  // TIMTIM
std::string QueryErrorMessage(GLuint program)
{
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    EXPECT_GL_NO_ERROR();

    if (infoLogLength >= 1)
    {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        EXPECT_GL_NO_ERROR();
        return infoLog.data();
    }

    return "";
}
#endif

// Simulates a call to glCreateShaderProgramv()
GLuint createShaderProgram(GLenum type, const GLchar *shaderString)
{
    GLShader shader(type);
    if (!shader.get())
    {
        return 0;
    }

    glShaderSource(shader, 1, &shaderString, nullptr);
    glCompileShader(shader);

    GLuint program = glCreateProgram();

    if (program)
    {
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        if (compiled)
        {
            glAttachShader(program, shader);
            glLinkProgram(program);
            //            glDetachShader(program, shader);
        }
    }

    EXPECT_GL_NO_ERROR();

    return program;
}

void ProgramPipelineTest::drawQuad(const std::string &positionAttribName,
                                   const GLfloat positionAttribZ,
                                   const GLfloat positionAttribXYScale)
{
    glUseProgram(0);

    std::array<Vector3, 6> quadVertices = ANGLETestBase::GetQuadVertices();

    for (Vector3 &vertex : quadVertices)
    {
        vertex.x() *= positionAttribXYScale;
        vertex.y() *= positionAttribXYScale;
        vertex.z() = positionAttribZ;
    }

    GLint positionLocation = glGetAttribLocation(vertProg, positionAttribName.c_str());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
}

// Test glUseProgramStages
TEST_P(ProgramPipelineTest31, UseProgramStages)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Red();

    vertProg = createShaderProgram(GL_VERTEX_SHADER, vertString);
    ASSERT_NE(vertProg, 0u);
    fragProg = createShaderProgram(GL_FRAGMENT_SHADER, fragString);
    ASSERT_NE(fragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    ProgramPipelineTest::drawQuad("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test glUseProgramStages
TEST_P(ProgramPipelineTest31, UseCreateShaderProgramv)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Red();

    vertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(vertProg, 0u);
    fragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(fragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    ProgramPipelineTest::drawQuad("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test glUniform
TEST_P(ProgramPipelineTest31, FragmentStageUniformTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

#if 0  // TIMTIM
    vertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(vertProg, 0u);
    fragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(fragProg, 0u);
#else
    vertProg = createShaderProgram(GL_VERTEX_SHADER, vertString);
    ASSERT_NE(vertProg, 0u);
    fragProg = createShaderProgram(GL_FRAGMENT_SHADER, fragString);
    ASSERT_NE(fragProg, 0u);
#endif

    // Generate a program pipeline and attach the programs to their respective stages
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    // Set the output color to red
    GLint location = glGetUniformLocation(fragProg, "redColorIn");
    glActiveShaderProgram(pipeline, fragProg);
    glUniform1f(location, 1.0);
    location = glGetUniformLocation(fragProg, "greenColorIn");
    glActiveShaderProgram(pipeline, fragProg);
    glUniform1f(location, 1.0);

    ProgramPipelineTest::drawQuad("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(ProgramPipelineTest);
ANGLE_INSTANTIATE_TEST_ES31(ProgramPipelineTest31);

}  // namespace
