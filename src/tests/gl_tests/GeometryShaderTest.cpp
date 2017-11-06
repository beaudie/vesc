//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GeometryShaderTest.cpp : Tests of the implementation of geometry shader

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class GeometryShaderTest : public ANGLETest
{
};

// Verify that Geometry Shader can be created and attached to a program.
TEST_P(GeometryShaderTest, CreateAndAttachGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!extensionEnabled("GL_OES_geometry_shader"));

    ASSERT_TRUE(extensionEnabled("GL_EXT_geometry_shader"));

    const std::string &geometryShaderSource =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (invocations = 3, triangles) in;\n"
        "layout (triangle_strip, max_vertices = 3) out;\n"
        "in vec4 texcoord[];\n"
        "out vec4 o_texcoord;\n"
        "void main()\n"
        "{\n"
        "    int n;\n"
        "    for (n = 0; n < gl_in.length(); n++)\n"
        "    {\n"
        "        gl_Position = gl_in[n].gl_Position;\n"
        "        gl_Layer   = gl_InvocationID;\n"
        "        o_texcoord = texcoord[n];\n"
        "        EmitVertex();\n"
        "    }\n"
        "    EndPrimitive();\n"
        "}";

    GLuint geometryShader = CompileShader(GL_GEOMETRY_SHADER_OES, geometryShaderSource);

    EXPECT_NE(0u, geometryShader);

    GLuint programID = glCreateProgram();
    glAttachShader(programID, geometryShader);

    glDetachShader(programID, geometryShader);
    glDeleteShader(geometryShader);
    glDeleteProgram(programID);

    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(GeometryShaderTest, ES31_OPENGL(), ES31_OPENGLES(), ES31_D3D11());
}
