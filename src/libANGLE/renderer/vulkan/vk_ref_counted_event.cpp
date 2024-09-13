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
namespace
{
// Predefined VkPipelineStageFlags for RefCountedEvent
constexpr angle::PackedEnumMap<EventStage, VkPipelineStageFlags2>
    kEventStageAndPipelineStageFlagsMap = {
        {EventStage::Transfer, VK_PIPELINE_STAGE_2_TRANSFER_BIT},
        {EventStage::VertexShader, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT},
        {EventStage::FragmentShader, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT},
        {EventStage::ComputeShader, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
        {EventStage::AllShaders, kAllShadersPipelineStageFlags},
        {EventStage::PreFragmentShaders, kPreFragmentStageFlags},
        {EventStage::FragmentShadingRate,
         VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR},
        {EventStage::ColorAttachmentOutput, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT},
        {EventStage::ColorAttachmentOutputAndFragmentShader,
         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT},
        {EventStage::ColorAttachmentOutputAndFragmentShaderAndTransfer,
         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT |
             VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT},
        {EventStage::ColorAttachmentOutputAndAllShaders,
         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | kAllShadersPipelineStageFlags},
        {EventStage::AllFragmentTest, kAllDepthStencilPipelineStageFlags},
        {EventStage::AllFragmentTestAndFragmentShader,
         VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags},
        {EventStage::AllFragmentTestAndAllShaders,
         kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags},
        {EventStage::TransferAndComputeShader,
         VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT}};

void DestroyRefCountedEvents(VkDevice device, RefCountedEventCollector &events)
{
    while (!events.empty())
    {
        events.back().destroy(device);
        events.pop_back();
    }
}
}  // namespace

void InitializeEventAndPipelineStagesMap(
    angle::PackedEnumMap<EventStage, VkPipelineStageFlags2> *map,
    VkPipelineStageFlags2 supportedVulkanPipelineStageMask)
{
    *map = kEventStageAndPipelineStageFlagsMap;
    for (VkPipelineStageFlags2 &flag : *map)
    {
        flag &= supportedVulkanPipelineStageMask;
    }
}

bool RefCountedEvent::init(Context *context, EventStage eventStage)
{
    ASSERT(mHandle == nullptr);
    ASSERT(eventStage != EventStage::InvalidEnum);

    // First try with recycler. We must issue VkCmdResetEvent before VkCmdSetEvent
    if (context->getRefCountedEventsGarbageRecycler()->fetch(context->getRenderer(), this))
    {
        ASSERT(valid());
        ASSERT(!mHandle->isReferenced());
    }
    else
    {
        // If failed to fetch from recycler, then create a new event.
        mHandle                      = new RefCounted<EventAndStage>;
        VkEventCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        // Use device only for performance reasons.
        createInfo.flags = context->getFeatures().supportsSynchronization2.enabled
                               ? VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR
                               : 0;
        VkResult result  = mHandle->get().event.init(context->getDevice(), createInfo);
        if (result != VK_SUCCESS)
        {
            WARN() << "event.init failed. Clean up garbage and retry again";
            // Proactively clean up garbage and retry
            context->getRefCountedEventsGarbageRecycler()->cleanup(context->getRenderer());
            result = mHandle->get().event.init(context->getDevice(), createInfo);
            if (result != VK_SUCCESS)
            {
                // Drivers usually can allocate huge amount of VkEvents, and we should never use
                // that many VkEvents under normal situation. If we failed to allocate, there is a
                // high chance that we may have a leak somewhere. This macro should help us catch
                // such potential bugs in the bots if that happens.
                UNREACHABLE();
                // If still fail to create, we just return. An invalid event will trigger
                // pipelineBarrier code path
                return false;
            }
        }
    }

    mHandle->addRef();
    mHandle->get().eventStage = eventStage;
    return true;
}

void RefCountedEvent::release(Context *context)
{
    if (mHandle != nullptr)
    {
        releaseImpl(context->getRenderer(), context->getRefCountedEventsGarbageRecycler());
    }
}

void RefCountedEvent::release(Renderer *renderer)
{
    if (mHandle != nullptr)
    {
        releaseImpl(renderer, renderer->getRefCountedEventRecycler());
    }
}

template <typename RecyclerT>
void RefCountedEvent::releaseImpl(Renderer *renderer, RecyclerT *recycler)
{
    ASSERT(mHandle != nullptr);
    // This should never called from async submission thread since the refcount is not atomic. It is
    // expected only called under context share lock.
    ASSERT(std::this_thread::get_id() != renderer->getCommandProcessorThreadId());

    const bool isLastReference = mHandle->getAndReleaseRef() == 1;
    if (isLastReference)
    {
        // When async submission is enabled, recycler will be null when release call comes from
        // CommandProcessor. But in that case it will not be the last reference since garbage
        // collector should have one reference count and will never release that reference count
        // until GPU finished.
        ASSERT(recycler != nullptr);
        recycler->recycle(std::move(*this));
        ASSERT(mHandle == nullptr);
    }
    else
    {
        mHandle = nullptr;
    }
}

void RefCountedEvent::destroy(VkDevice device)
{
    ASSERT(mHandle != nullptr);
    ASSERT(!mHandle->isReferenced());
    mHandle->get().event.destroy(device);
    SafeDelete(mHandle);
}

// RefCountedEventsGarbage implementation.
void RefCountedEventsGarbage::destroy(Renderer *renderer)
{
    ASSERT(renderer->hasQueueSerialFinished(mQueueSerial));
    while (!mRefCountedEvents.empty())
    {
        ASSERT(mRefCountedEvents.back().valid());
        mRefCountedEvents.back().release(renderer);
        mRefCountedEvents.pop_back();
    }
}

bool RefCountedEventsGarbage::releaseIfComplete(Renderer *renderer,
                                                RefCountedEventsGarbageRecycler *recycler)
{
    if (!renderer->hasQueueSerialFinished(mQueueSerial))
    {
        return false;
    }

    while (!mRefCountedEvents.empty())
    {
        ASSERT(mRefCountedEvents.back().valid());
        mRefCountedEvents.back().releaseImpl(renderer, recycler);
        ASSERT(!mRefCountedEvents.back().valid());
        mRefCountedEvents.pop_back();
    }
    return true;
}

bool RefCountedEventsGarbage::moveIfComplete(Renderer *renderer,
                                             std::deque<RefCountedEventCollector> *releasedBucket)
{
    if (!renderer->hasQueueSerialFinished(mQueueSerial))
    {
        return false;
    }

    releasedBucket->emplace_back(std::move(mRefCountedEvents));
    return true;
}

// RefCountedEventRecycler implementation.
void RefCountedEventRecycler::destroy(VkDevice device)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    while (!mEventsToReset.empty())
    {
        DestroyRefCountedEvents(device, mEventsToReset.back());
        mEventsToReset.pop_back();
    }

