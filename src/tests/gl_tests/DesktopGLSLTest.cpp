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
    const char kVS[] = R"(#version 150
in vec4 position;

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 150
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

ANGLE_INSTANTIATE_TEST_GL32_CORE(DesktopGLSLTest);

}  // namespace angle
