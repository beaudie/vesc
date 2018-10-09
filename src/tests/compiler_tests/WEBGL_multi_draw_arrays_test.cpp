//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLE_draw_id.cpp:
//   Test for ANGLE_draw_id extension
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class WEBGLMultiDrawArraysTest : public MatchOutputCodeTest
{
  public:
    WEBGLMultiDrawArraysTest()
        : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_VARIABLES, SH_GLSL_COMPATIBILITY_OUTPUT)
    {
        getResources()->WEBGL_multi_draw_arrays = 1;
    }
};

TEST_F(WEBGLMultiDrawArraysTest, RequiresEmulation)
{
    const std::string &shaderString =
        "#extension GL_WEBGL_multi_draw_arrays : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n";

    std::string translatedCode;
    std::string infoLog;
    bool success =
        compileTestShader(GL_VERTEX_SHADER, SH_GLES3_1_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT,
                          shaderString, getResources(), SH_VARIABLES, &translatedCode, &infoLog);

    ASSERT_FALSE(success);
}

TEST_F(WEBGLMultiDrawArraysTest, EmulatesUniform)
{
    const std::string &shaderString =
        "#extension GL_WEBGL_multi_draw_arrays : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n";
    compile(shaderString, SH_EMULATE_GL_DRAW_ID);
    ASSERT_TRUE(foundInCode("uniform int"));
    ASSERT_FALSE(foundInCode("float(gl_DrawID)"));
}