    ASSERT(mResettingQueue.empty());

    while (!mEventsToReuse.empty())
    {
        DestroyRefCountedEvents(device, mEventsToReuse.back());
        mEventsToReuse.pop_back();
    }
}

void RefCountedEventRecycler::resetEvents(Context *context,
                                          const QueueSerial queueSerial,
                                          PrimaryCommandBuffer *commandbuffer)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    if (mEventsToReset.empty())
    {
        return;
    }

    Renderer *renderer = context->getRenderer();
    while (!mEventsToReset.empty())
    {
        RefCountedEventCollector &events = mEventsToReset.back();
        ASSERT(!events.empty());
        for (const RefCountedEvent &refCountedEvent : events)
        {
            // TODO: use resetEvent2 when VK_KHR_Synchronization2 is supported
            VkPipelineStageFlags stageMask = static_cast<VkPipelineStageFlags>(
                renderer->getEventPipelineStageMask(refCountedEvent));
            commandbuffer->resetEvent(refCountedEvent.getEvent().getHandle(), stageMask);
        }
        mResettingQueue.emplace(queueSerial, std::move(events));
        mEventsToReset.pop_back();
    }
}

void RefCountedEventRecycler::cleanupResettingEvents(Renderer *renderer)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    while (!mResettingQueue.empty())
    {
        bool released = mResettingQueue.front().moveIfComplete(renderer, &mEventsToReuse);
        if (released)
        {
            mResettingQueue.pop();
        }
        else
        {
            break;
        }
    }
}

bool RefCountedEventRecycler::fetchEventsToReuse(RefCountedEventCollector *eventsToReuseOut)
{
    ASSERT(eventsToReuseOut != nullptr);
    ASSERT(eventsToReuseOut->empty());
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    if (mEventsToReuse.empty())
    {
        return false;
    }
    eventsToReuseOut->swap(mEventsToReuse.back());
    mEventsToReuse.pop_back();
    return true;
}

// RefCountedEventsGarbageRecycler implementation.
RefCountedEventsGarbageRecycler::~RefCountedEventsGarbageRecycler()
{
    ASSERT(mEventsToReset.empty());
    ASSERT(mGarbageQueue.empty());
    ASSERT(mEventsToReuse.empty());
    ASSERT(mGarbageCount == 0);
}

void RefCountedEventsGarbageRecycler::destroy(Renderer *renderer)
{
    VkDevice device = renderer->getDevice();
    DestroyRefCountedEvents(device, mEventsToReset);
    ASSERT(mGarbageQueue.empty());
    ASSERT(mGarbageCount == 0);
    mEventsToReuse.destroy(device);
}

