//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalInitArrayAssignmentConst_test.cpp:
//  Tests that uninitialized global arrays are initialized with array
//  assignment.

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;
using namespace testing;

class GlobalUninitializedArrayTest
    : public testing::TestWithParam<
          testing::tuple<ShShaderSpec, const char *, ShShaderOutput, bool>>
{
  public:
    GlobalUninitializedArrayTest() {}
    ShShaderSpec getShaderSpec() const { return testing::get<0>(GetParam()); }
    std::string getShader() const { return testing::get<1>(GetParam()); }
    ShShaderOutput getShaderOutput() const { return testing::get<2>(GetParam()); }
    bool expectAssignmentInitializer() const { return testing::get<3>(GetParam()); }
};

static constexpr char ESSL300_UninitializedVec4ArrayShader[] =
    R"(#version 300 es
        precision highp float;
        out vec4 color;
        vec4 data[256];
        void main()
        {
            int i = int(gl_FragCoord.x) % data.length();
            data[i] = vec4(0.0, sin(gl_FragCoord.x + 2.0), 0.0, 1.0 );
            color = data[i];
        })";
static constexpr char ESSL100_UninitializedVec4ArrayShader[] =
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

static constexpr char kExpectedAssignmentInitializer[] =
    "vec4 _udata[256] = vec4[256](vec4(0.0, 0.0, 0.0, 0.0), ";

static constexpr char kExpectedFunctionInitializer[] = "initGlobals";

// Test init with assignment in GLSL 300 ES source, GL SL variant targets.
TEST_P(GlobalUninitializedArrayTest, InitWithAssignmentOrFunction)
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
    if (expectAssignmentInitializer())
    {
        EXPECT_THAT(resultCode, testing::Not(testing::HasSubstr(kExpectedFunctionInitializer)));
        EXPECT_THAT(resultCode, testing::HasSubstr(kExpectedAssignmentInitializer));
    }
    else
    {
        EXPECT_THAT(resultCode, testing::HasSubstr(kExpectedFunctionInitializer));
        EXPECT_THAT(resultCode, testing::Not(testing::HasSubstr(kExpectedAssignmentInitializer)));
    }
}

constexpr bool kExpectArrayInitializer    = true;
constexpr bool kExpectFunctionInitializer = false;

INSTANTIATE_TEST_CASE_P(ESSL300_GlobalUninitializedArraysInitWithAssignment,
                        GlobalUninitializedArrayTest,
                        Combine(Values(SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
                                Values(ESSL300_UninitializedVec4ArrayShader),
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
                                Values(kExpectArrayInitializer)));

INSTANTIATE_TEST_CASE_P(
    ESSL100_GlobalUninitializedArraysInitWithAssignment,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES2_SPEC, SH_WEBGL_SPEC, SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(ESSL100_UninitializedVec4ArrayShader),
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
            Values(kExpectArrayInitializer)));

INSTANTIATE_TEST_CASE_P(
    ESSL100_GlobalUninitializedArraysInitWithFunction,
    GlobalUninitializedArrayTest,
    Combine(Values(SH_GLES2_SPEC, SH_WEBGL_SPEC, SH_GLES3_SPEC, SH_WEBGL2_SPEC, SH_GLES3_1_SPEC),
            Values(ESSL100_UninitializedVec4ArrayShader),
            Values(SH_ESSL_OUTPUT),
            Values(kExpectFunctionInitializer)));
