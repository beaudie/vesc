//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerMultisample_test.cpp:
// tests gsampler2DMS
//

#include "angle_gl.h"
#include "compiler/translator/TranslatorESSL.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
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

// checks whether compiler has parsed the gsampler2DMS, texelfetch, qualifiers correctly
TEST_F(SamplerMultisampleTest, TexelFetchGsamplerMSQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform highp sampler2DMS s;\n"
        "uniform highp isampler2DMS is;\n"
        "uniform highp usampler2DMS us;\n"
        ""
        "void main() {\n"
        "    vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);\n"
        "    ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);\n"
        "    uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);\n"
        "}\n";

    if (compile(shaderString))
    {
        SUCCEED() << "Shader compilation succeeded\n";
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// checks whether compiler has parsed the gsampler2DMS, texturesize qualifiers correctly
TEST_F(SamplerMultisampleTest, TextureSizeGsamplerMSQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform highp sampler2DMS s;\n"
        "uniform highp isampler2DMS is;\n"
        "uniform highp usampler2DMS us;\n"
        ""
        "void main() {\n"
        "    ivec2 size = textureSize(s);\n"
        "    size = textureSize(is);\n"
        "    size = textureSize(us);\n"
        "}\n";

    if (compile(shaderString))
    {
        SUCCEED() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
