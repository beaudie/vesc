//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#include "libANGLE/renderer/vulkan/RefCountedEvent.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
namespace vk
{

void ReleaseRefcountedEvent(VkDevice device,
                            RefCountedEventAndStageMaskHandle atomicRefCountedEvent)
{
    atomicRefCountedEvent->releaseRef();
    if (!atomicRefCountedEvent->isReferenced())
    {
        atomicRefCountedEvent->get().event.destroy(device);
        SafeDelete(atomicRefCountedEvent);
    }
}

void RefCountedEvent::init(VkDevice device, ImageLayout layout)
{
    ASSERT(mHandle == nullptr);
    ASSERT(layout != ImageLayout::Undefined);

    mHandle                      = new AtomicRefCounted<EventAndStageMask>;
    VkEventCreateInfo createInfo = {};
    createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    createInfo.flags             = 0;
    mHandle->get().event.init(device, createInfo);
    mHandle->addRef();
    mHandle->get().imageLayout = layout;
}

// RefCountedEventCollector implementation
void RefCountedEventCollector::collect(ImageHelper *image)
{
    if (image->getWriteEvent().valid())
    {
        mGarbageObjects.emplace_back(GetGarbage(&image->getWriteEvent()));
    }

    std::vector<RefCountedEvent> &events = image->getReadOnlyEvents();
    while (!events.empty())
    {
        mGarbageObjects.emplace_back(GetGarbage(&events.back()));
        events.pop_back();
    }
}

void RefCountedEventCollector::collect(RefCountedEvent *event)
{
    mGarbageObjects.emplace_back(GetGarbage(event));
}

void RefCountedEventCollector::collect(std::vector<RefCountedEvent> *events)
{
    while (!events->empty())
    {
        mGarbageObjects.emplace_back(GetGarbage(&events->back()));
        events->pop_back();
    }
}

void RefCountedEventCollector::release(RendererVk *renderer, const QueueSerial &queueSerial)
{
    if (!mGarbageObjects.empty())
    {
        ResourceUse use(queueSerial);
        renderer->collectGarbage(use, std::move(mGarbageObjects));
    }
}

// EventBarrier implementation.
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
void EventBarrierArray::addImageEvent(Context *context,
                                      const RefCountedEvent &writeEvent,
                                      const std::vector<RefCountedEvent> &readOnlyEvents,
                                      VkPipelineStageFlags dstStageMask,
                                      const VkImageMemoryBarrier &imageMemoryBarrier)
{
    mBarriers.emplace_back();

    EventBarrier &barrier = mBarriers.back();
    if (writeEvent.valid())
    {
        barrier.mSrcStageMask |= GetImageLayoutDstStageMask(context, writeEvent);
        // If there is an event, we use the waitEvent to do layout change.
        barrier.mEvents.emplace_back(writeEvent.getEvent().getHandle());
    }
    for (const RefCountedEvent &event : readOnlyEvents)
    {
        ASSERT(event.valid());
        barrier.mSrcStageMask |= GetImageLayoutDstStageMask(context, event);
        barrier.mEvents.emplace_back(event.getEvent().getHandle());
    }
    barrier.mDstStageMask |= dstStageMask;
    barrier.mImageMemoryBarriers.emplace_back(imageMemoryBarrier);
}

void EventBarrierArray::execute(RendererVk *renderer, PrimaryCommandBuffer *primary)
{
    // make a local copy for faster access
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
