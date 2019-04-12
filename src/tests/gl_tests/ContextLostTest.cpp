//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BindGeneratesResourceTest.cpp : Tests of the GL_CHROMIUM_bind_generates_resource extension.

#include "test_utils/ANGLETest.h"

namespace angle
{
class ContextLostTest : public ANGLETest
{
  protected:
    ContextLostTest() {}
};

// GL_CHROMIUM_lose_context is implemented in the frontend
TEST_P(ContextLostTest, ExtensionStringExposed)
{
    EXPECT_TRUE(ensureExtensionEnabled("GL_CHROMIUM_lose_context"));
}

// Use GL_CHROMIUM_lose_context to lose a context and verify
TEST_P(ContextLostTest, BasicUsage)
{
    ANGLE_SKIP_TEST_IF(!ensureExtensionEnabled("GL_CHROMIUM_lose_context"));

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GL_ERROR(GL_OUT_OF_MEMORY);
}

// When context is lost, polling queries such as glGetSynciv with GL_SYNC_STATUS should always
// return GL_SIGNALED
TEST_P(ContextLostTest, PollingQuery)
{
    ANGLE_SKIP_TEST_IF(!ensureExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();

    GLint syncStatus = 0;
    glGetSynciv(sync, GL_SYNC_STATUS, 1, nullptr, &syncStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(syncStatus, GL_SIGNALED);
}

// When context is lost, polling queries such as glGetSynciv with GL_SYNC_STATUS should always
// return GL_SIGNALED
TEST_P(ContextLostTest, ParallelCompileReadyQuery)
{
    ANGLE_SKIP_TEST_IF(!ensureExtensionEnabled("GL_CHROMIUM_lose_context"));
    ANGLE_SKIP_TEST_IF(!ensureExtensionEnabled("GL_KHR_parallel_shader_compile"));

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::UniformColor());

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    EXPECT_GL_NO_ERROR();

    glLoseContextCHROMIUM(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
    EXPECT_GL_NO_ERROR();

    GLint shaderCompletionStatus = 0;
    glGetShaderiv(vs, GL_COMPLETION_STATUS_KHR, &shaderCompletionStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(shaderCompletionStatus, GL_TRUE);

    GLint programCompletionStatus = 0;
    glGetProgramiv(program, GL_COMPLETION_STATUS_KHR, &programCompletionStatus);
    EXPECT_GL_ERROR(GL_CONTEXT_LOST);
    EXPECT_GLENUM_EQ(programCompletionStatus, GL_TRUE);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ContextLostTest,
                       ES2_NULL(),
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());

}  // namespace angle
