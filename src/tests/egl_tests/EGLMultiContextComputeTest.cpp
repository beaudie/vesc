//
// Copyright 2016, 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLMultiContextComputeTest.cpp:
//   Tests relating to multiple non-shared Contexts.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace angle;

namespace
{

EGLBoolean SafeDestroyContext(EGLDisplay display, EGLContext &context)
{
    EGLBoolean result = EGL_TRUE;
    if (context != EGL_NO_CONTEXT)
    {
        result  = eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
    }
    return result;
}

class EGLMultiContextComputeTest : public ANGLETest
{
  public:
    EGLMultiContextComputeTest() : mContexts{EGL_NO_CONTEXT, EGL_NO_CONTEXT}, mTexture(0) {}

    void testTearDown() override
    {
        glDeleteTextures(1, &mTexture);

        EGLDisplay display = getEGLWindow()->getDisplay();

        if (display != EGL_NO_DISPLAY)
        {
            for (auto &context : mContexts)
            {
                SafeDestroyContext(display, context);
            }
        }

        // Set default test state to not give an error on shutdown.
        getEGLWindow()->makeCurrent();
    }

    EGLContext mContexts[2];
    GLuint mTexture;
};

// Test that a compute shader running in one thread will still work when rendering is happening in
// another thread (with non-shared contexts).  The non-shared context will still share a Vulkan
// command buffer.
TEST_P(EGLMultiContextComputeTest, ComputeShaderOkayWithRendering)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    // GL Fences require GLES 3.0+
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 || getClientMinorVersion() < 1);

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint pbufferAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE};

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
        EXPECT_EGL_SUCCESS();

        ctx[t] = window->createContext(EGL_NO_CONTEXT);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
    }

    // Initialize test resources.  They are done outside the threads to reduce the sources of
    // errors and thus deadlock-free teardown.
    ASSERT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));

    // Shared texture to read from.
    constexpr GLsizei kTexSize = 1;
    const GLColor kTexData     = GLColor::red;

    GLTexture sharedTex;
    glBindTexture(GL_TEXTURE_2D, sharedTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &kTexData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Resources for each context.
    GLRenderbuffer renderbuffer[kThreadCount];
    GLFramebuffer fbo[kThreadCount];
    GLProgram program[kThreadCount];

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        ASSERT_EGL_TRUE(eglMakeCurrent(dpy, surface[t], surface[t], ctx[t]));

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer[t]);
        constexpr int kRenderbufferSize = 4;
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kRenderbufferSize, kRenderbufferSize);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo[t]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer[t]);

        glBindTexture(GL_TEXTURE_2D, sharedTex);
        program[t].makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        ASSERT_TRUE(program[t].valid());
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Thread0Start,
        Thread0DispatchedCompute,
        Thread1Drew,
        Thread0DispatchedComputeAgain,
        Finish,
        Abort,
    };
    Step currentStep = Step::Thread0Start;

    // Helper functions to synchronize the threads so that the operations are executed in the
    // specific order the test is written for.
    auto waitForStep = [&](Step waitStep) -> bool {
        std::unique_lock<std::mutex> lock(mutex);
        while (currentStep != waitStep)
        {
            // If necessary, abort execution as the other thread has encountered a GL error.
            if (currentStep == Step::Abort)
            {
                return false;
            }
            condVar.wait(lock);
        }

        return true;
    };
    auto nextStep = [&](Step newStep) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            currentStep = newStep;
        }
        condVar.notify_one();
    };

    class AbortOnFailure
    {
      public:
        AbortOnFailure(Step *currentStep, std::mutex *mutex, std::condition_variable *condVar)
            : mCurrentStep(currentStep), mMutex(mutex), mCondVar(condVar)
        {}

        ~AbortOnFailure()
        {
            bool isAborting = false;
            {
                std::unique_lock<std::mutex> lock(*mMutex);
                isAborting = *mCurrentStep != Step::Finish;

                if (isAborting)
                {
                    *mCurrentStep = Step::Abort;
                }
            }
            mCondVar->notify_all();
        }

      private:
        Step *mCurrentStep;
        std::mutex *mMutex;
        std::condition_variable *mCondVar;
    };

    // This first thread dispatches a compute shader.  It immediately starts.
    std::thread deletingThread = std::thread([&]() {
        AbortOnFailure abortOnFailure(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        ASSERT_TRUE(waitForStep(Step::Thread0Start));

        // FIXME: ADD COMPUTE SHADER AND DISPATCH

        constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

        GLuint program = glCreateProgram();

        GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);
        EXPECT_NE(0u, cs);

        glAttachShader(program, cs);
        glDeleteShader(cs);

        glLinkProgram(program);
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        EXPECT_GL_TRUE(linkStatus);

        glDetachShader(program, cs);
        EXPECT_GL_NO_ERROR();

        glUseProgram(program);
        glDispatchCompute(8, 4, 2);
        EXPECT_GL_NO_ERROR();

        // Wait for the other thread to draw.
        nextStep(Step::Thread0DispatchedCompute);
        ASSERT_TRUE(waitForStep(Step::Thread1Drew));

        // FIXME: ADD COMPUTE SHADER DISPATCH
        glDispatchCompute(8, 4, 2);

        // Wait for the other thread to draw again and finish.
        nextStep(Step::Thread0DispatchedComputeAgain);
        ASSERT_TRUE(waitForStep(Step::Finish));

        // FIXME: ADD COMPUTE SHADER DISPATCH
        glDispatchCompute(8, 4, 2);

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // This second thread renders.  It starts once the other thread does its first nextStep()
    std::thread continuingThread = std::thread([&]() {
        AbortOnFailure abortOnFailure(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        // Wait for first thread to create and dispatch a compute shader.
        ASSERT_TRUE(waitForStep(Step::Thread0DispatchedCompute));

        // Draw using the shared texture.
        drawQuad(program[0].get(), essl1_shaders::PositionAttrib(), 0.5f);

        // Force the fence to be created
        glFlush();

        // Wait for the other thread to dispatch a compute shader again.
        nextStep(Step::Thread1Drew);
        ASSERT_TRUE(waitForStep(Step::Thread0DispatchedComputeAgain));

        // FIXME: CLEAN UP THE COMMENTS
        // Write to the shared texture differently, so a dependency is created from the previous
        // readers of the shared texture (the two framebuffers of the two threads) to the new
        // commands being recorded for the shared texture.
        //
        // If the backend attempts to create a dependency from nodes associated with the
        // previous readers of the texture to the new node that will contain the following
        // commands, there will be a use-after-free error.
        const GLColor kTexData2 = GLColor::green;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     &kTexData2);
        drawQuad(program[0].get(), essl1_shaders::PositionAttrib(), 0.5f);

        nextStep(Step::Finish);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    deletingThread.join();
    continuingThread.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}
}  // anonymous namespace

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLMultiContextComputeTest);
ANGLE_INSTANTIATE_TEST_ES31(EGLMultiContextComputeTest);
