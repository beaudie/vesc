//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultiviewSideBySideTest:
//   Tests for side-by-side multiview drawing.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class MultiviewSideBySideTest : public ANGLETest
{
  protected:
    MultiviewSideBySideTest() : mProgram(0u)
    {
        setWindowWidth(128);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
    }

    ~MultiviewSideBySideTest()
    {
        if (mVertexBuffer != 0)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mProgram != 0u)
        {
            glDeleteProgram(mProgram);
        }
    }

    GLuint mVertexBuffer;
    GLuint mProgram;
};

// Test that the rendering output when using drawArrays with multiview draw buffers is correct.
TEST_P(MultiviewSideBySideTest, MultiviewSideBySideDrawArrays)
{
    // Init program
    const std::string vs =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "in vec2 vPosition;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vPosition, 0.0, 1.0);\n"
        "}\n";
    const std::string fs =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(gl_ViewID_OVR, 1.0 - float(gl_ViewID_OVR), 0.0, 1.0);\n"
        "}\n";
    mProgram = CompileProgram(vs, fs);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "vPosition");
    ASSERT_NE(-1, positionLocation);

    glGenBuffers(1, &mVertexBuffer);

    std::vector<GLfloat> positionData;
    // quad verts
    // A---C
    // |  /|
    // | / |
    // |/  |
    // B---.
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(1.0f);
    // .---F
    // |  /|
    // | / |
    // |/  |
    // D---E
    positionData.push_back(-1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(-1.0f);
    positionData.push_back(1.0f);
    positionData.push_back(1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    // Activate left and right halves for drawing
    glDrawBufferSideBySideANGLE(GL_BACK);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    GLsizei pixelCount = getWindowWidth() * getWindowHeight() / 2;

    std::vector<GLColor> actualLeft(pixelCount, GLColor::black);
    glReadPixels(0, 0, getWindowWidth() / 2, getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualLeft.data());
    std::vector<GLColor> expectedLeft(pixelCount, GLColor::green);
    EXPECT_EQ(expectedLeft, actualLeft);

    std::vector<GLColor> actualRight(pixelCount, GLColor::black);
    glReadPixels(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight(), GL_RGBA,
                 GL_UNSIGNED_BYTE, actualRight.data());
    std::vector<GLColor> expectedRight(pixelCount, GLColor::red);
    EXPECT_EQ(expectedRight, actualRight);
}

ANGLE_INSTANTIATE_TEST(MultiviewSideBySideTest, ES3_OPENGL());
}
