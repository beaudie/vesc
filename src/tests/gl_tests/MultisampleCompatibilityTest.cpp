//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StateChangeTest:
//   Specifically designed for an ANGLE implementation of GL, these tests validate that
//   ANGLE's dirty bits systems don't get confused by certain sequences of state changes.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

const GLint kWidth = 64;
const GLint kHeight = 64;

// test drawing with GL_MULTISAMPLE_EXT enabled/disabled.
class EXTMultisampleCompatibilityTest : public ANGLETest
{

protected:
    EXTMultisampleCompatibilityTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void PrepareForDraw()
    {
        static const char* v_shader_str =
            "attribute vec4 a_Position;\n"
            "void main()\n"
            "{ gl_Position = a_Position; }";

        static const char* f_shader_str =
            "precision mediump float;\n"
            "uniform vec4 color;"
            "void main() { gl_FragColor = color; }";

        GLuint program = glCreateProgram();
        glAttachShader(program, compileShader(GL_VERTEX_SHADER, v_shader_str));
        glAttachShader(program, compileShader(GL_FRAGMENT_SHADER, f_shader_str));
        glLinkProgram(program);

        glUseProgram(program);
        GLuint position_loc = glGetAttribLocation(program, "a_Position");
        color_loc_ = glGetUniformLocation(program, "color");

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        static float vertices[] = {
            1.0f,  1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(position_loc);
        glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Create a sample buffer.
        GLsizei num_samples = 4, max_samples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
        num_samples = std::min(num_samples, max_samples);
        GLuint sample_rb;
        glGenRenderbuffers(1, &sample_rb);
        glBindRenderbuffer(GL_RENDERBUFFER, sample_rb);
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, num_samples,
                                             GL_RGBA8_OES, kWidth, kHeight);
        GLint param = 0;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES,
                                 &param);
        EXPECT_GE(param, num_samples);

        glGenFramebuffers(1, &sample_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, sample_fbo_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, sample_rb);
        EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
          glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create another FBO to resolve the multisample buffer into.
        GLuint resolve_tex;
        glGenTextures(1, &resolve_tex);
        glBindTexture(GL_TEXTURE_2D, resolve_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA,
           GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenFramebuffers(1, &resolve_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, resolve_fbo_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           resolve_tex, 0);
        EXPECT_EQ(static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE),
          glCheckFramebufferStatus(GL_FRAMEBUFFER));

        glViewport(0, 0, kWidth, kHeight);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, sample_fbo_);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void PrepareForVerify()
    {
        // Resolve.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, sample_fbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo_);
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlitFramebufferANGLE(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo_);
    }

    bool IsApplicable() const
    {
        return extensionEnabled("GL_EXT_multisample_compatibility") &&
             extensionEnabled("GL_ANGLE_framebuffer_multisample") &&
             extensionEnabled("GL_OES_rgb8_rgba8") &&
             !IsAMD();
    }
    GLuint sample_fbo_;
    GLuint resolve_fbo_;
    GLuint color_loc_;

};

} //


TEST_P(EXTMultisampleCompatibilityTest, TestSimple) {
    if (!IsApplicable())
        return;

    EXPECT_TRUE(glIsEnabled(GL_MULTISAMPLE_EXT));
    glDisable(GL_MULTISAMPLE_EXT);
    EXPECT_FALSE(glIsEnabled(GL_MULTISAMPLE_EXT));
    glEnable(GL_MULTISAMPLE_EXT);
    EXPECT_TRUE(glIsEnabled(GL_MULTISAMPLE_EXT));

    EXPECT_FALSE(glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE_EXT));
    glEnable(GL_SAMPLE_ALPHA_TO_ONE_EXT);
    EXPECT_TRUE(glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE_EXT));
    glDisable(GL_SAMPLE_ALPHA_TO_ONE_EXT);
    EXPECT_FALSE(glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE_EXT));

    EXPECT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError());
}

