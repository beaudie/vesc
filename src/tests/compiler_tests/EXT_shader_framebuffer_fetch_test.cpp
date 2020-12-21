//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_shader_framebuffer_fetch_test.cpp:
//   Test for EXT_shader_framebuffer_fetch and EXT_shader_framebuffer_fetch_non_coherent
//

#include "tests/test_utils/ShaderExtensionTest.h"

namespace
{
const char EXTPragma[] = "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";

// Redeclare gl_LastFragData with noncoherent qualifier
const char ESSL100_LastFragDataRedeclared[] =
    R"(
    uniform highp vec4 u_color;
    layout(noncoherent) highp vec4 gl_LastFragData[gl_MaxDrawBuffers];

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0];
    })";

// Use gl_LastFragData without redeclaration of gl_LastFragData with noncoherent qualifier
const char ESSL100_LastFragDataWithoutRedeclaration[] =
    R"(
    uniform highp vec4 u_color;

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0];
    })";

// Redeclare gl_LastFragData without noncoherent qualifier
const char ESSL100_LastFragDataRedeclaredWithoutNoncoherent[] =
    R"(
    uniform highp vec4 u_color;
    highp vec4 gl_LastFragData[gl_MaxDrawBuffers];

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0];
    })";

// Use inout variable with noncoherent qualifier
const char ESSL300_InOut[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec4(0.0f), vec4(1.0f));
    })";

// Use inout variable with noncoherent qualifier and 3-components vector
const char ESSL300_InOut2[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec3 o_color;
    uniform highp vec3 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec3(0.0f), vec3(1.0f));
    })";

// Use inout variable with noncoherent qualifier and integer type qualifier
const char ESSL300_InOut3[] =
    R"(
    layout(noncoherent, location = 0) inout highp ivec4 o_color;
    uniform highp ivec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, ivec4(0), ivec4(1));
    })";

// Use inout variable with noncoherent qualifier and unsigned integer type qualifier
const char ESSL300_InOut4[] =
    R"(
    layout(noncoherent, location = 0) inout highp uvec4 o_color;
    uniform highp uvec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, uvec4(0), uvec4(1));
    })";

// Use inout variable with noncoherent qualifier and inout function parameter
const char ESSL300_InOut5[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void getClampValue(inout highp mat4 io_color, highp vec4 i_color)
    {
        io_color[0] = clamp(io_color[0] + i_color, vec4(0.0f), vec4(1.0f));
    }

    void main (void)
    {
        highp mat4 o_color_mat = mat4(0);
        o_color_mat[0] = o_color;
        getClampValue(o_color_mat, u_color);
        o_color = o_color_mat[0];
    })";

// Use inout variable without noncoherent qualifier
const char ESSL300_InOutWithoutNoncoherent[] =
    R"(
    layout(location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec4(0.0f), vec4(1.0f));
    })";

class EXTShaderFramebufferFetchNoncoherentTest : public sh::ShaderExtensionTest
{
  public:
    void InitializeCompiler() { InitializeCompiler(SH_GLSL_450_CORE_OUTPUT); }
    void InitializeCompiler(ShShaderOutput shaderOutputType)
    {
        DestroyCompiler();

        mCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, testing::get<0>(GetParam()),
                                          shaderOutputType, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(const char *pragma)
    {
        const char *shaderStrings[] = {testing::get<1>(GetParam()), pragma,
                                       testing::get<2>(GetParam())};
        bool success = sh::Compile(mCompiler, shaderStrings, 3, SH_VARIABLES | SH_OBJECT_CODE);
        if (success)
        {
            return ::testing::AssertionSuccess() << "Compilation success";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompiler);
    }
};

class EXTShaderFramebufferFetchNoncoherentES100Test
    : public EXTShaderFramebufferFetchNoncoherentTest
{};

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES100Test, CompileFailsWithoutExtension)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
#if defined(ANGLE_ENABLE_VULKAN)
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
#endif
}

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES100Test, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
#if defined(ANGLE_ENABLE_VULKAN)
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(""));
#endif
}

//
#if defined(ANGLE_ENABLE_VULKAN)
TEST_P(EXTShaderFramebufferFetchNoncoherentES100Test, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}
#endif

class EXTShaderFramebufferFetchNoncoherentES100FailureTest
    : public EXTShaderFramebufferFetchNoncoherentES100Test
{};

//
#if defined(ANGLE_ENABLE_VULKAN)
TEST_P(EXTShaderFramebufferFetchNoncoherentES100FailureTest, CompileFailsWithoutNoncoherent)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}
#endif

class EXTShaderFramebufferFetchNoncoherentES300Test
    : public EXTShaderFramebufferFetchNoncoherentTest
{};

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES300Test, CompileFailsWithoutExtension)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
#if defined(ANGLE_ENABLE_VULKAN)
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
#endif
}

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES300Test, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
#if defined(ANGLE_ENABLE_VULKAN)
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(""));
#endif
}

//
#if defined(ANGLE_ENABLE_VULKAN)
TEST_P(EXTShaderFramebufferFetchNoncoherentES300Test, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}
#endif

class EXTShaderFramebufferFetchNoncoherentES300FailureTest
    : public EXTShaderFramebufferFetchNoncoherentES300Test
{};

//
#if defined(ANGLE_ENABLE_VULKAN)
TEST_P(EXTShaderFramebufferFetchNoncoherentES300FailureTest, CompileFailsWithoutNoncoherent)
{
    mResources.EXT_shader_framebuffer_fetch_non_coherent = 1;
    InitializeCompiler(SH_GLSL_VULKAN_OUTPUT);
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}
#endif

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTShaderFramebufferFetchNoncoherentES100Test,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_LastFragDataRedeclared)));

INSTANTIATE_TEST_SUITE_P(IncorrectESSL100Shaders,
                         EXTShaderFramebufferFetchNoncoherentES100FailureTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_LastFragDataWithoutRedeclaration,
                                        ESSL100_LastFragDataRedeclaredWithoutNoncoherent)));

INSTANTIATE_TEST_SUITE_P(
    CorrectESSL300Shaders,
    EXTShaderFramebufferFetchNoncoherentES300Test,
    Combine(Values(SH_GLES3_SPEC),
            Values(sh::ESSLVersion300),
            Values(ESSL300_InOut, ESSL300_InOut2, ESSL300_InOut3, ESSL300_InOut4, ESSL300_InOut5)));

INSTANTIATE_TEST_SUITE_P(IncorrectESSL300Shaders,
                         EXTShaderFramebufferFetchNoncoherentES300FailureTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_InOutWithoutNoncoherent)));

}  // anonymous namespace
