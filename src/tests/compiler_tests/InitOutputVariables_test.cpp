//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InitOutputVariables_test.cpp:
//   Tests for initializing output variables.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class InitVertexOutputVariablesTest : public MatchOutputCodeTest
{
  public:
    InitVertexOutputVariablesTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, 0, SH_ESSL_OUTPUT) {}
};

// Test that gl_Position is initialized once in case it is not statically used and both
// SH_INIT_OUTPUT_VARIABLES and SH_INIT_GL_POSITION flags are set.
TEST_F(InitVertexOutputVariablesTest, InitGLPositionWhenNotStaticallyUsed)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "void main() {\n"
        "}\n";
    compile(shaderString, SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES | SH_INIT_GL_POSITION);
    EXPECT_TRUE(foundInCode("gl_Position = ", 1));
}

// Test that gl_Position is initialized once in case it is statically used and both
// SH_INIT_OUTPUT_VARIABLES and SH_INIT_GL_POSITION flags are set.
TEST_F(InitVertexOutputVariablesTest, InitGLPositionOnceWhenStaticallyUsed)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 foo;\n"
        "void main() {\n"
        "    foo = gl_Position;\n"
        "}\n";
    compile(shaderString, SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES | SH_INIT_GL_POSITION);
    EXPECT_TRUE(foundInCode("gl_Position = ", 1));
}
