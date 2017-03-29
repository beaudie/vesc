//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AtomicCounterBufferTest:
//   Various tests related for atomic counter buffers.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class AtomicCounterBufferTest : public ANGLETest
{
  protected:
    AtomicCounterBufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test GL_ATOMIC_COUNTER_BUFFER is not supported with version lower than ES31.
TEST_P(AtomicCounterBufferTest, AtomicCounterBufferBindings)
{
    ASSERT_EQ(3, getClientMajorVersion());
    GLBuffer atomicCounterBuffer;
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicCounterBuffer.get());
    if (getClientMinorVersion() < 1)
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
    else
    {
        EXPECT_GL_NO_ERROR();
    }
}

TEST_P(AtomicCounterBufferTest, BasicAtomicCounterDeclaration)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 12) uniform atomic_uint c;\n"
        "layout(binding = 1, offset = 4) uniform atomic_uint d;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

TEST_P(AtomicCounterBufferTest, InvalidShaderVersion)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 300 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_EQ(0u, shader);
}

TEST_P(AtomicCounterBufferTest, InvalidQualifier)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) in atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_EQ(0u, shader);
}

TEST_P(AtomicCounterBufferTest, BindingOffsetOverlapping)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a[3];\n"
        "layout(binding = 2, offset = 8) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_EQ(0u, shader);
}

TEST_P(AtomicCounterBufferTest, GlobalBindingOffsetOverlapping)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint c;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_EQ(0u, shader);
}

// The spec only demands offset unique and non-overlapping. So this should be allowed.
TEST_P(AtomicCounterBufferTest, DeclarationSequenceWithDecrementalOffsetsSpecified)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2, offset = 0) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, source);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Linking should fail if counters in vertex shader exceed gl_MaxVertexAtomicCounters.
TEST_P(AtomicCounterBufferTest, ExceedMaxVertexAtomicCounters)
{
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        return;
    }

    const std::string &vertexShaderSource =
        "#version 310 es\n"
        "layout(binding = 2) uniform atomic_uint foo[gl_MaxVertexAtomicCounters + 1];\n"
        "void main()\n"
        "{\n"
        "    atomicCounterIncrement(foo[0]);\n"
        "}\n";
    const std::string &fragmentShaderSource =
        "#version 310 es\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint program = CompileProgram(vertexShaderSource, fragmentShaderSource);
    EXPECT_EQ(0u, program);
}

ANGLE_INSTANTIATE_TEST(AtomicCounterBufferTest,
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES31_OPENGL(),
                       ES31_OPENGLES());

}  // namespace
