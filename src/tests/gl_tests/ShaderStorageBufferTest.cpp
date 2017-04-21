//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBufferTest:
//   Various tests related for shader storage buffers.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ShaderStorageBufferTest : public ANGLETest
{
  protected:
    ShaderStorageBufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

TEST_P(ShaderStorageBufferTest, BasicShaderStorageBlockDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 0) buffer buf {\n"
        "    int mode;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_NE(0u, shader);
}

ANGLE_INSTANTIATE_TEST(ShaderStorageBufferTest, ES31_OPENGL(), ES31_OPENGLES());

}  // namespace
