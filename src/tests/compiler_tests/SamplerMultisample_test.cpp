//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerMultisample_test.cpp:
// tests gsampler2dms, gsampler2dmsarray
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class SamplerMultisampleTest : public testing::Test
{
  public:
    SamplerMultisampleTest() {}

  protected:
    void SetUp() override
    {
        ShBuiltInResources resources;
        InitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_FRAGMENT_SHADER, SH_GLES3_1_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    void TearDown() override { SafeDelete(mTranslator); }

    // Return true when compilation succeeds
    bool compile(const std::string &shaderString)
    {
        const char *shaderStrings[] = {shaderString.c_str()};
        bool status = mTranslator->compile(shaderStrings, 1, SH_INTERMEDIATE_TREE | SH_VARIABLES);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog            = infoSink.info.c_str();
        return status;
    }

  protected:
    std::string mInfoLog;
    TranslatorESSL *mTranslator = nullptr;
};

// checks whether compiler has parsed the gsampler2dms, texelfetch, qualifiers correctly
TEST_F(SamplerMultisampleTest, TexelFetchGsamplerMSQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform bool u;\n"
        "uniform sampler2DMS s;\n"
        "uniform isampler2DMS is;\n"
        "uniform usampler2DMS us;\n"
        ""
        "void main() {\n"
        "    vec4 tex1 = texelFetch(s, ivec2(gl_LocalInvocationID.xy));\n"
        "    ivec tex2 = texelFetch(is, ivec2(gl_LocalInvocationID.xy));\n"
        "    uivec tex3 = texelFetch(us, ivec2(gl_LocalInvocationID.xy));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// checks whether compiler has parsed the gsampler2dms, texturesize qualifiers correctly
TEST_F(SamplerMultisampleTest, TextureSizeGsamplerMSQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform bool u;\n"
        "uniform sampler2DMS s;\n"
        "uniform isampler2DMS is;\n"
        "uniform usampler2DMS us;\n"
        ""
        "void main() {\n"
        "    ivec2 size = textureSize(s);\n"
        "    size = textureSize(is);\n"
        "    size = textureSize(us);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
