//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

namespace angle
{

class DesktopGLSLTest : public ANGLETest
{
  protected:
    DesktopGLSLTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Simple test case of compiling a desktop OpenGL shader to verify that the shader compiler
// initializes.
TEST_P(DesktopGLSLTest, BasicCompilation)
{
    const char kVS[] = R"(#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

    const char kFS[] = R"(#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

ANGLE_INSTANTIATE_TEST_GL33_CORE(DesktopGLSLTest);

}  // namespace angle
