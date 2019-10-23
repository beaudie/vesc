//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

constexpr bool kEnableFeedbackLoop  = true;
constexpr bool kDisableFeedbackLoop = false;

class RenderingFeedbackLoopTest : public ANGLETest
{
  protected:
    RenderingFeedbackLoopTest()
        : ANGLETest(),
          mProgram(0),
          mFramebuffer(0),
          mInputTexture(0),
          mOutputTexture(0),
          mTextureUniformLocation(-1)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    const char *getVertexShaderSource()
    {
        return R"(
precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
}
)";
    }

    const char *getPositionAttributeName() { return "position"; }

    void setupTextures()
    {
        glGenTextures(2, mTextures);
        for (int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        std::vector<GLubyte> pixeldata;
        pixeldata.reserve(getWindowWidth() * getWindowHeight() * 4);
        for (int i = 0; i < getWindowWidth() * getWindowHeight(); i++)
        {
            pixeldata.push_back(0xFF);
            pixeldata.push_back(0x00);
            pixeldata.push_back(0x00);
            pixeldata.push_back(0xFF);
        }

        mInputTexture = mTextures[0];
        glBindTexture(GL_TEXTURE_2D, mInputTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA,
                        GL_UNSIGNED_BYTE, pixeldata.data());

        mOutputTexture = mTextures[1];

        glBindTexture(GL_TEXTURE_2D, 0);
        ASSERT_GL_NO_ERROR();
    }

    void testSetUp() override { setupTextures(); }

    void testTearDown() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &mFramebuffer);
        glDeleteTextures(2, mTextures);
        glDeleteProgram(mProgram);
    }

    void setUpProgram(const char *fragmentShaderSource, const char *textureUniformName)
    {
        const char *vertexShaderSource = getVertexShaderSource();

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        ASSERT_NE(0u, mProgram);

        mTextureUniformLocation = glGetUniformLocation(mProgram, textureUniformName);

        glUseProgram(mProgram);
        glUniform1i(mTextureUniformLocation, 0);
        ASSERT_GL_NO_ERROR();
    }

    void setupFramebuffer(bool createFeedbackLoop)
    {
        mOutputTexture = createFeedbackLoop ? mInputTexture : mTextures[1];

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glBindTexture(GL_TEXTURE_2D, mOutputTexture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOutputTexture,
                               0);
        ASSERT_GL_NO_ERROR();
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void performDraw()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mInputTexture);
        EXPECT_GL_NO_ERROR();

        drawQuad(mProgram, getPositionAttributeName(), 0.5f);
    }

    GLuint mProgram;
    GLuint mFramebuffer;

    GLuint mTextures[2];

    GLuint mInputTexture;
    GLuint mOutputTexture;

    GLint mTextureUniformLocation;
};

TEST_P(RenderingFeedbackLoopTest, BasicRenderNoLoop)
{
    const char *shader = R"(
precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
}
)";

    setUpProgram(shader, "tex");
    setupFramebuffer(kDisableFeedbackLoop);
    EXPECT_GL_NO_ERROR();

    performDraw();

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0xFF, 0x00, 0x00, 0xFF));
}

TEST_P(RenderingFeedbackLoopTest, SamplerUnreferencedFalseLoop)
{
    const char *shader = R"(
precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

    setUpProgram(shader, "tex");
    setupFramebuffer(kEnableFeedbackLoop);
    EXPECT_GL_NO_ERROR();

    performDraw();

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0x00, 0xFF, 0x00, 0xFF));
}

TEST_P(RenderingFeedbackLoopTest, SamplerReferencedDeadCode)
{
    const char *shader = R"(
precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

// should be removed by dead code elimination
vec4 uselessSample()
{
    return texture2D(tex, texcoord);
}

void main()
{
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

    setUpProgram(shader, "tex");
    setupFramebuffer(kEnableFeedbackLoop);
    EXPECT_GL_NO_ERROR();

    performDraw();
    ReadColor(0, 0);
}

TEST_P(RenderingFeedbackLoopTest, SamplerReferencedRealLoop)
{
    ANGLE_SKIP_TEST_IF(IsVulkan());  // anglebug.com/4075

    const char *shader = R"(
precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    vec4 usefulSample = texture2D(tex, texcoord);
    gl_FragColor = vec4(1.0 - usefulSample.r, 1.0 - usefulSample.g, 1.0 - usefulSample.b, 1.0);
}
)";

    setUpProgram(shader, "tex");
    setupFramebuffer(kEnableFeedbackLoop);
    EXPECT_GL_NO_ERROR();

    performDraw();
    ReadColor(0, 0);
}

TEST_P(RenderingFeedbackLoopTest, SamplerReferencedWritesDisabled)
{
    ANGLE_SKIP_TEST_IF(IsVulkan());  // anglebug.com/4076

    const char *shader = R"(
precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    vec4 usefulSample = texture2D(tex, texcoord);
    gl_FragColor = vec4(1.0 - usefulSample.r, 1.0 - usefulSample.g, 1.0 - usefulSample.b, 1.0);
}
)";

    setUpProgram(shader, "tex");
    setupFramebuffer(kEnableFeedbackLoop);
    glColorMask(false, false, false, false);
    EXPECT_GL_NO_ERROR();

    performDraw();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0xFF, 0x00, 0x00, 0xFF));
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(RenderingFeedbackLoopTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES2_VULKAN());

}  // anonymous namespace
