//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalUninitializedArray_test.cpp:
//  Tests that uninitialized global arrays are initialized with array
//  assignment in cases this can be done. This makes it easier for the
//  driver compiler to optimize based on the array contents.

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;
using namespace testing;

class GlobalUninitializedArrayTest
    : public testing::TestWithParam<
          testing::tuple<ShShaderSpec, const char *, ShShaderOutput, const char *, const char *>>
{
  public:
    GlobalUninitializedArrayTest() {}
    ShShaderSpec getShaderSpec() const { return testing::get<0>(GetParam()); }
    std::string getShader() const { return testing::get<1>(GetParam()); }
    ShShaderOutput getShaderOutput() const { return testing::get<2>(GetParam()); }
    std::string getExpectedAssignment() const { return testing::get<3>(GetParam()); }
    std::string getUnexpectedAssignment() const { return testing::get<4>(GetParam()); }
};

// Below uses 'lowp', as otherwise the generator for gles will need to add an
// immediate array initializer.
static constexpr char kESSL300UninitializedVec4ArrayShader[] =
    R"(#version 300 es
        precision highp float;
        out vec4 color;
        lowp vec4 data[256];
        void main()
        {
            int i = int(gl_FragCoord.x) % data.length();
            data[i] = vec4(0.0, sin(gl_FragCoord.x + 2.0), 0.0, 1.0 );
            color = data[i];
        })";
static constexpr char kESSL100UninitializedVec4ArrayShader[] =
    R"(
        precision mediump float;
        vec4 data[256];
        void main()
        {
            for (int i = 0; i < 256; ++i) {
              data[i] = vec4(0.0, sin(gl_FragCoord.x + 2.0), 0.0, 1.0 );
              gl_FragColor = data[i];
            }
        })";

static constexpr char kGLSLExpectedAssignmentInitializer[] =
    "vec4 _udata[256] = vec4[256](vec4(0.0, 0.0, 0.0, 0.0), ";

static constexpr char kGLSLExpectedFunctionInitializer[] = "initGlobals";

static constexpr char kHLSLExpectedInitializer[] = "static float4 _data[256];";

static constexpr char kHLSLUnexpectedInitializer[] = "static float4 _data[256] = ";

// Test that uninitialized global arrays in source shader get translated to
// shader code with expected initialization method. Mostly the test is used to
// test that the expectd initialization method is global array initialization as
// opposed to uninitialized array and an initialization routine.
TEST_P(GlobalUninitializedArrayTest, InitializationAsExpected)
{
    const GLenum kShaderType               = GL_FRAGMENT_SHADER;
    const ShCompileOptions kCompileOptions = SH_INITIALIZE_UNINITIALIZED_LOCALS;
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);
    std::stringstream header;
    header << "Shader spec: 0x" << std::hex << getShaderSpec() << " Output: 0x" << std::hex
           << getShaderOutput() << " Shader: " << getShader();
    SCOPED_TRACE(header.rdbuf());

    std::string resultCode;
    std::string resultInfoLog;
    bool result = compileTestShader(kShaderType, getShaderSpec(), getShaderOutput(), getShader(),
                                    &resources, kCompileOptions, &resultCode, &resultInfoLog);
    EXPECT_TRUE(result) << "Compile failed. Info log: '" << resultInfoLog << "'. ";
    EXPECT_THAT(resultCode, testing::HasSubstr(getExpectedAssignment()));
    EXPECT_THAT(resultCode, testing::Not(testing::HasSubstr(getUnexpectedAssignment())));
}

// Tests where destination shader should have global array initialization.
INSTANTIATE_TEST_CASE_P(ESSL300_GlobalUninitializedArrayTest,
                        GlobalUninitializedArrayTest,
                        Combine(Values(SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
                                Values(kESSL300UninitializedVec4ArrayShader),
                                Values(SH_ESSL_OUTPUT,
                                       SH_GLSL_COMPATIBILITY_OUTPUT,
                                       SH_GLSL_130_OUTPUT,
                                       SH_GLSL_140_OUTPUT,
                                       SH_GLSL_150_CORE_OUTPUT,
                                       SH_GLSL_330_CORE_OUTPUT,
                                       SH_GLSL_400_CORE_OUTPUT,
                                       SH_GLSL_410_CORE_OUTPUT,
                                       SH_GLSL_420_CORE_OUTPUT,
                                       SH_GLSL_430_CORE_OUTPUT,
                                       SH_GLSL_440_CORE_OUTPUT,
                                       SH_GLSL_450_CORE_OUTPUT),
                                Values(kGLSLExpectedAssignmentInitializer),
                                Values(kGLSLExpectedFunctionInitializer)));

// Tests where destination shader should have global array initialization.
INSTANTIATE_TEST_CASE_P(
    ESSL100_GlobalUninitializedArrayTest,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES2_SPEC, SH_WEBGL_SPEC, SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(kESSL100UninitializedVec4ArrayShader),
            Values(SH_GLSL_COMPATIBILITY_OUTPUT,
                   SH_GLSL_130_OUTPUT,
                   SH_GLSL_140_OUTPUT,
                   SH_GLSL_150_CORE_OUTPUT,
                   SH_GLSL_330_CORE_OUTPUT,
                   SH_GLSL_400_CORE_OUTPUT,
                   SH_GLSL_410_CORE_OUTPUT,
                   SH_GLSL_420_CORE_OUTPUT,
                   SH_GLSL_430_CORE_OUTPUT,
                   SH_GLSL_440_CORE_OUTPUT,
                   SH_GLSL_450_CORE_OUTPUT),
            Values(kGLSLExpectedAssignmentInitializer),
            Values(kGLSLExpectedFunctionInitializer)));

// Tests where destination shader should have an uninitialized global array and an
// initialization routine.
// This is the case where we cannot use array initializer since there is no notion of
// translating from GLSL 100 ES -> GLSL 300 ES. If ESSL output is selected, GLSL 100 ES
// shader is preserved.
INSTANTIATE_TEST_CASE_P(
    ESSL100_GlobalUninitializedArraysInitWithFunction,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES2_SPEC, SH_WEBGL_SPEC, SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(kESSL100UninitializedVec4ArrayShader),
            Values(SH_ESSL_OUTPUT),
            Values(kGLSLExpectedFunctionInitializer),
            Values(kGLSLExpectedAssignmentInitializer)));

// Tests where destination shader should have an static global shader that uses
// HLSL implicit array initialization rule.
INSTANTIATE_TEST_CASE_P(
    ESSL100_HLSL_GlobalUninitializedArrayTest,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES2_SPEC, SH_WEBGL_SPEC, SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(kESSL100UninitializedVec4ArrayShader),
            Values(SH_HLSL_3_0_OUTPUT, SH_HLSL_4_1_OUTPUT, SH_HLSL_4_0_FL9_3_OUTPUT),
            Values(kHLSLExpectedInitializer),
            Values(kHLSLUnexpectedInitializer)));

// Tests where destination shader should have an static global shader that uses
// HLSL implicit array initialization rule.
INSTANTIATE_TEST_CASE_P(
    ESSL300_HLSL_GlobalUninitializedArrayTest,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(kESSL300UninitializedVec4ArrayShader),
            Values(SH_HLSL_3_0_OUTPUT, SH_HLSL_4_1_OUTPUT, SH_HLSL_4_0_FL9_3_OUTPUT),
            Values(kHLSLExpectedInitializer),
            Values(kHLSLUnexpectedInitializer)));
