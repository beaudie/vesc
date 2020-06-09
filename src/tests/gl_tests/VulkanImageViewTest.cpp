//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <vector>

using namespace angle;

namespace
{

class ImageViewTest : public ANGLETest
{
  protected:
    ImageViewTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");
        ASSERT_NE(-1, mTextureUniformLocation);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ASSERT_GL_NO_ERROR();

        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(1, &mSourceTexture);
        glGenTextures(1, &mTargetTexture);
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        if (mTargetTexture != 0)
        {
            glDeleteTextures(1, &mTargetTexture);
        }
        if (mSourceTexture != 0)
        {
            glDeleteTextures(1, &mSourceTexture);
        }
        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
        }
    }

    template <typename T>
    void init2DSourceTexture(GLenum internalFormat,
                             GLenum dataFormat,
                             GLenum dataType,
                             const T *data)
    {
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void attachTargetTextureToFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTargetTexture,
                               0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderToTargetTexture()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);

        glUseProgram(mProgram);
        glUniform1i(mTextureUniformLocation, 0);

        drawQuad(mProgram, "position", 0.5f);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void renderToDefaultFramebuffer(GLColor *expectedData)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(mProgram);
        glBindTexture(GL_TEXTURE_2D, mTargetTexture);
        glUniform1i(mTextureUniformLocation, 0);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mProgram, "position", 0.5f);

        EXPECT_PIXEL_COLOR_EQ(0, 0, *expectedData);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    GLuint mProgram;
    GLint mTextureUniformLocation;

    GLuint mSourceTexture;
    GLuint mTargetTexture;

    GLuint mFramebuffer;
};

TEST_P(ImageViewTest, Swizzle)
{
    GLubyte data[] = {1, 64, 128, 200};
    GLColor expectedData(data[0], data[1], data[2], data[3]);

    // Create the source and target texture
    init2DSourceTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create a framebuffer and the target texture is attached to the framebuffer.
    attachTargetTextureToFramebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render content of source texture to target texture
    // This command triggers the creation of -
    //     - draw imageviews of the texture
    //     - VkFramebuffer object of the framebuffer
    renderToTargetTexture();

    // This swizzle operation should cause the read imageviews of the texture to be released
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Draw using the newly created read imageviews
    renderToDefaultFramebuffer(&expectedData);

    // Render content of source texture to target texture, again
    renderToTargetTexture();

    renderToDefaultFramebuffer(&expectedData);
}

ANGLE_INSTANTIATE_TEST_ES3(ImageViewTest);

}  // namespace
