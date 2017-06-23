//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVariables_test.cpp:
//   Tests for buffer variables in GLSL ES 3.10 section 4.3.7.
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

// Test that the buffer qualifier described in GLSL ES 3.10 section 4.3.7 can be successfully
// compiled.
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

// Test that buffer qualifier is not supported in verson lower than GLSL ES 3.10.
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

// Test that can't assign to a readonly buffer variable.
TEST_F(BufferVariablesTest, AssignToROBufferVariable)
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

// Test that can't assign to a buffer variable declared within shader storage block with readonly.
TEST_F(BufferVariablesTest, AssignToBufferVariableWithinROBlock)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) readonly buffer buf {\n"
        "    int b1;\n"
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

// Test that a readonly and writeonly buffer variable should neither read or write.
TEST_F(BufferVariablesTest, AccessRWBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 = 5;\n"
        "    int test = b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that accessing a writeonly buffer variable should be error.
TEST_F(BufferVariablesTest, AccessWOBufferVariable)
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

// Test that accessing a buffer variable through an instance name is ok.
TEST_F(BufferVariablesTest, AccessROBufferVariableByInstanceName)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "} instanceBuffer;\n"
        "void main()\n"
        "{\n"
        "    float test = instanceBuffer.f;\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that accessing a buffer variable through an instance name inherits the writeonly qualifier
// and generates errors.
TEST_F(BufferVariablesTest, AccessWOBufferVariableByInstanceName)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) writeonly buffer buf {\n"
        "    float f;\n"
        "} instanceBuffer;\n"
        "void main()\n"
        "{\n"
        "    float test = instanceBuffer.f;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as the argument of an unary op should be error.
TEST_F(BufferVariablesTest, UnaryOpWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    ++b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable on the left-hand side of compound assignment should be error.
TEST_F(BufferVariablesTest, CompoundAssignmentToWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 += 5;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as ternary op argument should be error.
TEST_F(BufferVariablesTest, TernarySelectionWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly bool b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = b1 ? 1 : 0;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as constructor argument should be error.
TEST_F(BufferVariablesTest, ArrayConstructorWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    float a[3] = float[3](f, f, f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as built-in function argument should be error.
TEST_F(BufferVariablesTest, BuildInFunctionWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int a;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = min(a, 1);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that readonly buffer variable as user-defined function in argument should be ok.
TEST_F(BufferVariablesTest, UserDefinedFnInArgWithROBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "};\n"
        "void foo(float) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as user-defined function in argument should be error.
TEST_F(BufferVariablesTest, UserDefinedFnInArgWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void foo(float) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as user-defined function out argument should be ok.
TEST_F(BufferVariablesTest, UserDefinedFnOutArgWithWOBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void foo(out float) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that readonly buffer variable as user-defined function out argument should be error.
TEST_F(BufferVariablesTest, UserDefinedFnOutArgWithROBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "};\n"
        "void foo(out float) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that buffer qualifier can't modify a function parameter.
TEST_F(BufferVariablesTest, BufferQualifierOnFunctionParameter)
{
    const std::string &source =
        "#version 310 es\n"
        "void foo(buffer float) {}\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
