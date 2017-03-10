//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramInterfaceTest: Tests of program interfaces.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ProgramInterfaceTestES31 : public ANGLETest
{
  protected:
    ProgramInterfaceTestES31()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override { ANGLETest::TearDown(); }
};

// Tests the properties of PROGRAM_INPUT, and PROGRAM_OUTPUT.
TEST_P(ProgramInterfaceTestES31, ProgramInputOutputProperties)
{
    const std::string &vertexShaderSource =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(location = 3)\n"
        "in highp vec4 position;\n"
        "in highp vec4 dummyIn;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "}";

    const std::string fragmentShaderSource =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "layout(location = 2) out vec4 oColor[3];\n"
        "layout(location = 1) out vec4 oColor1;\n"
        "void main()\n"
        "{\n"
        "    oColor[2] = color;\n"
        "    oColor1 = color;\n"
        "}";

    GLuint program = CompileProgram(vertexShaderSource, fragmentShaderSource);
    if (program == 0)
    {
        FAIL() << "shader compilation failed.";
    }
    GLint value;
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &value);
    EXPECT_EQ(value, 1);
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &value);
    EXPECT_EQ(value, 9);

    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &value);
    EXPECT_EQ(value, 2);
    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_MAX_NAME_LENGTH, &value);
    EXPECT_EQ(value, 10);

    GLuint index = glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, "position");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(index, GL_INVALID_INDEX);
    GLint params[7];
    GLsizei length;
    GLenum props[] = {GL_TYPE,
                      GL_ARRAY_SIZE,
                      GL_LOCATION,
                      GL_NAME_LENGTH,
                      GL_REFERENCED_BY_VERTEX_SHADER,
                      GL_REFERENCED_BY_FRAGMENT_SHADER,
                      GL_REFERENCED_BY_COMPUTE_SHADER};
    glGetProgramResourceiv(program, GL_PROGRAM_INPUT, index, 7, props, 7, &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(length, 7);
    EXPECT_EQ(params[0], GL_FLOAT_VEC4);
    EXPECT_EQ(params[1], 1);
    EXPECT_EQ(params[2], 3);
    EXPECT_EQ(params[3], 9);
    EXPECT_EQ(params[4], 1);
    EXPECT_EQ(params[5], 0);
    EXPECT_EQ(params[6], 0);

    GLchar name[64];
    glGetProgramResourceName(program, GL_PROGRAM_INPUT, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(length, 8);
    EXPECT_EQ(std::string(name), "position");

    GLint location;
    location = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "position");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(location, 3);

    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "oColor[0]");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(index, GL_INVALID_INDEX);
    glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, index, 7, props, 6, &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(length, 6);
    EXPECT_EQ(params[0], GL_FLOAT_VEC4);
    EXPECT_EQ(params[1], 3);
    EXPECT_EQ(params[2], 2);
    EXPECT_EQ(params[3], 10);
    EXPECT_EQ(params[4], 0);
    EXPECT_EQ(params[5], 1);

    glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(length, 9);
    EXPECT_EQ(std::string(name), "oColor[0]");

    location = glGetProgramResourceLocation(program, GL_PROGRAM_OUTPUT, "oColor1");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(location, 1);

    glDeleteProgram(program);
}

ANGLE_INSTANTIATE_TEST(ProgramInterfaceTestES31, ES31_OPENGL(), ES31_D3D11(), ES31_OPENGLES());
}
