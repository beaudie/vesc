//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerMultisample_test.cpp:
// Tests compiling shaders that use gsampler2DMS types
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class SamplerVideoWEBGLTest : public ShaderCompileTreeTest
{
  public:
    SamplerVideoWEBGLTest() {}

    void initResources(ShBuiltInResources *resources) override
    {
        resources->WEBGL_video_texture = 1;
    }

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL_SPEC; }
};

// Checks whether compiler has parsed the samplerVideoWEBGL, texture2D correctly.
TEST_F(SamplerVideoWEBGLTest, Texture2D)
{
    constexpr char kShaderString[] =
        R"(
        #extension GL_WEBGL_video_texture : require
        precision mediump float;
        uniform mediump samplerVideoWEBGL s;

        void main()
        {
            gl_FragColor = texture2D(s, vec2(0.0, 0.0));
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
