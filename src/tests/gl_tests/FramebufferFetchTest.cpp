//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExternalBufferTest:
//   Tests the correctness of EXT_shader_framebuffer_fetch_non_coherent extension.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

namespace angle
{

class FramebufferFetchNonCoherentES31 : public ANGLETest
{
  protected:
    static constexpr GLuint kMaxColorBuffer = 4u;
    static constexpr GLuint kViewportWidth  = 16u;
    static constexpr GLuint kViewportHeight = 16u;

    FramebufferFetchNonCoherentES31()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void render(GLuint coordLoc)
    {
        const GLfloat coords[] = {
            -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f,
        };

        const GLushort indices[] = {
            0, 1, 2, 2, 3, 0,
        };

        glViewport(0, 0, kViewportWidth, kViewportHeight);

        GLBuffer coordinatesBuffer;
        GLBuffer elementsBuffer;

        glBindBuffer(GL_ARRAY_BUFFER, coordinatesBuffer);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(coords), coords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(coordLoc);
        glVertexAttribPointer(coordLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)sizeof(indices), &indices[0],
                     GL_STATIC_DRAW);

        glFramebufferFetchBarrierEXT();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        ASSERT_GL_NO_ERROR();
    }
};

// Testing EXT_shader_framebuffer_fetch_non_coherent with inout qualifier
TEST_P(FramebufferFetchNonCoherentES31, BasicInout)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] =
        "#version 310 es\n"
        "in highp vec4 a_position;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	gl_Position = a_position;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n"
        "layout(noncoherent, location = 0) inout highp vec4 o_color;\n"
        "\n"
        "uniform highp vec4 u_color;\n"
        "void main (void)\n"
        "{\n"
        "	o_color += u_color;\n"
        "}\n";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
    GLTexture colorBufferTex;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex, 0);

    ASSERT_GL_NO_ERROR();

    float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Testing EXT_shader_framebuffer_fetch_non_coherent with gl_LastFragData
TEST_P(FramebufferFetchNonCoherentES31, BasicLastFragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] =
        "#version 100\n"
        "attribute vec4 a_position;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	gl_Position = a_position;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 100\n"
        "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n"
        "layout(noncoherent) mediump vec4 gl_LastFragData[gl_MaxDrawBuffers];\n"
        "uniform highp vec4 u_color;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	gl_FragColor = u_color + gl_LastFragData[0];\n"
        "}\n";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
    GLTexture colorBufferTex;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenColor.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex, 0);

    ASSERT_GL_NO_ERROR();

    float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Testing EXT_shader_framebuffer_fetch_non_coherent with multiple render target
TEST_P(FramebufferFetchNonCoherentES31, MultipleRenderTarget)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] =
        "#version 310 es\n"
        "in highp vec4 a_position;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	gl_Position = a_position;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n"
        "layout(noncoherent, location = 0) inout highp vec4 o_color0;\n"
        "layout(noncoherent, location = 1) inout highp vec4 o_color1;\n"
        "layout(noncoherent, location = 2) inout highp vec4 o_color2;\n"
        "layout(noncoherent, location = 3) inout highp vec4 o_color3;\n"
        "uniform highp vec4 u_color;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	o_color0 += u_color;\n"
        "	o_color1 += u_color;\n"
        "	o_color2 += u_color;\n"
        "	o_color3 += u_color;\n"
        "}\n";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> color0(kViewportWidth * kViewportHeight, GLColor::black);
    std::vector<GLColor> color1(kViewportWidth * kViewportHeight, GLColor::green);
    std::vector<GLColor> color2(kViewportWidth * kViewportHeight, GLColor::blue);
    std::vector<GLColor> color3(kViewportWidth * kViewportHeight, GLColor::cyan);
    GLTexture colorBufferTex[kMaxColorBuffer];
    GLenum colorAttachments[kMaxColorBuffer] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color0.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color1.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color2.data());
    glBindTexture(GL_TEXTURE_2D, colorBufferTex[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, color3.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    for (unsigned int i = 0; i < kMaxColorBuffer; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachments[i], GL_TEXTURE_2D,
                               colorBufferTex[i], 0);
    }
    glDrawBuffers(kMaxColorBuffer, &colorAttachments[0]);

    ASSERT_GL_NO_ERROR();

    float color[4]      = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation);

    ASSERT_GL_NO_ERROR();

    glReadBuffer(colorAttachments[0]);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::red);
    glReadBuffer(colorAttachments[1]);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::yellow);
    glReadBuffer(colorAttachments[2]);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::magenta);
    glReadBuffer(colorAttachments[3]);
    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::white);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Testing EXT_shader_framebuffer_fetch_non_coherent with multiple draw
TEST_P(FramebufferFetchNonCoherentES31, MultipleDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    constexpr char kVS[] =
        "#version 310 es\n"
        "in highp vec4 a_position;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "	gl_Position = a_position;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n"
        "layout(noncoherent, location = 0) inout highp vec4 o_color;\n"
        "\n"
        "uniform highp vec4 u_color;\n"
        "void main (void)\n"
        "{\n"
        "	o_color += u_color;\n"
        "}\n";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    std::vector<GLColor> greenColor(kViewportWidth * kViewportHeight, GLColor::green);
    GLTexture colorBufferTex;
    glBindTexture(GL_TEXTURE_2D, colorBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kViewportWidth, kViewportHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenColor.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex, 0);

    ASSERT_GL_NO_ERROR();

    float color1[4]     = {1.0f, 0.0f, 0.0f, 1.0f};
    GLint colorLocation = glGetUniformLocation(program, "u_color");
    glUniform4fv(colorLocation, 1, color1);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    render(positionLocation);

    float color2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    glUniform4fv(colorLocation, 1, color2);

    render(positionLocation);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(kViewportWidth / 2, kViewportHeight / 2, GLColor::white);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ANGLE_INSTANTIATE_TEST_ES31(FramebufferFetchNonCoherentES31);
}  // namespace angle
