//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ThreadedContextsBenchmark:
//   Performance test for doing things in worker contexts, such as texture upload, program link etc.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "test_utils/MultiThreadSteps.h"
#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
// The following shader is used to create busy work while the worker thread is doing something.  It
// intentionally spreads its uniforms and inputs so the main thread has to make many GL calls.
constexpr char kBusyDrawVS[] = R"(#version 300 es
uniform mediump float x0;
uniform mediump float y0;
uniform mediump float x1;
uniform mediump float y1;

in mediump float r;
in mediump float g;
in mediump float b;
in mediump float a;

out mediump vec4 color;

void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position.x = bit0 == 0 ? x0 : x1;
    gl_Position.y = bit1 == 0 ? y0 : y1;
    gl_Position.z = 0.;
    gl_Position.w = 1.;

    color = vec4(r, g, b, a);
})";
constexpr char kBusyDrawFS[] = R"(#version 300 es

in mediump vec4 color;
out mediump vec4 colorOut;

void main()
{
    colorOut = color;
})";

struct ThreadedContextsParams final : public RenderTestParams
{
    ThreadedContextsParams()
    {
        iterationsPerStep = 1;
        trackGpuTime      = true;

        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 256;
        windowHeight = 512;
    }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const ThreadedContextsParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string ThreadedContextsParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class ThreadedContextsBenchmark : public ANGLERenderTest,
                                  public ::testing::WithParamInterface<ThreadedContextsParams>
{
  public:
    ThreadedContextsBenchmark(const char *benchmarkName);

    void initializeBenchmark() override;
    void destroyBenchmark() override;

  protected:
    GLuint mBusyDrawProgram = 0;
    GLuint mBusyDrawX0Loc   = 0;
    GLuint mBusyDrawY0Loc   = 0;
    GLuint mBusyDrawX1Loc   = 0;
    GLuint mBusyDrawY1Loc   = 0;
    GLuint mBusyDrawRLoc    = 0;
    GLuint mBusyDrawGLoc    = 0;
    GLuint mBusyDrawBLoc    = 0;
    GLuint mBusyDrawALoc    = 0;

    enum class Step
    {
        Start,
        MainThreadStart,
        WorkerTaskEnd,
        Finish,
        Abort,
    };
    Step mCurrentStep = Step::Start;
    std::mutex mStepMutex;
    std::condition_variable mStepCondVar;

    EGLDisplay mDisplay                  = EGL_NO_DISPLAY;
    GLWindowBase::Surface mWorkerSurface = EGL_NO_SURFACE;
    GLWindowContext mWorkerContext       = nullptr;
    std::thread mWorkerThread;
};

class ThreadedContextsCompileLinkBenchmark : public ThreadedContextsBenchmark
{
  public:
    ThreadedContextsCompileLinkBenchmark() : ThreadedContextsBenchmark("CompileLink") {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;

    void drawBenchmark() override;

  private:
    void compilesAndLinks();

    static constexpr uint32_t kProgramCount = 10;

    struct Program
    {
        std::string vertexShader;
        std::string fragmentShader;

        GLuint vs;
        GLuint fs;
        GLuint program;
    };
    std::array<Program, kProgramCount> mPrograms;
};

ThreadedContextsBenchmark::ThreadedContextsBenchmark(const char *benchmarkName)
    : ANGLERenderTest(benchmarkName, GetParam())
{
    addExtensionPrerequisite("GL_EXT_texture_storage");
}

void ThreadedContextsBenchmark::initializeBenchmark()
{
    // Set up the busy draw program
    mBusyDrawProgram = CompileProgram(kBusyDrawVS, kBusyDrawFS);
    ASSERT_NE(mBusyDrawProgram, 0u);

    glUseProgram(mBusyDrawProgram);
    mBusyDrawX0Loc = glGetUniformLocation(mBusyDrawProgram, "x0");
    mBusyDrawY0Loc = glGetUniformLocation(mBusyDrawProgram, "y0");
    mBusyDrawX1Loc = glGetUniformLocation(mBusyDrawProgram, "x1");
    mBusyDrawY1Loc = glGetUniformLocation(mBusyDrawProgram, "y1");
    mBusyDrawRLoc  = glGetAttribLocation(mBusyDrawProgram, "r");
    mBusyDrawGLoc  = glGetAttribLocation(mBusyDrawProgram, "g");
    mBusyDrawBLoc  = glGetAttribLocation(mBusyDrawProgram, "b");
    mBusyDrawALoc  = glGetAttribLocation(mBusyDrawProgram, "a");

    // Set up the worker thread's context
    GLWindowBase *window        = getGLWindow();
    GLWindowContext mainContext = window->getCurrentContextGeneric();

    // Initialize the pbuffer and context
    EGLint pbufferAttributes[] = {
        EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE, EGL_NONE,
    };

    mDisplay       = window->getCurrentDisplay();
    mWorkerSurface = window->createPbufferSurface(pbufferAttributes);
    mWorkerContext = window->createContextGeneric(mainContext);
    ASSERT_NE(mWorkerSurface, nullptr);
    ASSERT_NE(mWorkerContext, nullptr);
}

void ThreadedContextsCompileLinkBenchmark::initializeBenchmark()
{
    ThreadedContextsBenchmark::initializeBenchmark();

    for (uint32_t i = 0; i < kProgramCount; ++i)
    {
        std::ostringstream vs;
        vs << R"(#version 300 es
uniform UBO
{
    vec4 data[)"
           << kProgramCount << R"(];
};

uniform UBO2
{
    mat4 m[2];
} ubo;

in mediump vec4 attr1;
in mediump uvec4 attr2;

out highp vec3 var1;
flat out lowp uvec2 var2;

void main()
{
    vec4 a = attr1 + vec4(attr2);)";
        for (uint32_t j = 0; j < i * 5; ++j)
        {
            vs << R"(
    a *= ubo.m[0];)";
        }
        vs << R"(
    a += ubo.m[1][2];
    var1 = cross(a.zxy, data[)"
           << i << R"(].xxw);
    var2 = uvec2(1, 3) * uvec2(6, 3);
})";
        mPrograms[i].vertexShader = vs.str();

