//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>
#include <vector>

#include "test_utils/ANGLETest.h"
#include "util/shader_utils.h"

using namespace angle;

class EGLRaceConditionTest : public ANGLETest<>
{
  protected:
    EGLConfig chooseConfig(EGLDisplay display)
    {
        const EGLint attribs[] = {EGL_RED_SIZE,
                                  8,
                                  EGL_GREEN_SIZE,
                                  8,
                                  EGL_BLUE_SIZE,
                                  8,
                                  EGL_ALPHA_SIZE,
                                  8,
                                  EGL_RENDERABLE_TYPE,
                                  EGL_OPENGL_ES2_BIT,
                                  EGL_SURFACE_TYPE,
                                  EGL_PBUFFER_BIT,
                                  EGL_NONE};
        EGLConfig config       = EGL_NO_CONFIG_KHR;
        EGLint count           = 0;
        EXPECT_EGL_TRUE(eglChooseConfig(display, attribs, &config, 1, &count));
        EXPECT_EGL_TRUE(count > 0);
        return config;
    }

    EGLContext createContext(EGLDisplay display, EGLConfig config)
    {
        const EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE};
        EGLContext context     = eglCreateContext(display, config, nullptr, attribs);
        EXPECT_NE(context, EGL_NO_CONTEXT);
        return context;
    }

    EGLSurface createSurface(EGLDisplay display, EGLConfig config)
    {
        const EGLint attribs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 64, EGL_NONE};
        EGLSurface surface     = eglCreatePbufferSurface(display, config, attribs);
        EXPECT_NE(surface, EGL_NO_SURFACE);
        return surface;
    }
};

// This test will terminate an EGL display while a shader program
// is still allocated. This will cause a warmup thread to process
// the shader program after the main() function has exited and
// eventually access freed memory.
TEST_P(EGLRaceConditionTest, RaceAfterTerminate)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr));

    EGLConfig config   = chooseConfig(display);
    EGLContext context = createContext(display, config);
    EGLSurface surface = createSurface(display, config);
    EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context));

    const char *vertSrc = R"(#version 300 es
precision highp float;
in vec4 position;
in vec4 vertex_color;
smooth out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrc = R"(#version 300 es
precision highp float;
smooth in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";

    auto program = CompileProgram(vertSrc, fragSrc);
    INFO() << "Compiled program: " << program;

    // Terminate display while "context" is current.
    EXPECT_EGL_TRUE(eglTerminate(display));
}

ANGLE_INSTANTIATE_TEST(EGLRaceConditionTest, WithNoFixture(ES3_VULKAN()));
