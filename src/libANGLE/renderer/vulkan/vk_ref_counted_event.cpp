//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#include "libANGLE/renderer/vulkan/vk_ref_counted_event.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace vk
{
void RefCountedEvent::init(Context *context, ImageLayout layout)
{
    ASSERT(mHandle == nullptr);
    ASSERT(layout != ImageLayout::Undefined);

    // First try with share group event recycler. If that failed, try with per renderer event
    // recycler.
    if (!context->getRefCountedEventRecycler()->fetch(context, this) &&
        !context->getRenderer()->getRefCountedEventRecycler()->fetch(this))
    {
        // Create a new event if failed to get from recycler
        mHandle                      = new AtomicRefCounted<EventAndLayout>;
        VkEventCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        // Use device only for performance reasons.
        createInfo.flags = context->getFeatures().supportsSynchronization2.enabled
                               ? VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR
                               : 0;
        mHandle->get().event.init(context->getDevice(), createInfo);
    }

    mHandle->addRef();
    mHandle->get().imageLayout = layout;
}

void RefCountedEvent::release(Context *context)
{
    if (mHandle != nullptr)
    {
        const bool isLastReference = mHandle->getAndReleaseRef() == 1;
        if (isLastReference)
        {
            context->getRefCountedEventRecycler()->recycle(std::move(*this));
            ASSERT(mHandle == nullptr);
        }
        else
        {
            mHandle = nullptr;
        }
    }
}

void RefCountedEvent::release(Renderer *renderer)
{
    if (mHandle != nullptr)
    {
        const bool isLastReference = mHandle->getAndReleaseRef() == 1;
        if (isLastReference)
        {
            renderer->getRefCountedEventRecycler()->recycle(std::move(*this));
            ASSERT(mHandle == nullptr);
        }
        else
        {
            mHandle = nullptr;
        }
    }
}

void RefCountedEvent::destroy(VkDevice device)
{
    if (mHandle != nullptr)
    {
        const bool isLastReference = mHandle->getAndReleaseRef() == 1;
        ASSERT(isLastReference);
        ASSERT(mHandle != nullptr);
        ASSERT(!mHandle->isReferenced());
        mHandle->get().event.destroy(device);
        SafeDelete(mHandle);
    }
}

// RefCountedEventsGarbage implementation.
void RefCountedEventsGarbage::destroy(Renderer *renderer)
{
    ASSERT(renderer->hasResourceUseFinished(mLifetime));
    for (RefCountedEvent &event : mRefCountedEvents)
    {
        ASSERT(event.valid());
        event.destroy(renderer->getDevice());
    }
    mRefCountedEvents.clear();
}

bool RefCountedEventsGarbage::recycleIfComplete(Context *context)
{
    if (!context->getRenderer()->hasResourceUseFinished(mLifetime))
    {
        return false;
    }

    for (RefCountedEvent &event : mRefCountedEvents)
    {
        ASSERT(event.valid());
        event.release(context);
        ASSERT(!event.valid());
    }
    mRefCountedEvents.clear();
    return true;
}

// RefCountedEventRecycler implementation.
RefCountedEventRecycler::~RefCountedEventRecycler()
{
    ASSERT(mFreeStack.empty());
    ASSERT(mGarbageQueue.empty());
}

void RefCountedEventRecycler::destroy(Renderer *renderer)
{
    while (!mGarbageQueue.empty())
    {
        mGarbageQueue.front().destroy(renderer);
        mGarbageQueue.pop();
    }

    mFreeStack.destroy(renderer->getDevice());
}

void RefCountedEventRecycler::cleanup(Context *context)
{
    while (!mGarbageQueue.empty())
    {
        bool destroyed = mGarbageQueue.front().recycleIfComplete(context);
        if (destroyed)
        {
            mGarbageQueue.pop();
        }
        else
        {
            break;
        }
    }
}

bool RefCountedEventRecycler::fetch(Context *context, RefCountedEvent *outObject)
{
    if (!mFreeStack.empty())
    {
        mFreeStack.fetch(outObject);
        return true;
    }
    else
    {
        cleanup(context);
        if (!mFreeStack.empty())
        {
            mFreeStack.fetch(outObject);
            return true;
        }
    }
    return false;
}

// EventBarrier implementation.
bool EventBarrier::hasEvent(const VkEvent &event) const
{
    for (const VkEvent &existingEvent : mEvents)
    {
        if (existingEvent == event)
        {
            return true;
        }
    }
    return false;
}

void EventBarrier::addDiagnosticsString(std::ostringstream &out) const
{
    if (mMemoryBarrierSrcAccess != 0 || mMemoryBarrierDstAccess != 0)
    {
        out << "Src: 0x" << std::hex << mMemoryBarrierSrcAccess << " &rarr; Dst: 0x" << std::hex
            << mMemoryBarrierDstAccess << std::endl;
    }
}

void EventBarrier::execute(PrimaryCommandBuffer *primary)
{
    if (isEmpty())
    {
        return;
    }

    // Issue vkCmdWaitEvents call
    VkMemoryBarrier memoryBarrier = {};
    uint32_t memoryBarrierCount   = 0;
    if (mMemoryBarrierDstAccess != 0)
    {
        memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = mMemoryBarrierSrcAccess;
        memoryBarrier.dstAccessMask = mMemoryBarrierDstAccess;
        memoryBarrierCount++;
    }

    primary->waitEvents(static_cast<uint32_t>(mEvents.size()), mEvents.data(), mSrcStageMask,
                        mDstStageMask, memoryBarrierCount, &memoryBarrier, 0, nullptr,
                        static_cast<uint32_t>(mImageMemoryBarriers.size()),
                        mImageMemoryBarriers.data());

    reset();
}

// EventBarrierArray implementation.
void EventBarrierArray::addMemoryEvent(Context *context,
                                       const RefCountedEvent &waitEvent,
                                       VkPipelineStageFlags dstStageMask,
                                       VkAccessFlags dstAccess)
{
    ASSERT(waitEvent.valid());

    WARN() << " addMemoryEvent:" << waitEvent.getEvent().getHandle() << " dstStageMask:0x"
           << std::hex << dstStageMask << " dstAccess:0x" << dstAccess;

    for (EventBarrier &barrier : mBarriers)
    {
        // If the event is already in the waiting list, just add the new stageMask to the
        // dstStageMask. Otherwise we will end up with two waitEvent calls that wait for the same
        // VkEvent but for different dstStage and confuses VVL.
        if (barrier.hasEvent(waitEvent.getEvent().getHandle()))
        {
            barrier.addAdditionalStageAccess(dstStageMask, dstAccess);
            return;
        }
    }

    VkAccessFlags accessMask;
    VkPipelineStageFlags stageFlags = GetRefCountedEventStageMask(context, waitEvent, &accessMask);
    // Since this is used with WAW without layout change, dstStageMask should be the same as event's
    // stageMask. Otherwise you should get into addImageEvent.
    ASSERT(stageFlags == dstStageMask && accessMask == dstAccess);
    mBarriers.emplace_back(stageFlags, dstStageMask, accessMask, dstAccess,
                           waitEvent.getEvent().getHandle());
}

void EventBarrierArray::addImageEvent(Context *context,
                                      const RefCountedEvent &waitEvent,
                                      VkPipelineStageFlags dstStageMask,
                                      const VkImageMemoryBarrier &imageMemoryBarrier)
{
    ASSERT(waitEvent.valid());
    VkPipelineStageFlags srcStageFlags = GetRefCountedEventStageMask(context, waitEvent);

    WARN() << " addImageEvent:" << waitEvent.getEvent().getHandle()
           << " image:" << imageMemoryBarrier.image << " srcStageMask:0x" << std::hex
           << srcStageFlags << " dstStageMask:0x" << dstStageMask;

    mBarriers.emplace_back();
    EventBarrier &barrier = mBarriers.back();
    // VkCmdWaitEvent must uses the same stageMask as VkCmdSetEvent due to
    // VUID-vkCmdWaitEvents-srcStageMask-01158 requirements.
    barrier.mSrcStageMask = srcStageFlags;
    // If there is an event, we use the waitEvent to do layout change.
    barrier.mEvents.emplace_back(waitEvent.getEvent().getHandle());
    barrier.mDstStageMask = dstStageMask;
    barrier.mImageMemoryBarriers.emplace_back(imageMemoryBarrier);
}

void EventBarrierArray::execute(Renderer *renderer, PrimaryCommandBuffer *primary)
{
    if (mBarriers.empty())
    {
        return;
    }

    for (EventBarrier &barrier : mBarriers)
    {
        barrier.execute(primary);
    }
    mBarriers.clear();
}

void EventBarrierArray::addDiagnosticsString(std::ostringstream &out) const
{
    out << "Event Barrier: ";
    for (const EventBarrier &barrier : mBarriers)
    {
        barrier.addDiagnosticsString(out);
    }
    out << "\\l";
}
}  // namespace vk
}  // namespace rx
