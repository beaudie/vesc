//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MulithreadingTest.cpp : Tests of multithreaded rendering

#include "platform/FeaturesVk.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <atomic>
#include <mutex>
#include <thread>

#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace angle
{

class MultithreadingTest : public ANGLETest
{
  public:
    rx::ContextVk *hackANGLE() const
    {
        // Hack the angle!
        const gl::Context *context = static_cast<gl::Context *>(getEGLWindow()->getContext());
        return rx::GetImplAs<rx::ContextVk>(context);
    }

  protected:
    MultithreadingTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mMaxSetsPerPool = rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolForTesting();
        mMaxSetsPerPoolMultiplier =
            rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolMultiplierForTesting();
    }

    void testTearDown() override
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(mMaxSetsPerPool);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            mMaxSetsPerPoolMultiplier);
    }

    static constexpr uint32_t kMaxSetsForTesting           = 1;
    static constexpr uint32_t kMaxSetsMultiplierForTesting = 1;

    void limitMaxSets()
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(kMaxSetsForTesting);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            kMaxSetsMultiplierForTesting);
    }

    void runMultithreadedGLTest(std::function<void(MultithreadingTest *multithreadingTest,
                                                   EGLSurface surface,
                                                   size_t threadIndex)> testBody,
                                MultithreadingTest *multithreadingTest,
                                size_t threadCount)
    {
        std::mutex mutex;

        EGLWindow *window = getEGLWindow();
        EGLDisplay dpy    = window->getDisplay();
        EGLConfig config  = window->getConfig();

        constexpr EGLint kPBufferSize = 256;

        std::vector<std::thread> threads(threadCount);
        for (size_t threadIdx = 0; threadIdx < threadCount; threadIdx++)
        {
            threads[threadIdx] = std::thread([&, threadIdx]() {
                EGLSurface surface = EGL_NO_SURFACE;
                EGLContext ctx     = EGL_NO_CONTEXT;

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);

                    // Initialize the pbuffer and context
                    EGLint pbufferAttributes[] = {
                        EGL_WIDTH, kPBufferSize, EGL_HEIGHT, kPBufferSize, EGL_NONE, EGL_NONE,
                    };
                    surface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
                    EXPECT_EGL_SUCCESS();

                    ctx = window->createContext(EGL_NO_CONTEXT);
                    EXPECT_NE(EGL_NO_CONTEXT, ctx);

                    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
                    EXPECT_EGL_SUCCESS();
                }

                testBody(multithreadingTest, surface, threadIdx);

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);

                    // Clean up
                    EXPECT_EGL_TRUE(
                        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
                    EXPECT_EGL_SUCCESS();

                    eglDestroySurface(dpy, surface);
                    eglDestroyContext(dpy, ctx);
                }
            });
        }

        for (std::thread &thread : threads)
        {
            thread.join();
        }
    }

  private:
    uint32_t mMaxSetsPerPool;
    uint32_t mMaxSetsPerPoolMultiplier;
};

// Test that it's possible to make one context current on different threads
TEST_P(MultithreadingTest, MakeCurrentSingleContext)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    std::mutex mutex;

    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext ctx     = window->getContext();
    EGLSurface surface = window->getSurface();

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    constexpr size_t kThreadCount = 16;
    std::array<std::thread, kThreadCount> threads;
    for (std::thread &thread : threads)
    {
        thread = std::thread([&]() {
            std::lock_guard<decltype(mutex)> lock(mutex);

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
            EXPECT_EGL_SUCCESS();

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
            EXPECT_EGL_SUCCESS();
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();
}

// Test that multiple threads can clear and readback pixels successfully at the same time
TEST_P(MultithreadingTest, MultiContextClear)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    auto testBody = [](MultithreadingTest *multithreadingTest, EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();

            glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
            EXPECT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_GL_NO_ERROR();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, this, 72);
}

// Test that multiple threads can draw and readback pixels successfully at the same time
TEST_P(MultithreadingTest, MultiContextDraw)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    auto testBody = [](MultithreadingTest *multithreadingTest, EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        constexpr size_t kDrawsPerIteration   = 500;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, this, 4);
}

// Test that multiple threads can draw and read back pixels correctly.
// Using eglSwapBuffers stresses race conditions around use of QueueSerials.
TEST_P(MultithreadingTest, MultiContextDrawWithSwapBuffers)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // http://anglebug.com/5099
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    auto testBody = [dpy](MultithreadingTest *multithreadingTest, EGLSurface surface,
                          size_t thread) {
        constexpr size_t kIterationsPerThread = 100;
        constexpr size_t kDrawsPerIteration   = 10;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, this, 32);
}

