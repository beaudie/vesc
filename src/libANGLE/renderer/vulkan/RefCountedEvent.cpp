//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#include "libANGLE/renderer/vulkan/RefCountedEvent.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace vk
{

void ReleaseRefcountedEvent(VkDevice device, RefCountedEventAndLayoutHandle atomicRefCountedEvent)
{
    const bool isLastReference = atomicRefCountedEvent->getAndReleaseRef() == 1;
    if (isLastReference)
    {
        atomicRefCountedEvent->get().event.destroy(device);
        SafeDelete(atomicRefCountedEvent);
    }
}

void RefCountedEvent::init(VkDevice device, ImageLayout layout)
{
    ASSERT(mHandle == nullptr);
    ASSERT(layout != ImageLayout::Undefined);

    mHandle                      = new AtomicRefCounted<EventAndLayout>;
    VkEventCreateInfo createInfo = {};
    createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    createInfo.flags             = 0;
    mHandle->get().event.init(device, createInfo);
    mHandle->addRef();
    mHandle->get().imageLayout = layout;
}

// RefCountedEventGarbageObjects implementation
void RefCountedEventGarbageObjects::add(RefCountedEvent *event)
{
    mGarbageObjects.emplace_back(GetGarbage(event));
}

void RefCountedEventGarbageObjects::addDuplicate(RefCountedEvent event)
{
    mGarbageObjects.emplace_back(GetGarbage(&event));
}

void RefCountedEventGarbageObjects::add(std::vector<RefCountedEvent> *events)
{
    while (!events->empty())
    {
        mGarbageObjects.emplace_back(GetGarbage(&events->back()));
        events->pop_back();
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
                                      const RefCountedEvent &event,
                                      VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask,
                                      const VkImageMemoryBarrier &imageMemoryBarrier)
{
    ASSERT(event.valid());

    mBarriers.emplace_back();
    EventBarrier &barrier = mBarriers.back();
    barrier.mSrcStageMask |= srcStageMask;
    // If there is an event, we use the waitEvent to do layout change.
    barrier.mEvents.emplace_back(event.getEvent().getHandle());
    barrier.mDstStageMask |= dstStageMask;
    barrier.mImageMemoryBarriers.emplace_back(imageMemoryBarrier);
}

void EventBarrierArray::execute(Renderer *renderer, PrimaryCommandBuffer *primary)
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
