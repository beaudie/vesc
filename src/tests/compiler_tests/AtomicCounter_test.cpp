//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AtomicCounter_test.cpp:
//   Tests for validating ESSL 3.10 section 4.4.6.
//

#include "gtest/gtest.h"

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class AtomicCounterTest : public ShaderCompileTreeTest
{
  public:
    AtomicCounterTest() {}
  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxAtomicCounterBindings = 8;
    }
};

TEST_F(AtomicCounterTest, BasicAtomicCounterDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 12) uniform atomic_uint c;\n"
        "layout(binding = 1, offset = 4) uniform atomic_uint d;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

TEST_F(AtomicCounterTest, InvalidShaderVersion)
{
    const std::string &source =
        "#version 300 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

TEST_F(AtomicCounterTest, InvalidQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) in atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

TEST_F(AtomicCounterTest, BindingOffsetOverlapping)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a[3];\n"
        "layout(binding = 2, offset = 8) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

TEST_F(AtomicCounterTest, GlobalBindingOffsetOverlapping)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint c;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The spec only demands offset unique and non-overlapping. So this should be allowed.
TEST_F(AtomicCounterTest, DeclarationSequenceWithDecrementalOffsetsSpecified)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2, offset = 0) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
