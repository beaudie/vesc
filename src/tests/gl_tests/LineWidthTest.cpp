//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimpleOperationTest:
//   Basic GL commands such as linking a program, initializing a buffer, etc.

#include "test_utils/ANGLETest.h"

#include <vector>
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace
{
constexpr char kBasicVertexShader[] =
    R"(attribute highp vec4 position;
void main()
{
    gl_Position = position;
})";

constexpr char kGreenFragmentShader[] =
    R"(void main()
{
    gl_FragColor = vec4(0, 1, 0, 1);
})";

class LineWidthTest : public ANGLETest<>
{
  protected:
    LineWidthTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Verify that glLineWidth works
// Reproduce Qualcomm dEQP-GLES2.functional.rasterization.primitives.lines_wide failure
TEST_P(LineWidthTest, Basic)
{
    GLfloat range[2] = {1};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    EXPECT_GL_NO_ERROR();

    ANGLE_SKIP_TEST_IF(range[1] < 127);

    ANGLE_GL_PROGRAM(program, kBasicVertexShader, kGreenFragmentShader);
    glUseProgram(program);

    // Draw a line that is not parallel to x axis or y axis
    std::vector<Vector4> vertices = {{-0.2f, 0.0f, 0.0f, 1.0f}, {0.2f, 0.1f, 0.0f, 1.0f}};

    const GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(127.5);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
    glDisableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // verify line width
    for (int x = 0; x < 256; ++x)
    {
        int lineWidth = 0;
        std::vector<GLColor> pixels(256);
        glReadPixels(x, 0, 1, 256, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
        EXPECT_GL_NO_ERROR();

        for (int y = 0; y < 256; ++y)
        {
            if (pixels[y] == GLColor::white)
            {
                if (lineWidth > 0)
                {
                    ASSERT(lineWidth == 128);
                }
            }
            else if (pixels[y] == GLColor::green)
            {
                ++lineWidth;
            }
            else
            {
                ASSERT(false);
            }
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2(LineWidthTest);
}  // namespace