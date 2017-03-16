//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_YUV_target.cpp:
//   Test for EXT_YUV_target implementation.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

using testing::Combine;
using testing::Values;
using testing::make_tuple;

namespace
{
const char ESSLVersion300[] = "#version 300 es\n";
const char EXTYTPragma[]    = "#extension GL_EXT_YUV_target : require\n";

const char ESSL300_SimpleShader[] =
    "precision mediump float;\n"
    "uniform __samplerExternal2DY2YEXT uSampler;\n"
    "out vec4 fragColor;\n"
    "void main() { \n"
    "    fragColor = vec4(1.0);\n"
    "}\n";

// Shader that samples the texture and writes to FragColor.
const char ESSL300_FragColorShader[] =
    "precision mediump float;\n"
    "uniform __samplerExternal2DY2YEXT uSampler;\n"
    "layout (yuv) out vec4 fragColor;\n"
    "void main() { \n"
    "    fragColor = texture(uSampler, vec2(0.0));\n"
    "}\n";

class EXTYUVTargetTest : public testing::TestWithParam<testing::tuple<const char *, const char *>>
{
  protected:
    virtual void SetUp()
    {
        sh::InitBuiltInResources(&mResources);
        mResources.EXT_YUV_target = 1;

        mCompiler = NULL;
    }

    virtual void TearDown() { DestroyCompiler(); }
    void DestroyCompiler()
    {
        if (mCompiler)
        {
            sh::Destruct(mCompiler);
            mCompiler = NULL;
        }
    }

    void InitializeCompiler()
    {
        DestroyCompiler();
        mCompiler =
            sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_GLES3_SPEC, SH_ESSL_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != NULL) << "Compiler could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(const char *pragma)
    {
        return TestShaderCompile(testing::get<0>(GetParam()),  // Version.
                                 pragma,
                                 testing::get<1>(GetParam())  // Shader.
                                 );
    }

    testing::AssertionResult TestShaderCompile(const char *version,
                                               const char *pragma,
                                               const char *shader)
    {
        const char *shaderStrings[] = {version, pragma, shader};
        bool success                = sh::Compile(mCompiler, shaderStrings, 3, 0);
        if (success)
        {
            return ::testing::AssertionSuccess() << "Compilation success";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompiler);
    }

  protected:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTYUVTargetTest, CompileFailsWithoutExtension)
{
    mResources.EXT_YUV_target = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTYTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTYUVTargetTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTYUVTargetTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
}

INSTANTIATE_TEST_CASE_P(CorrectESSL300Shaders,
                        EXTYUVTargetTest,
                        Combine(Values(ESSLVersion300),
                                Values(ESSL300_SimpleShader, ESSL300_FragColorShader)));

class EXTYUVTargetCompileFailureTest : public EXTYUVTargetTest
{
};

TEST_P(EXTYUVTargetCompileFailureTest, CompileFails)
{
    // Expect compile failure due to shader error, with shader having correct pragma.
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTYTPragma));
}

}  // namespace
