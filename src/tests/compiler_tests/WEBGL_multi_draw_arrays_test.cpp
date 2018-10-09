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
#include "compiler/translator/tree_ops/EmulateGLDrawID.h"
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

  protected:
    void CheckEmulatesUniform(ShShaderOutput outputType)
    {
        const std::string &shaderString =
            "#extension GL_WEBGL_multi_draw_arrays : require\n"
            "void main() {\n"
            "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
            "}\n";

        sh::TCompiler *translator =
            sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC, outputType);
        if (!translator->Init(*getResources()))
        {
            SafeDelete(translator);
            FAIL() << "Failed to construct compiler";
            return;
        }

        const char *shaderStrings[] = {shaderString.c_str()};
        EXPECT_TRUE(translator->compile(shaderStrings, 1,
                                        SH_OBJECT_CODE | SH_VARIABLES | SH_EMULATE_GL_DRAW_ID));

        TInfoSink &infoSink        = translator->getInfoSink();
        std::string translatedCode = infoSink.obj.c_str();

        // gl_DrawID builtin should be replaced
        EXPECT_TRUE(translatedCode.find("float(gl_DrawID)") == std::string::npos);

        // Extension directive should be removed
        EXPECT_TRUE(translatedCode.find("GL_WEBGL_multi_draw_arrays") == std::string::npos);

        bool hasEmulatedUniform = false;
        for (const auto &uniform : translator->getUniforms())
        {
            if (uniform.name == sh::GetEmulatedGLDrawIDName())
            {
                hasEmulatedUniform = true;
            }
        }
        EXPECT_TRUE(hasEmulatedUniform);

        SafeDelete(translator);
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
        compileTestShader(GL_VERTEX_SHADER, SH_GLES2_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT,
                          shaderString, getResources(), SH_VARIABLES, &translatedCode, &infoLog);

    ASSERT_FALSE(success);
}

TEST_F(WEBGLMultiDrawArraysTest, EmulatesUniformGLSL)
{
    CheckEmulatesUniform(SH_GLSL_COMPATIBILITY_OUTPUT);
}

TEST_F(WEBGLMultiDrawArraysTest, EmulatesUniformESSL)
{
    CheckEmulatesUniform(SH_ESSL_OUTPUT);
}

TEST_F(WEBGLMultiDrawArraysTest, EmulatesUniformVulkanGLSL)
{
    CheckEmulatesUniform(SH_GLSL_VULKAN_OUTPUT);
}

#if defined(ANGLE_ENABLE_HLSL)
TEST_F(WEBGLMultiDrawArraysTest, EmulatesUniformHLSL)
{
    CheckEmulatesUniform(SH_HLSL_3_0_OUTPUT);
}
#endif