        std::ostringstream fs;
        fs << R"(#version 300 es
uniform UBO
{
    highp vec4 data[)"
           << kProgramCount << R"(];
};

in highp vec3 var1;
flat in lowp uvec2 var2;

uniform mediump sampler2D s;
uniform mediump sampler2D s2;

layout(location = 0) out mediump vec4 color1;
layout(location = 2) out highp uvec2 color2;

void main()
{
    color1 = var1.xyzx + texture(s, vec2(var2) / 9.);
    mediump vec4 sum = vec4(0);
    for (int i = 0; i < 10; ++i)
      sum += texture(s2, vec2(float(i / 2) / 5., float(i % 5) / 5.));
    uvec2 res = uvec2(sum.xz + sum.yw);)";
        for (uint32_t j = 0; j < i * 5; ++j)
        {
            fs << R"(
    res += uvec2(data[)"
               << (j % 100) << R"(]);)";
        }
        fs << R"(
    color2 = res;
})";
        mPrograms[i].fragmentShader = fs.str();

        mPrograms[i].vs      = glCreateShader(GL_VERTEX_SHADER);
        mPrograms[i].fs      = glCreateShader(GL_FRAGMENT_SHADER);
        mPrograms[i].program = glCreateProgram();

        const char *vsCStr = mPrograms[i].vertexShader.c_str();
        const char *fsCStr = mPrograms[i].fragmentShader.c_str();
        glShaderSource(mPrograms[i].vs, 1, &vsCStr, 0);
        glShaderSource(mPrograms[i].fs, 1, &fsCStr, 0);

        glAttachShader(mPrograms[i].program, mPrograms[i].vs);
        glAttachShader(mPrograms[i].program, mPrograms[i].fs);
    }

    mWorkerThread = std::thread([this]() { compilesAndLinks(); });

    ASSERT_GL_NO_ERROR();
}

void ThreadedContextsBenchmark::destroyBenchmark()
{
    {
        std::unique_lock<std::mutex> lock(mStepMutex);
        mCurrentStep = Step::Abort;
        mStepCondVar.notify_all();
    }
    mWorkerThread.join();

    GLWindowBase *window = getGLWindow();

    glDeleteProgram(mBusyDrawProgram);

    window->destroySurface(mWorkerSurface);
    eglDestroyContext(mDisplay, mWorkerContext);
}

