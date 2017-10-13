//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class GeometryShaderTest : public ANGLETest
{
};

// Verify that Geometry Shader can be compiled and attached to a program.
// TODO(jiawei.shao@intel.com): test linkage of a program with geometry shader once it is
// implemented.
TEST_P(GeometryShaderTest, CompileAndAttachGeometryShader)
{
    const std::string &geometryShaderSource =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (invocations = 3, triangles) in;\n"
        "layout (triangle_strip, max_vertices = 3) out;\n"
        "void main()\n"
        "{\n"
        "    int n;\n"
        "    for (n = 0; n < gl_in.length(); n++)\n"
        "    {\n"
        "        gl_Position = gl_in[n].gl_Position;\n"
        "        gl_Layer   = gl_InvocationID;\n"
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

ANGLE_INSTANTIATE_TEST(GeometryShaderTest, ES31_OPENGL(), ES31_OPENGLES());

}  // anonymous namespace