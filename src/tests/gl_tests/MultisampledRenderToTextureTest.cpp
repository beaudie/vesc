//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultisampledRenderToTextureTest: Tests of EXT_multisampled_render_to_texture extension

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{
constexpr char kBasicVertexShader[] =
    R"(attribute vec3 position;
void main()
{
    gl_Position = vec4(position, 1);
})";

constexpr char kGreenFragmentShader[] =
    R"(void main()
{
    gl_FragColor = vec4(0, 1, 0, 1);
})";

constexpr char kRedFragmentShader[] =
    R"(void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
})";

class MultisampledRenderToTextureTest : public ANGLETest
{
  protected:
    MultisampledRenderToTextureTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenRenderbuffers(1, &mRenderbuffer);
        glGenTextures(2, mTextures);
        glGenFramebuffers(1, &mFBO);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteRenderbuffers(1, &mRenderbuffer);
        glDeleteTextures(2, mTextures);
        glDeleteFramebuffers(1, &mFBO);
        mRenderbuffer = 0;
    }

    GLuint mRenderbuffer = 0;
    GLuint mTextures[2]  = {};
    GLuint mFBO          = 0;
    GLuint mProgram      = 0;
};

TEST_P(MultisampledRenderToTextureTest, RenderbufferParameterCheck)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);

    // Positive test case
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_RGBA8, 64, 64);
    ASSERT_GL_NO_ERROR();

    GLint samples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
    ASSERT_GL_NO_ERROR();
    EXPECT_GE(samples, 1);

    // Samples too large
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples + 1, GL_RGBA8, 64, 64);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // Renderbuffer size too large
    GLint maxSize;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxSize);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 2, GL_RGBA8, maxSize + 1, maxSize);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 2, GL_RGBA8, maxSize, maxSize + 1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // Retrieving samples
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_RGBA8, 64, 64);
    GLint param = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES_EXT, &param);
    // GE becuase samples may vary base on implementation. Spec says "the resulting value for
    // RENDERBUFFER_SAMPLES_EXT is guaranteed to be greater than or equal to samples and no more
    // than the next larger sample count supported by the implementation"
    EXPECT_GE(param, 4);
}

TEST_P(MultisampledRenderToTextureTest, Texture2DParameterCheck)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    // Positive test case
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    ASSERT_GL_NO_ERROR();

    // Attachment not COLOR_ATTACHMENT0
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    // Target not framebuffer
    glFramebufferTexture2DMultisampleEXT(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    GLint samples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
    ASSERT_GL_NO_ERROR();
    EXPECT_GE(samples, 1);

    // Samples too large
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, samples + 1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // Retrieving samples
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    GLint param = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT, &param);
    // GE becuase samples may vary base on implementation. Spec says "the resulting value for
    // TEXTURE_SAMPLES_EXT is guaranteed to be greater than or equal to samples and no more than the
    // next larger sample count supported by the implementation"
    EXPECT_GE(param, 4);
}

TEST_P(MultisampledRenderToTextureTest, TextureCubeMapParameterCheck)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[1]);
    for (GLenum face = 0; face < 6; face++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        ASSERT_GL_NO_ERROR();
    }

    GLint samples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
    ASSERT_GL_NO_ERROR();
    EXPECT_GE(samples, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    for (GLenum face = 0; face < 6; face++)
    {
        // Positive test case
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTextures[1], 0,
                                             4);
        ASSERT_GL_NO_ERROR();

        // Attachment not COLOR_ATTACHMENT0
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTextures[1], 0,
                                             4);
        ASSERT_GL_ERROR(GL_INVALID_ENUM);

        // Target not framebuffer
        glFramebufferTexture2DMultisampleEXT(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTextures[1], 0,
                                             4);
        ASSERT_GL_ERROR(GL_INVALID_ENUM);

        // Samples too large
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTextures[1], 0,
                                             samples + 1);
        ASSERT_GL_ERROR(GL_INVALID_VALUE);

        // Retrieving samples
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTextures[1], 0,
                                             4);
        GLint param = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT,
                                              &param);
        // GE becuase samples may vary base on implementation. Spec says "the resulting value for
        // TEXTURE_SAMPLES_EXT is guaranteed to be greater than or equal to samples and no more than
        // the next larger sample count supported by the implementation"
        EXPECT_GE(param, 4);
    }
}

TEST_P(MultisampledRenderToTextureTest, FramebufferCompleteness)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    // Checking that Renderbuffer and texture2d having different number of samples results
    // in a FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT16, 64, 64);
    ASSERT_GL_NO_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

TEST_P(MultisampledRenderToTextureTest, 2DColorAttachmentMultisampleTest)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));
    GLsizei size = 6;
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, size, size);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(program, kBasicVertexShader, kGreenFragmentShader);
    glUseProgram(program);
    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    setupQuadVertexBuffer(0.5f, 0.5f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Tests that TRIANGLES works.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(size / 2, size / 2, GLColor::green);

    ANGLE_GL_PROGRAM(program2, kBasicVertexShader, kRedFragmentShader);
    glUseProgram(program2);
    GLint positionLocation2 = glGetAttribLocation(program2, "position");
    ASSERT_NE(-1, positionLocation2);

    setupQuadVertexBuffer(0.5f, 0.75f);
    glVertexAttribPointer(positionLocation2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(size / 2, size / 2, GLColor::red);
}

TEST_P(MultisampledRenderToTextureTest, 2DColorDepthMultisampleTest)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));
    GLsizei size = 6;
    // create complete framebuffer with depth buffer
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mTextures[0], 0, 4);

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, size, size);
    ASSERT_GL_NO_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderbuffer);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Set viewport and clear framebuffer
    glViewport(0, 0, size, size);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepthf(0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw first green square
    ANGLE_GL_PROGRAM(program, kBasicVertexShader, kGreenFragmentShader);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glUseProgram(program);
    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    setupQuadVertexBuffer(0.8f, 0.5f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Tests that TRIANGLES works.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(size / 2, size / 2, GLColor::green);

    // Draw red square behind green square
    ANGLE_GL_PROGRAM(program2, kBasicVertexShader, kRedFragmentShader);
    glUseProgram(program2);
    GLint positionLocation2 = glGetAttribLocation(program2, "position");
    ASSERT_NE(-1, positionLocation2);

    setupQuadVertexBuffer(0.7f, 0.1f);
    glVertexAttribPointer(positionLocation2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    glDisable(GL_DEPTH_TEST);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(size / 2, size / 2, GLColor::green);
}
ANGLE_INSTANTIATE_TEST(MultisampledRenderToTextureTest, ES2_D3D11(), ES3_D3D11());
}  // namespace
