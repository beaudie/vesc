//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RobustBufferAccessBehaviorTest:
//   Various tests related for GL_KHR_robust_buffer_access_behavior.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class RobustBufferAccessBehaviorTest : public ANGLETest
{
  protected:
    RobustBufferAccessBehaviorTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setRobustAccess(true);
    }

    void runIndexOutOfRangeTests(GLenum drawType)
    {
        const std::string &vsCheckOutOfBounds =
            "precision mediump float;\n"
            "attribute vec2 position;\n"
            "attribute vec4 vecRandom;\n"
            "varying vec4 v_color;\n"
            "bool testFloatComponent(float component) {\n"
            "    return (component == 0.2 || component == 0.0);\n"
            "}\n"
            "bool testLastFloatComponent(float component) {\n"
            "    return testFloatComponent(component) || component == 1.0;\n"
            "}\n"
            "void main() {\n"
            "    if (testFloatComponent(vecRandom.x) &&\n"
            "        testFloatComponent(vecRandom.y) &&\n"
            "        testFloatComponent(vecRandom.z) &&\n"
            "        testLastFloatComponent(vecRandom.w)) {\n"
            "        v_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "    } else {\n"
            "        v_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "    }\n"
            "    gl_Position = vec4(position, 0.0, 1.0);\n"
            "}\n";

        const std::string &fragmentShaderSource =
            "precision mediump float;\n"
            "varying vec4 v_color;\n"
            "void main() {\n"
            "    gl_FragColor = v_color;\n"
            "}\n";

        ANGLE_GL_PROGRAM(prog, vsCheckOutOfBounds, fragmentShaderSource);
        glUseProgram(prog.get());

        int posLoc       = glGetAttribLocation(prog.get(), "position");
        int vecRandomLoc = glGetAttribLocation(prog.get(), "vecRandom");

        GLBuffer bufferPos;
        glBindBuffer(GL_ARRAY_BUFFER, bufferPos.get());
        std::array<GLfloat, 8> posData = {{1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f}};
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posData.size(), posData.data(), drawType);

        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        GLBuffer bufferIncomplete;
        glBindBuffer(GL_ARRAY_BUFFER, bufferIncomplete.get());
        std::array<GLfloat, 12> randomData = {
            {0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f}};
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * randomData.size(), randomData.data(),
                     drawType);

        glEnableVertexAttribArray(vecRandomLoc);
        glVertexAttribPointer(vecRandomLoc, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        std::array<GLushort, 6> indices = {{0, 1, 2, 0, 2, 3}};
        GLBuffer indexBuffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.get());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(),
                     drawType);

        glClearColor(0.0, 0.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        std::vector<GLColor> actual(1, GLColor::white);
        GLenum result = glGetError();
        // For D3D dynamic draw, we still return invalid operation. Once we force the index buffer
        // to clamp any out of range indices instead of invalid operation, this part can be removed.
        // We can always get GL_NO_ERROR.
        if (result == GL_INVALID_OPERATION)
        {
            std::vector<GLColor> expected(1, GLColor::blue);
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, actual.data());
            EXPECT_EQ(expected, actual);
        }
        else
        {
            EXPECT_GLENUM_EQ(GL_NO_ERROR, result);
            std::vector<GLColor> expected(1, GLColor::green);
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, actual.data());
            EXPECT_EQ(expected, actual);
        }
    }
};

// Test that Out-of-bounds reads will not read outside of the data store of the buffer object and
// will not result in GL interruption or termination when GL_KHR_robust_buffer_access_behavior is
// supported.
TEST_P(RobustBufferAccessBehaviorTest, DrawElementsIndexOutOfRange)
{
    if (!extensionEnabled("GL_KHR_robust_buffer_access_behavior"))
    {
        std::cout << "Test skipped due to not supporting GL_KHR_robust_buffer_access_behavior"
                  << std::endl;
        return;
    }

    std::array<GLenum, 2> drawTypes = {{GL_STATIC_DRAW, GL_DYNAMIC_DRAW}};
    for (auto drawType : drawTypes)
    {
        runIndexOutOfRangeTests(drawType);
    }
}

ANGLE_INSTANTIATE_TEST(RobustBufferAccessBehaviorTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES31_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES31_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES31_OPENGLES());

}  // namespace
