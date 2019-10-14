//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class WEBGLVideoTextureTest : public ANGLETest
{
  protected:
    WEBGLVideoTextureTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class WEBGLVideoTextureES300Test : public ANGLETest
{
  protected:
    WEBGLVideoTextureES300Test()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test to verify samplerVideoWEBGL works fine when extension is enabled.
TEST_P(WEBGLVideoTextureTest, VerifySamplerVideoWEBGL)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

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
    gl_FragColor = textureVideoWEBGL(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to verify samplerVideoWEBGL works fine as parameter of user defined function
// when extension is enabled.
TEST_P(WEBGLVideoTextureTest, VerifySamplerVideoWEBGLAsParameter)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

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

vec4 wrapTextureVideoWEBGL(samplerVideoWEBGL sampler, vec2 coord)
{
    return textureVideoWEBGL(sampler, coord);
}

void main()
{
    gl_FragColor = wrapTextureVideoWEBGL(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to verify samplerVideoWEBGL works fine in ES300 when extension is enabled.
TEST_P(WEBGLVideoTextureES300Test, VerifySamplerVideoWEBGLInES300)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
out mediump vec2 texCoord;

void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_WEBGL_video_texture : require
precision mediump float;
in mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(WEBGLVideoTextureTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL());
ANGLE_INSTANTIATE_TEST(WEBGLVideoTextureES300Test, ES3_D3D11(), ES3_OPENGL());
}  // namespace
