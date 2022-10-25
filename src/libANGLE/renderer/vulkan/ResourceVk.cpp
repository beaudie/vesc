//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Resource:
//    Resource lifetime tracking in the Vulkan back-end.
//

#include "libANGLE/renderer/vulkan/ResourceVk.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{
namespace vk
{
namespace
{
angle::Result FinishRunningCommands(ContextVk *contextVk, const QueueSerials &queueSerials)
{
    return contextVk->finishToSerials(queueSerials);
}

template <typename T>
angle::Result WaitForIdle(ContextVk *contextVk,
                          T *resource,
                          const char *debugMessage,
                          RenderPassClosureReason reason)
{
    // If there are pending commands for the resource, flush them.
    if (resource->usedInRecordedCommand(contextVk))
    {
        ANGLE_TRY(contextVk->flushImpl(nullptr, reason));
    }

    // Make sure the driver is done with the resource.
    if (resource->usedInRunningCommands(contextVk->getLastCompletedQueueSerials()))
    {
        if (debugMessage)
        {
            ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH, "%s", debugMessage);
        }
        ANGLE_TRY(resource->finishRunningCommands(contextVk));
    }

    ASSERT(!resource->isCurrentlyInUse(contextVk->getLastCompletedQueueSerials()));

    return angle::Result::Continue;
}
}  // namespace

// Resource implementation.
Resource::Resource()
{
    mUse.init();
}

Resource::Resource(Resource &&other) : Resource()
{
    mUse = std::move(other.mUse);
}

Resource &Resource::operator=(Resource &&rhs)
{
    std::swap(mUse, rhs.mUse);
    return *this;
}

Resource::~Resource()
{
    mUse.release();
}

angle::Result Resource::finishRunningCommands(ContextVk *contextVk)
{
    return FinishRunningCommands(contextVk, mUse.getQueueSerials());
}

angle::Result Resource::waitForIdle(ContextVk *contextVk,
                                    const char *debugMessage,
                                    RenderPassClosureReason reason)
{
    return WaitForIdle(contextVk, this, debugMessage, reason);
}

// Resource implementation.
ReadWriteResource::ReadWriteResource()
{
    mReadOnlyUse.init();
    mReadWriteUse.init();
}

ReadWriteResource::ReadWriteResource(ReadWriteResource &&other) : ReadWriteResource()
{
    *this = std::move(other);
}

ReadWriteResource::~ReadWriteResource()
{
    mReadOnlyUse.release();
    mReadWriteUse.release();
}

ReadWriteResource &ReadWriteResource::operator=(ReadWriteResource &&other)
{
    mReadOnlyUse  = std::move(other.mReadOnlyUse);
    mReadWriteUse = std::move(other.mReadWriteUse);
    return *this;
}

angle::Result ReadWriteResource::finishRunningCommands(ContextVk *contextVk)
{
    ASSERT(!mReadOnlyUse.usedInRecordedCommand(contextVk->getSerialIndex(),
                                               contextVk->getLastSubmittedSerial()));
    return FinishRunningCommands(contextVk, mReadOnlyUse.getQueueSerials());
}

angle::Result ReadWriteResource::finishGPUWriteCommands(ContextVk *contextVk)
{
    ASSERT(!mReadWriteUse.usedInRecordedCommand(contextVk->getSerialIndex(),
                                                contextVk->getLastSubmittedSerial()));
    return FinishRunningCommands(contextVk, mReadWriteUse.getQueueSerials());
}

angle::Result ReadWriteResource::waitForIdle(ContextVk *contextVk,
                                             const char *debugMessage,
                                             RenderPassClosureReason reason)
{
    return WaitForIdle(contextVk, this, debugMessage, reason);
}

// SharedGarbage implementation.
SharedGarbage::SharedGarbage() = default;

SharedGarbage::SharedGarbage(SharedGarbage &&other)
{
    *this = std::move(other);
}

SharedGarbage::SharedGarbage(SharedResourceUse &&use, std::vector<GarbageObject> &&garbage)
    : mLifetime(std::move(use)), mGarbage(std::move(garbage))
{}

SharedGarbage::~SharedGarbage() = default;

SharedGarbage &SharedGarbage::operator=(SharedGarbage &&rhs)
{
    std::swap(mLifetime, rhs.mLifetime);
    std::swap(mGarbage, rhs.mGarbage);
    return *this;
}

bool SharedGarbage::destroyIfComplete(RendererVk *renderer,
                                      const QueueSerials &completedQueueSerials)
{
    if (mLifetime.isCurrentlyInUse(completedQueueSerials))
    {
        return false;
    }

    for (GarbageObject &object : mGarbage)
    {
        object.destroy(renderer);
    }

    mLifetime.release();

    return true;
}

}  // namespace vk
}  // namespace rx
