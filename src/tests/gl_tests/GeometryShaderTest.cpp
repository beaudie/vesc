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

TEST_P(GeometryShaderTest, CompileBasicGeometryShader)
{
    const std::string testGeometryShaderSource =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (triangles, invocations = 3) in;\n"
        "layout (triangle_strip, max_vertices = 3) out;\n"
        "in vec4 vs_normal[];\n"
        "in vec2 vs_texcoord[3];\n"
        "out vec4 gs_normal;\n"
        "out vec2 gs_texcoord;\n"
        "void main()\n"
        "{\n"
        "    int n;\n"
        "    for (n = 0; n < gl_in.length(); n++)\n"
        "    {\n"
        "        gl_Position = gl_in[n].gl_Position;\n"
        "        gs_normal = vs_normal[gl_InvocationID];\n"
        "        gl_Layer   = gl_InvocationID;\n"
        "        gs_texcoord = vs_texcoord[gl_InvocationID];\n"
        "        EmitVertex();\n"
        "    }\n"
        "    EndPrimitive();\n"
        "}";

    GLuint geometryShader = CompileShader(GL_GEOMETRY_SHADER_EXT, testGeometryShaderSource);

    EXPECT_NE(0u, geometryShader);
}

ANGLE_INSTANTIATE_TEST(GeometryShaderTest, ES31_OPENGL(), ES31_OPENGLES());

}  // anonymous namespace