void ThreadedContextsCompileLinkBenchmark::destroyBenchmark()
{
    ThreadedContextsBenchmark::destroyBenchmark();

    for (uint32_t i = 0; i < kProgramCount; ++i)
    {
        glDetachShader(mPrograms[i].program, mPrograms[i].vs);
        glDetachShader(mPrograms[i].program, mPrograms[i].fs);
        glDeleteShader(mPrograms[i].vs);
        glDeleteShader(mPrograms[i].fs);
        glDeleteProgram(mPrograms[i].program);
    }
}

void ThreadedContextsCompileLinkBenchmark::compilesAndLinks()
{
    eglMakeCurrent(mDisplay, mWorkerSurface, mWorkerSurface, mWorkerContext);

    ThreadSynchronization<Step> threadSynchronization(&mCurrentStep, &mStepMutex, &mStepCondVar);

    while (threadSynchronization.pollStep() != Step::Abort)
    {
        // Wait for the benchmark to start
        if (!threadSynchronization.waitForStep(Step::MainThreadStart))
        {
            break;
        }

        for (uint32_t i = 0; i < kProgramCount; ++i)
        {
            glCompileShader(mPrograms[i].vs);
            glCompileShader(mPrograms[i].fs);
            glLinkProgram(mPrograms[i].program);
        }

        // Signal that the compiles and links are finished.
        threadSynchronization.nextStep(Step::WorkerTaskEnd);
        threadSynchronization.waitForStep(Step::Finish);

        // Prepare for next iteration of the perf test.
        threadSynchronization.nextStep(Step::Start);
    }

    eglMakeCurrent(getGLWindow()->getCurrentDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
}

void ThreadedContextsCompileLinkBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    ThreadSynchronization<Step> threadSynchronization(&mCurrentStep, &mStepMutex, &mStepCondVar);

    // Make sure the worker thread is back to the start.
    while (threadSynchronization.pollStep() != Step::Start)
    {
        // Busy wait; this should never happen anyway.
        if (threadSynchronization.pollStep() == Step::Abort)
        {
            return;
        }
    }

    const int w = params.windowWidth;
    const int h = params.windowHeight;

    // Let the worker thread do compile/link, and perform draws at the same time.
    threadSynchronization.nextStep(Step::MainThreadStart);

    startGpuTimer();

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, w, h);
    for (uint32_t y = 0; y < 8; ++y)
    {
        for (uint32_t x = 0; x < 8; ++x)
        {
            float width  = w / 4;
            float height = h / 8;

            glUseProgram(mBusyDrawProgram);
            glUniform1f(mBusyDrawX0Loc, x * width / w - 1);
            glUniform1f(mBusyDrawY0Loc, y * height / h);
            glUniform1f(mBusyDrawX1Loc, (x + 1) * width / w - 1);
            glUniform1f(mBusyDrawY1Loc, (y + 1) * height / h);

            glVertexAttrib1f(mBusyDrawRLoc, x / 8.0f);
            glVertexAttrib1f(mBusyDrawGLoc, y / 8.0f);
            glVertexAttrib1f(mBusyDrawBLoc, 0);
            glVertexAttrib1f(mBusyDrawALoc, 1);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    // Wait for worker thread to finish.  Ideally, it should already be done.
    threadSynchronization.waitForStep(Step::WorkerTaskEnd);

    stopGpuTimer();

    threadSynchronization.nextStep(Step::Finish);

    ASSERT_GL_NO_ERROR();
}

ThreadedContextsParams VulkanParams()
{
    ThreadedContextsParams params;
    params.eglParameters = egl_platform::VULKAN();
    return params;
}
}  // anonymous namespace

// Measure performance of the case where an application has a thread dedicated to texture uploads.
TEST_P(ThreadedContextsCompileLinkBenchmark, Run)
{
    run();
}

using namespace params;

// Note: only the Vulkan backend is thread safe.
ANGLE_INSTANTIATE_TEST(ThreadedContextsCompileLinkBenchmark,
                       NullDevice(VulkanParams()),
                       VulkanParams());
