//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerMultisample_test.cpp:
// Tests compiling shaders that use gsampler2DMS types
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class SamplerMultisampleTest : public ShaderCompileTreeTest
{
  public:
    SamplerMultisampleTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// checks whether compiler has parsed the gsampler2DMS, texelfetch qualifiers correctly
TEST_F(SamplerMultisampleTest, TexelFetchSampler2DMSQualifier)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// checks whether compiler has parsed the gsampler2DMS, texturesize qualifiers correctly
TEST_F(SamplerMultisampleTest, TextureSizeSampler2DMSQualifier)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// checks gsampler2DMS has no default precision
TEST_F(SamplerMultisampleTest, NoPrecisionSampler2DMS)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        uniform sampler2DMS s;
        uniform isampler2DMS is;
        uniform usampler2DMS us;

        void main() {})";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

class SamplerMultisampleEXTTest : public SamplerMultisampleTest
{
  public:
    SamplerMultisampleEXTTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->ARB_texture_multisample = 1;
    }

    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_SPEC; }
};

// checks ARB_texture_multisample is supported in es 3.0
TEST_F(SamplerMultisampleEXTTest, TextureMultisampleEXTEnabled)
{
    const std::string &shaderString =
        R"(#version 300 es
        #extension GL_ARB_texture_multisample : require
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting success:\n" << mInfoLog;
    }
}

TEST_F(SamplerMultisampleEXTTest, TextureMultisampleEXTDisabled)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation success, expecting failure:\n" << mInfoLog;
    }
}