void RefCountedEventsGarbageRecycler::cleanup(Renderer *renderer)
{
    // First cleanup already completed events and add to mEventsToReset
    while (!mGarbageQueue.empty())
    {
        size_t count  = mGarbageQueue.front().size();
        bool released = mGarbageQueue.front().releaseIfComplete(renderer, this);
        if (released)
        {
            mGarbageCount -= count;
            mGarbageQueue.pop();
        }
        else
        {
            break;
        }
    }

    // Move mEventsToReset to the renderer so that it can be reset.
    if (!mEventsToReset.empty())
    {
        renderer->getRefCountedEventRecycler()->recycle(std::move(mEventsToReset));
    }
}

bool RefCountedEventsGarbageRecycler::fetch(Renderer *renderer, RefCountedEvent *outObject)
{
    if (mEventsToReuse.empty())
    {
        // Retrieve a list of ready to reuse events from renderer.
        RefCountedEventCollector events;
        if (!renderer->getRefCountedEventRecycler()->fetchEventsToReuse(&events))
        {
            return false;
        }
        mEventsToReuse.refill(std::move(events));
        ASSERT(!mEventsToReuse.empty());
    }
    mEventsToReuse.fetch(outObject);
    return true;
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

void EventBarrier::execute(const bool supportSynchronization2, PrimaryCommandBuffer *primary)
{
    if (isEmpty())
    {
        return;
    }
    ASSERT(mEvent != VK_NULL_HANDLE);
    ASSERT(mImageMemoryBarrierCount == 0 ||
           (mImageMemoryBarrierCount == 1 && mImageMemoryBarrier.image != VK_NULL_HANDLE));

    // Fallback to vkCmdWaitEvents
    // TODO: add usage of vkCmdWaitEvents2 when VK_KHR_Synchronization2 is supported
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = static_cast<VkAccessFlags>(mMemoryBarrierSrcAccess);
    memoryBarrier.dstAccessMask   = static_cast<VkAccessFlags>(mMemoryBarrierDstAccess);

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext                = nullptr;
    imageMemoryBarrier.srcAccessMask =
        static_cast<VkAccessFlags>(mImageMemoryBarrier.srcAccessMask);
    imageMemoryBarrier.dstAccessMask =
        static_cast<VkAccessFlags>(mImageMemoryBarrier.dstAccessMask);
    imageMemoryBarrier.oldLayout           = mImageMemoryBarrier.oldLayout;
    imageMemoryBarrier.newLayout           = mImageMemoryBarrier.newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = mImageMemoryBarrier.srcQueueFamilyIndex;
    imageMemoryBarrier.dstQueueFamilyIndex = mImageMemoryBarrier.dstQueueFamilyIndex;
    imageMemoryBarrier.image               = mImageMemoryBarrier.image;
    imageMemoryBarrier.subresourceRange    = mImageMemoryBarrier.subresourceRange;
    primary->waitEvents(1, &mEvent, static_cast<VkPipelineStageFlags>(mSrcStageMask),
                        static_cast<VkPipelineStageFlags>(mDstStageMask), 1, &memoryBarrier, 0,
                        nullptr, mImageMemoryBarrierCount,
                        mImageMemoryBarrierCount == 0 ? nullptr : &imageMemoryBarrier);
}

// EventBarrierArray implementation.
void EventBarrierArray::addAdditionalStageAccess(const RefCountedEvent &waitEvent,
                                                 VkPipelineStageFlags2 dstStageMask,
                                                 VkAccessFlags2 dstAccess)
{
    for (EventBarrier &barrier : mBarriers)
    {
        if (barrier.hasEvent(waitEvent.getEvent().getHandle()))
        {
            barrier.addAdditionalStageAccess(dstStageMask, dstAccess);
            return;
        }
    }
    UNREACHABLE();
}

void EventBarrierArray::addMemoryEvent(Renderer *renderer,
                                       const RefCountedEvent &waitEvent,
                                       VkPipelineStageFlags2 dstStageMask,
                                       VkAccessFlags2 dstAccess)
{
    ASSERT(waitEvent.valid());
    VkPipelineStageFlags2 stageFlags = renderer->getEventPipelineStageMask(waitEvent);
    // This should come down as WAW without layout change, dstStageMask should be the same as
    // event's stageMask. Otherwise you should get into addImageEvent.
    ASSERT(stageFlags == dstStageMask);
    mBarriers.emplace_back(stageFlags, dstStageMask, dstAccess, dstAccess,
                           waitEvent.getEvent().getHandle());
}

void EventBarrierArray::addImageEvent(Renderer *renderer,
                                      const RefCountedEvent &waitEvent,
                                      const VkImageMemoryBarrier2 &imageMemoryBarrier2)
{
    ASSERT(waitEvent.valid());
    mBarriers.emplace_back(waitEvent.getEvent().getHandle(), imageMemoryBarrier2);
}

void EventBarrierArray::execute(Renderer *renderer, PrimaryCommandBuffer *primary)
{
    while (!mBarriers.empty())
    {
        mBarriers.back().execute(renderer->getFeatures().supportsSynchronization2.enabled, primary);
        mBarriers.pop_back();
    }
    reset();
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
