//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVariables_test.cpp:
//   Tests for buffer variables in ESSL 3.10 section 4.3.7.
//

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class BufferVariablesTest : public ShaderCompileTreeTest
{
  public:
    BufferVariablesTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxShaderStorageBufferBindings = 8;
    }
};

// Test that The buffer qualifier described in ESSL 3.10 section 4.3.7 can be successfully compiled.
TEST_F(BufferVariablesTest, BasicShaderStorageBlockDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that shader storage block layout qualifiers can be declared for global scope.
TEST_F(BufferVariablesTest, LayoutQualifiersDeclaredInGlobal)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(shared, column_major) buffer;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that shader storage block can be used with one or more memory qualifiers.
TEST_F(BufferVariablesTest, ShaderStorageBlockWithMemoryQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) writeonly buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that buffer variables can be used with one or more memory qualifiers.
TEST_F(BufferVariablesTest, BufferVariablesWithMemoryQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "    writeonly buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is a compile-time error to declare buffer variables at global scope (outside a
// block).
TEST_F(BufferVariablesTest, DeclareBufferVariableAtGlobal)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer int a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the buffer variable can't be opaque type.
TEST_F(BufferVariablesTest, BufferVariableWithOpaqueType)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    atomic_uint b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the uniform variable can't be in shader storage block.
TEST_F(BufferVariablesTest, UniformVariableInShaderStorageBlock)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    uniform int a;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that buffer qualifier is not supported in ESSL verson lower than 3.10
TEST_F(BufferVariablesTest, BufferQualifierInESSL3)
{
    const std::string &source =
        "#version 300 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a buffer variable qualified with readonly.
TEST_F(BufferVariablesTest, AssignToBufferVariableWithReadonly)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 = 5;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't read from a buffer variable qualified with writeonly.
TEST_F(BufferVariablesTest, ReadFromBufferVariableWithWriteonly)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
