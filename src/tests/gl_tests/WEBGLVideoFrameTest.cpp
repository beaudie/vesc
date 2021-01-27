//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class WEBGLVideoFrameTest : public ANGLETest
{
  protected:
    WEBGLVideoFrameTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class WEBGLVideoFrameES300Test : public ANGLETest
{
  protected:
    WEBGLVideoFrameES300Test()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test to verify binding an unnamed texture to GL_TEXTURE_VIDEO_FRAME_WEBGL is NOT allowed.
TEST_P(WEBGLVideoFrameTest, MustNotBindUnnamedTexture)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, 1u);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test to verify binding a new texture to GL_TEXTURE_VIDEO_FRAME_WEBGL is NOT allowed.
TEST_P(WEBGLVideoFrameTest, MustNotBindNewTexture)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test to verify GenerateMipmap on GL_TEXTURE_VIDEO_FRAME_WEBGL is NOT allowed.
TEST_P(WEBGLVideoFrameTest, MustNotGenerateMipmap)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    glGenerateMipmap(GL_TEXTURE_VIDEO_FRAME_WEBGL);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test to verify binding an existing 2D texture is allowed.
TEST_P(WEBGLVideoFrameTest, BindExisting2DTexture)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    ASSERT_GL_NO_ERROR();
}

// Test to verify binding a texture other than 2D, External, Rectangle to
// GL_TEXTURE_VIDEO_FRAME_WEBGL is NOT allowed.
TEST_P(WEBGLVideoFrameTest, MustNotBindExistingCubeMapTexture)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test to verify samplerVideoFrameWEBGL works fine when extension is enabled.
TEST_P(WEBGLVideoFrameTest, VerifySamplerVideoFrameWEBGL)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    ANGLE_SKIP_TEST_IF(!IsD3D11() && !IsD3D9());

    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(

#extension GL_ANGLE_webgl_video_frame : require
precision mediump float;
varying mediump vec2 texCoord;
uniform samplerVideoFrameWEBGL s;
void main()
{
    gl_FragColor = textureVideoFrameWEBGL(s, texCoord);
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

    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to verify samplerVideoFrameWEBGL works fine in ES300 when extension is enabled.
TEST_P(WEBGLVideoFrameES300Test, VerifySamplerVideoFrameWEBGLInES300)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_webgl_video_frame"));

    ANGLE_SKIP_TEST_IF(!IsD3D11() && !IsD3D9());

    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
out mediump vec2 texCoord;

void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_ANGLE_webgl_video_frame : require
precision mediump float;
in mediump vec2 texCoord;
uniform samplerVideoFrameWEBGL s;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(s, texCoord);
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

    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, texture);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_FRAME_WEBGL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WEBGLVideoFrameTest);
ANGLE_INSTANTIATE_TEST_ES3(WEBGLVideoFrameES300Test);
}  // namespace
