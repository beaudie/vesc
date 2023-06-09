//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextMutexTest:
//   Tests SharedContextMutex functionality.
//

#include "test_utils/ANGLETest.h"

#include "libANGLE/Display.h"

using namespace angle;

namespace
{

class ContextMutexTest : public ANGLETest<>
{
  protected:
    gl::Context *hackContext() const
    {
        egl::Display *display   = static_cast<egl::Display *>(getEGLWindow()->getDisplay());
        gl::ContextID contextID = {
            static_cast<GLuint>(reinterpret_cast<uintptr_t>(getEGLWindow()->getContext()))};
        return display->getContext(contextID);
    }
};

// Tests if current kActivationDelayMicro is sufficient to safely activate SharedContextMutex.
TEST_P(ContextMutexTest, SharedContextMutexActivation)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr uint32_t kNumTestIterations = 1000;
    constexpr double kIterationLengthSec  = 0.01;

    gl::Context *context = hackContext();

    std::atomic_bool needRun(true);
    std::atomic_bool needRevert(false);
    std::atomic_bool isRunning(false);

    std::thread thread([&]() {
        while (needRun)
        {
            if (needRevert)
            {
                context->tryRevertToSingleContextMutex(gl::SingleContextMutexRevert::kEnforced);
                needRevert = false;
            }
            // Emulate busy Context thread.
            for (uint32_t i = 0; i < 100; ++i)
            {
                std::lock_guard<egl::ContextMutex> lock(*context->getContextMutex());
                EXPECT_TRUE(context->isContextMutexStateConsistent());
            }
            isRunning = 1;
        }
    });

    while (!isRunning)
    {
        std::this_thread::yield();
    }

    for (uint32_t i = 0; i < kNumTestIterations; ++i)
    {
        const double beginTime = angle::GetCurrentSystemTime();
        while (angle::GetCurrentSystemTime() - beginTime < kIterationLengthSec)
        {
            // Emulate busy non Context thread, that activates SharedContextMutex.
            for (uint32_t j = 0; j < 100; ++j)
            {
                egl::ScopedContextMutexLock lock = context->lockAndActivateSharedContextMutex(
                    gl::SharedContextMutexActivation::kTemporary);
                EXPECT_TRUE(context->isContextMutexStateConsistent());
            }
        }
        needRevert = true;
        while (needRevert)
        {
            std::this_thread::yield();
        }
    }

    needRun = false;
    thread.join();
}

ANGLE_INSTANTIATE_TEST(ContextMutexTest, ES2_VULKAN());

}  // anonymous namespace