// Test that multiple threads can draw and readback pixels successfully at the same time with small
// descriptor pools.
TEST_P(MultithreadingTest, MultiContextDrawSmallDescriptorPools)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // Must be before program creation to limit the descriptor pool sizes when creating the pipeline
    // layout.
    limitMaxSets();

    auto testBody = [](MultithreadingTest *multithreadingTest, EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        constexpr size_t kDrawsPerIteration   = 500;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Slowly grow the viewport so it changes each iteration and forces an update to the
            // driver uniforms.
            glViewport(0, 0, iteration + 1, iteration + 1);

            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, this, 4);
}

// Test that multiple threads can draw and read back pixels correctly with small descriptor pools.
// Using eglSwapBuffers stresses race conditions around use of QueueSerials.
TEST_P(MultithreadingTest, MultiContextDrawWithSwapBuffersSmallDescriptorPools)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // http://anglebug.com/5099
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // Must be before program creation to limit the descriptor pool sizes when creating the pipeline
    // layout.
    limitMaxSets();

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    auto testBody = [dpy](MultithreadingTest *multithreadingTest, EGLSurface surface,
                          size_t thread) {
        constexpr size_t kIterationsPerThread = 100;
        constexpr size_t kDrawsPerIteration   = 10;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Set a really small min size so that uniform updates often allocates a new buffer.
            rx::ContextVk *contextVk = multithreadingTest->hackANGLE();
            contextVk->setDefaultUniformBlocksMinSizeForTesting(32);

            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                // Change the viewport so it changes each iteration and forces an update to the
                // driver uniforms.
                glViewport(0, 0, iteration + draw + thread + 1, iteration + draw + thread + 1);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, this, 32);
}

// Test that ANGLE handles multiple threads creating and destroying resources (vertex buffer in this
// case). Disable defer_flush_until_endrenderpass so that glFlush will issue work to GPU in order to
// maximize the chance we resources can be destroyed at the wrong time.
TEST_P(MultithreadingTest, MultiContextCreateAndDeleteResources)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    auto testBody = [dpy](MultithreadingTest *multithreadingTest, EGLSurface surface,
                          size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        constexpr size_t kDrawsPerIteration   = 1;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            GLBuffer vertexBuffer;
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(),
                         GL_STATIC_DRAW);

            GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
        glFinish();
    };
    runMultithreadedGLTest(testBody, this, 32);
}

TEST_P(MultithreadingTest, MultiCreateContext)
{
    // Supported by CGL, GLX, and WGL (https://anglebug.com/4725)
    // Not supported on Ozone (https://crbug.com/1103009)
    ANGLE_SKIP_TEST_IF(!(IsWindows() || IsLinux() || IsOSX()) || IsOzone());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext ctx     = window->getContext();
    EGLSurface surface = window->getSurface();

    // Un-makeCurrent the test window's context
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    constexpr size_t kThreadCount = 16;
    std::atomic<uint32_t> barrier(0);
    std::vector<std::thread> threads(kThreadCount);
    std::vector<EGLContext> contexts(kThreadCount);
    for (size_t threadIdx = 0; threadIdx < kThreadCount; threadIdx++)
    {
        threads[threadIdx] = std::thread([&, threadIdx]() {
            contexts[threadIdx] = EGL_NO_CONTEXT;
            {
                contexts[threadIdx] = window->createContext(EGL_NO_CONTEXT);
                EXPECT_NE(EGL_NO_CONTEXT, contexts[threadIdx]);

                barrier++;
            }

            while (barrier < kThreadCount)
            {
            }

            {
                EXPECT_TRUE(eglDestroyContext(dpy, contexts[threadIdx]));
            }
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    // Re-make current the test window's context for teardown.
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();
}

// TODO(geofflang): Test sharing a program between multiple shared contexts on multiple threads

ANGLE_INSTANTIATE_TEST(MultithreadingTest,
                       WithNoVirtualContexts(ES2_OPENGL()),
                       WithNoVirtualContexts(ES3_OPENGL()),
                       WithNoVirtualContexts(ES2_OPENGLES()),
                       WithNoVirtualContexts(ES3_OPENGLES()),
                       WithNoVirtualContexts(ES2_VULKAN()),
                       WithNoVirtualContexts(ES3_VULKAN()));

}  // namespace angle
