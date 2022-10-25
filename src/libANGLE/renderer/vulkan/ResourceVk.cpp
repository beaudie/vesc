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
    if (resource->usedInRunningCommands(contextVk->getRenderer()))
    {
        if (debugMessage)
        {
            ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH, "%s", debugMessage);
        }
        ANGLE_TRY(resource->finishRunningCommands(contextVk));
    }

    ASSERT(!resource->isCurrentlyInUse(contextVk->getRenderer()));

    return angle::Result::Continue;
}
}  // namespace

// Resource implementation.
Resource::Resource() {}

Resource::Resource(Resource &&other) : Resource()
{
    mUse = std::move(other.mUse);
}

Resource &Resource::operator=(Resource &&rhs)
{
    std::swap(mUse, rhs.mUse);
    return *this;
}

Resource::~Resource() {}

bool Resource::usedInRunningCommands(RendererVk *renderer) const
{
    if (!mUse.valid())
    {
        return false;
    }
    return renderer->hasUnfinishedUse(mUse);
}

bool Resource::isCurrentlyInUse(RendererVk *renderer) const
{
    if (!mUse.valid())
    {
        return false;
    }
    return renderer->hasUnfinishedUse(mUse);
}

angle::Result Resource::finishRunningCommands(ContextVk *contextVk)
{
    if (!mUse.valid())
    {
        return angle::Result::Continue;
    }
    return contextVk->finishToSerials(mUse.getSerials());
}

angle::Result Resource::waitForIdle(ContextVk *contextVk,
                                    const char *debugMessage,
                                    RenderPassClosureReason reason)
{
    return WaitForIdle(contextVk, this, debugMessage, reason);
}

// ReadWriteResource implementation.
ReadWriteResource::ReadWriteResource() {}

ReadWriteResource::ReadWriteResource(ReadWriteResource &&other) : ReadWriteResource()
{
    *this = std::move(other);
}

ReadWriteResource::~ReadWriteResource() {}

ReadWriteResource &ReadWriteResource::operator=(ReadWriteResource &&other)
{
    mReadOnlyUse  = std::move(other.mReadOnlyUse);
    mReadWriteUse = std::move(other.mReadWriteUse);
    return *this;
}

bool ReadWriteResource::usedInRunningCommands(RendererVk *renderer) const
{
    if (!mReadOnlyUse.valid())
    {
        return false;
    }
    return renderer->hasUnfinishedUse(mReadOnlyUse);
}

bool ReadWriteResource::isCurrentlyInUse(RendererVk *renderer) const
{
    if (!mReadOnlyUse.valid())
    {
        return false;
    }
    return renderer->hasUnfinishedUse(mReadOnlyUse);
}

bool ReadWriteResource::isCurrentlyInUseForWrite(RendererVk *renderer) const
{
    if (!mReadWriteUse.valid())
    {
        return false;
    }
    return renderer->hasUnfinishedUse(mReadWriteUse);
}

angle::Result ReadWriteResource::finishRunningCommands(ContextVk *contextVk)
{
    ASSERT(!mReadOnlyUse.usedInRecordedCommand(contextVk));
    return contextVk->finishToSerials(mReadOnlyUse.getSerials());
}

angle::Result ReadWriteResource::finishGPUWriteCommands(ContextVk *contextVk)
{
    ASSERT(!mReadWriteUse.usedInRecordedCommand(contextVk));
    return contextVk->finishToSerials(mReadWriteUse.getSerials());
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

SharedGarbage::SharedGarbage(const ResourceUse &use, std::vector<GarbageObject> &&garbage)
    : mLifetime(use), mGarbage(std::move(garbage))
{}

SharedGarbage::~SharedGarbage() = default;

SharedGarbage &SharedGarbage::operator=(SharedGarbage &&rhs)
{
    std::swap(mLifetime, rhs.mLifetime);
    std::swap(mGarbage, rhs.mGarbage);
    return *this;
}

bool SharedGarbage::destroyIfComplete(RendererVk *renderer)
{
    if (renderer->hasUnfinishedUse(mLifetime))
    {
        return false;
    }

    for (GarbageObject &object : mGarbage)
    {
        object.destroy(renderer);
    }

    return true;
}

bool SharedGarbage::usedInRecordedCommands(RendererVk *renderer) const
{
    return renderer->hasUnsubmittedUse(mLifetime);
}

}  // namespace vk
}  // namespace rx
