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

// The following shader is used to consume the results of texture uploads, ensuring appropriate
// synchronization.
constexpr char kTextureDrawVS[] = R"(#version 300 es
out mediump vec2 uv;

void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
    uv = vec2(bit0, bit1);
})";
constexpr char kTextureDrawFS[] = R"(#version 300 es

uniform mediump sampler2D s0;
uniform mediump sampler2D s1;
uniform mediump sampler2D s2;
uniform mediump sampler2D s3;
uniform mediump sampler2D s4;
uniform mediump sampler2D s5;
uniform mediump sampler2D s6;
uniform mediump sampler2D s7;
uniform mediump sampler2D s8;
uniform mediump sampler2D s9;

in mediump vec2 uv;
out mediump vec4 colorOut;

void main()
{
    highp vec4 result = texture(s0, uv) +
                        texture(s1, uv) +
                        texture(s2, uv) +
                        texture(s3, uv) +
                        texture(s4, uv) +
                        texture(s5, uv) +
                        texture(s6, uv) +
                        texture(s7, uv) +
                        texture(s8, uv) +
                        texture(s9, uv);
    result /= 10.;

    colorOut = result;
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

    GLsizei textureSize = 1024;
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
    GLsync mFence = 0;

    EGLDisplay mDisplay                  = EGL_NO_DISPLAY;
    GLWindowBase::Surface mWorkerSurface = EGL_NO_SURFACE;
    GLWindowContext mWorkerContext       = nullptr;
    std::thread mWorkerThread;
};

class ThreadedContextsTextureUploadBenchmark : public ThreadedContextsBenchmark
{
  public:
    ThreadedContextsTextureUploadBenchmark() : ThreadedContextsBenchmark("TextureUpload") {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;

    void drawBenchmark() override;

  private:
    void uploadTextureData();

    GLenum getCompressedTextureFormat()
    {
        ASSERT(IsGLExtensionEnabled("GL_KHR_texture_compression_astc_ldr") ||
               IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));
        // Note ASTC may be emulated in ANGLE, so check for BPTC first
        const bool hasBPTC = IsGLExtensionEnabled("GL_EXT_texture_compression_bptc");
        return hasBPTC ? GL_COMPRESSED_RGBA_BPTC_UNORM_EXT : GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
    }

    static constexpr uint32_t kTextureCount = 10;

    GLuint mTextureDrawProgram                   = 0;
    GLuint mTextureDrawSamplerLoc[kTextureCount] = {};

    GLuint mTextures[kTextureCount] = {};

    std::vector<uint8_t> mTextureData[kTextureCount];
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

void ThreadedContextsTextureUploadBenchmark::initializeBenchmark()
{
    ThreadedContextsBenchmark::initializeBenchmark();

    const auto &params            = GetParam();
    const GLenum compressedFormat = getCompressedTextureFormat();

    // Set up the texture upload program
    mTextureDrawProgram = CompileProgram(kTextureDrawVS, kTextureDrawFS);
    ASSERT_NE(mTextureDrawProgram, 0u);

    glGenTextures(kTextureCount, mTextures);

    glUseProgram(mTextureDrawProgram);
    for (uint32_t i = 0; i < kTextureCount; ++i)
    {
        std::ostringstream name;
        name << "s" << i;

        mTextureDrawSamplerLoc[i] = glGetUniformLocation(mTextureDrawProgram, name.str().c_str());

        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, compressedFormat, params.textureSize, params.textureSize);

        // Both ASTC 4x4 and BPTC have 1 byte per pixel.  The textures contents are arbitrary
        // but distinct.
        mTextureData[i].resize(params.textureSize * params.textureSize);
        for (GLsizei y = 0; y < params.textureSize; ++y)
        {
            for (GLsizei x = 0; x < params.textureSize; ++x)
            {
                mTextureData[i][y * params.textureSize + x] = (i * 50 + y + x) % 255;
            }
        }
    }

    mWorkerThread = std::thread([this]() { uploadTextureData(); });

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

void ThreadedContextsTextureUploadBenchmark::destroyBenchmark()
{
    ThreadedContextsBenchmark::destroyBenchmark();

    glDeleteProgram(mTextureDrawProgram);
    glDeleteTextures(kTextureCount, mTextures);
}

void ThreadedContextsTextureUploadBenchmark::uploadTextureData()
{
    eglMakeCurrent(mDisplay, mWorkerSurface, mWorkerSurface, mWorkerContext);

    const auto &params            = GetParam();
    const GLenum compressedFormat = getCompressedTextureFormat();

    ThreadSynchronization<Step> threadSynchronization(&mCurrentStep, &mStepMutex, &mStepCondVar);

    while (threadSynchronization.pollStep() != Step::Abort)
    {
        // Wait for the benchmark to start
        if (!threadSynchronization.waitForStep(Step::MainThreadStart))
        {
            break;
        }

        for (uint32_t i = 0; i < kTextureCount; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[i]);
            glCompressedTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0, params.textureSize, params.textureSize, compressedFormat,
                static_cast<GLsizei>(mTextureData[i].size()), mTextureData[i].data());
        }

        mFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // Signal that the copies are finished.
        threadSynchronization.nextStep(Step::WorkerTaskEnd);
        threadSynchronization.waitForStep(Step::Finish);

        // Prepare for next iteration of the perf test.
        threadSynchronization.nextStep(Step::Start);
    }

    eglMakeCurrent(getGLWindow()->getCurrentDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
}

void ThreadedContextsTextureUploadBenchmark::drawBenchmark()
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

    // Let the worker thread do copies, and perform draws at the same time.
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

    // Wait for fence and use all textures in a draw.
    glWaitSync(mFence, 0, GL_TIMEOUT_IGNORED);

    glUseProgram(mTextureDrawProgram);
    for (uint32_t i = 0; i < kTextureCount; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glUniform1i(mTextureDrawSamplerLoc[i], i);
    }
    glViewport(0, 0, w, h / 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
TEST_P(ThreadedContextsTextureUploadBenchmark, Run)
{
    run();
}

using namespace params;

// Note: only the Vulkan backend is thread safe.
ANGLE_INSTANTIATE_TEST(ThreadedContextsTextureUploadBenchmark,
                       NullDevice(VulkanParams()),
                       VulkanParams());