TEST_P(EXTMultisampleCompatibilityTest, DrawAndResolve)
{
    // Test that disabling GL_MULTISAMPLE_EXT is handled correctly.
    if (!IsApplicable())
        return;

    static const float kBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
    static const float kGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
    static const float kRed[] = {1.0f, 0.0f, 0.0f, 1.0f};

    // Different drivers seem to behave differently with respect to resulting
    // values. These might be due to different MSAA sample counts causing
    // different samples to hit.  Other option is driver bugs. Just test that
    // disabling multisample causes a difference.
    std::unique_ptr<uint8_t[]> results[3];
    const GLint kResultSize = kWidth * kHeight * 4;
    for (int pass = 0; pass < 3; pass++)
    {
        PrepareForDraw();
        // Green: from top right to bottom left.
        glUniform4fv(color_loc_, 1, kGreen);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Blue: from top left to bottom right.
        glUniform4fv(color_loc_, 1, kBlue);
        glDrawArrays(GL_TRIANGLES, 3, 3);

        // Red, with and without MSAA: from bottom left to top right.
        if (pass == 1)
        {
            glDisable(GL_MULTISAMPLE_EXT);
        }
        glUniform4fv(color_loc_, 1, kRed);
        glDrawArrays(GL_TRIANGLES, 6, 3);
        if (pass == 1)
        {
            glEnable(GL_MULTISAMPLE_EXT);
        }
        PrepareForVerify();
        results[pass].reset(new uint8_t[kResultSize]);
        memset(results[pass].get(), 123u, kResultSize);
        glReadPixels(0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                   results[pass].get());
    }
    EXPECT_NE(0, memcmp(results[0].get(), results[1].get(), kResultSize));
    // Verify that rendering is deterministic, so that the pass above does not
    // come from non-deterministic rendering.
    EXPECT_EQ(0, memcmp(results[0].get(), results[2].get(), kResultSize));
}

TEST_P(EXTMultisampleCompatibilityTest, DrawAlphaOneAndResolve)
{
    // Test that enabling GL_SAMPLE_ALPHA_TO_ONE_EXT affects rendering.
    if (!IsApplicable())
        return;

    // SAMPLE_ALPHA_TO_ONE is specified to transform alpha values of
    // covered samples to 1.0. In order to detect it, we use non-1.0
    // alpha.
    static const float kBlue[] = {0.0f, 0.0f, 1.0f, 0.5f};
    static const float kGreen[] = {0.0f, 1.0f, 0.0f, 0.5f};
    static const float kRed[] = {1.0f, 0.0f, 0.0f, 0.5f};

    // Different drivers seem to behave differently with respect to resulting
    // alpha value. These might be due to different MSAA sample counts causing
    // different samples to hit.  Other option is driver bugs. Testing exact or
    // even approximate sample values is not that easy.  Thus, just test
    // representative positions which have fractional pixels, inspecting that
    // normal rendering is different to SAMPLE_ALPHA_TO_ONE rendering.
    std::unique_ptr<uint8_t[]> results[3];
    const GLint kResultSize = kWidth * kHeight * 4;

    for (int pass = 0; pass < 3; ++pass) {
        PrepareForDraw();
        if (pass == 1)
        {
          glEnable(GL_SAMPLE_ALPHA_TO_ONE_EXT);
        }
        glEnable(GL_MULTISAMPLE_EXT);
        glUniform4fv(color_loc_, 1, kGreen);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniform4fv(color_loc_, 1, kBlue);
        glDrawArrays(GL_TRIANGLES, 3, 3);

        glDisable(GL_MULTISAMPLE_EXT);
        glUniform4fv(color_loc_, 1, kRed);
        glDrawArrays(GL_TRIANGLES, 6, 3);

        PrepareForVerify();
        results[pass].reset(new uint8_t[kResultSize]);
        memset(results[pass].get(), 123u, kResultSize);
        glReadPixels(0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
           results[pass].get());
        if (pass == 1)
        {
            glDisable(GL_SAMPLE_ALPHA_TO_ONE_EXT);
        }
    }
    EXPECT_NE(0, memcmp(results[0].get(), results[1].get(), kResultSize));
    // Verify that rendering is deterministic, so that the pass above does not
    // come from non-deterministic rendering.
    EXPECT_EQ(0, memcmp(results[0].get(), results[2].get(), kResultSize));
}


ANGLE_INSTANTIATE_TEST(EXTMultisampleCompatibilityTest, ES2_OPENGL(), ES2_OPENGLES(), ES3_OPENGL());