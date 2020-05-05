//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandProcessor.h:
//    A class to process and submit Vulkan command buffers that can be
//    used in an asynchronous worker thread.
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
#define LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "libANGLE/renderer/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class CommandProcessor : angle::NonCopyable
{
  public:
    CommandProcessor();
    ~CommandProcessor() = default;

    // Main worker loop that should be launched in its own thread. The
    //  loop waits for work to be submitted from a separate thread.
    angle::Result workLoop();
    // Called asynchronously from workLoop() thread to queue work that is
    //  then processed by the workLoop() thread
    void queueCommands(vk::CommandWorkBlock commandWork);
    // Used by separate thread to wait for worker thread to complete all
    //  outstanding work.
    void waitForWorkComplete();

  private:
    // Worker Thread related members
    std::queue<vk::CommandWorkBlock> mCommandsQueue;
    std::thread mWorkerThread;
    std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
