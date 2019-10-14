//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class SamplerVideoWEBGLTest : public ANGLETest
{
  protected:
    SamplerVideoWEBGLTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test to verify the a shader can have a sampler unused in a vertex shader
// but used in the fragment shader.
TEST_P(SamplerVideoWEBGLTest, VerifySamplerInBothVertexAndFragmentShaders)
{
    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(

#extension GL_WEBGL_video_texture : require
precision mediump float;
varying mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;
void main()
{
    gl_FragColor = texture2D(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, redColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(SamplerVideoWEBGLTest, ES2_D3D11(), ES2_OPENGL());
}  // namespace
