//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#ifndef LIBANGLE_RENDERER_VULKAN_REFCOUNTED_EVENT_H_
#define LIBANGLE_RENDERER_VULKAN_REFCOUNTED_EVENT_H_

#include <atomic>
#include <limits>
#include <queue>

#include "common/PackedEnums.h"
#include "common/debug.h"
#include "libANGLE/renderer/serial_utils.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{
namespace vk
{
enum class ImageLayout;
class ImageHelper;

enum class BarrierType
{
    Pipeline,
    Event,
};

struct EventAndLayout
{
    bool valid() const { return event.valid(); }
    Event event;
    ImageLayout imageLayout;
};
using RefCountedEventAndLayoutHandle = AtomicRefCounted<EventAndLayout> *;

void ReleaseRefcountedEvent(VkDevice device, RefCountedEventAndLayoutHandle atomicRefCountedEvent);

// Wrapper for reference counted event and image layout struct.
class RefCountedEvent final : public WrappedObject<RefCountedEvent, RefCountedEventAndLayoutHandle>
{
  public:
    RefCountedEvent() = default;

    // Move constructor moves reference of the underline object from other to this.
    RefCountedEvent(RefCountedEvent &&other)
    {
        mHandle       = other.mHandle;
        other.mHandle = nullptr;
    }

    // Copy constructor adds reference to the underline object.
    RefCountedEvent(const RefCountedEvent &other)
    {
        mHandle = other.mHandle;
        if (mHandle != nullptr)
        {
            mHandle->addRef();
        }
    }

    // Move assignment moves reference of the underline object from other to this.
    RefCountedEvent &operator=(RefCountedEvent &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    // Copy assignment adds reference to the underline object.
    RefCountedEvent &operator=(const RefCountedEvent &other)
    {
        ASSERT(!valid());
        ASSERT(other.valid());
        mHandle = other.mHandle;
        mHandle->addRef();
        return *this;
    }

    // Returns true if both points to the same underline object.
    bool operator==(const RefCountedEvent &other) const { return mHandle == other.mHandle; }

    void init(VkDevice device, ImageLayout layout);

    void release(VkDevice device)
    {
        if (!valid())
        {
            return;
        }
        ReleaseRefcountedEvent(device, mHandle);
        mHandle = nullptr;
    }

    bool valid() const { return mHandle != nullptr; }

    const Event &getEvent() const
    {
        ASSERT(valid());
        return mHandle->get().event;
    }

    ImageLayout getImageLayout() const
    {
        ASSERT(valid());
        return mHandle->get().imageLayout;
    }
};

template <>
struct HandleTypeHelper<RefCountedEvent>
{
    constexpr static HandleType kHandleType = HandleType::RefCountedEvent;
};

// This tracks a vector of RefcountedEvent garbage.
class RefCountedEventGarbageObjects final
{
  public:
    // Move event to the garbages
    void add(RefCountedEvent *event);
    // Make a copy of event (which adds another refcount to the VkEvent) and add the copied event to
    // the garbages
    void addDuplicate(const RefCountedEvent &event);
    // Move the vector of events to the garbages
    void add(std::vector<RefCountedEvent> *events);

    bool empty() const { return mGarbageObjects.empty(); }

    GarbageObjects &&release() { return std::move(mGarbageObjects); }

  private:
    GarbageObjects mGarbageObjects;
};

// This wraps data and API for vkCmdWaitEvent call
class EventBarrier : angle::NonCopyable
{
  public:
    EventBarrier()
        : mSrcStageMask(0), mDstStageMask(0), mMemoryBarrierSrcAccess(0), mMemoryBarrierDstAccess(0)
    {}

    EventBarrier(VkPipelineStageFlags srcStageMask,
                 VkPipelineStageFlags dstStageMask,
                 VkAccessFlags srcAccess,
                 VkAccessFlags dstAccess,
                 const VkEvent &event)
        : mSrcStageMask(srcStageMask),
          mDstStageMask(dstStageMask),
          mMemoryBarrierSrcAccess(srcAccess),
          mMemoryBarrierDstAccess(dstAccess)
    {
        mEvents.push_back(event);
    }

    EventBarrier(VkPipelineStageFlags srcStageMask,
                 VkPipelineStageFlags dstStageMask,
                 const VkEvent &event,
                 const VkImageMemoryBarrier &imageMemoryBarrier)
        : mSrcStageMask(srcStageMask),
          mDstStageMask(dstStageMask),
          mMemoryBarrierSrcAccess(0),
          mMemoryBarrierDstAccess(0)
    {
        ASSERT(event != VK_NULL_HANDLE);
        ASSERT(imageMemoryBarrier.pNext == nullptr);
        mEvents.push_back(event);
        mImageMemoryBarriers.push_back(imageMemoryBarrier);
    }

    EventBarrier(EventBarrier &&other)
    {
        mSrcStageMask           = other.mSrcStageMask;
        mDstStageMask           = other.mDstStageMask;
        mMemoryBarrierSrcAccess = other.mMemoryBarrierSrcAccess;
        mMemoryBarrierDstAccess = other.mMemoryBarrierDstAccess;
        std::swap(mEvents, other.mEvents);
        std::swap(mImageMemoryBarriers, other.mImageMemoryBarriers);
        other.mSrcStageMask           = 0;
        other.mDstStageMask           = 0;
        other.mMemoryBarrierSrcAccess = 0;
        other.mMemoryBarrierDstAccess = 0;
    }

    ~EventBarrier()
    {
        ASSERT(mImageMemoryBarriers.empty());
        ASSERT(mEvents.empty());
    }

    bool isEmpty() const
    {
        return mEvents.empty() && mImageMemoryBarriers.empty() && mMemoryBarrierDstAccess == 0;
    }

    bool hasEvent(const VkEvent &event) const;

    void addAdditionalStageAccess(VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccess)
    {
        mDstStageMask |= dstStageMask;
        mMemoryBarrierDstAccess |= dstAccess;
    }

    void execute(PrimaryCommandBuffer *primary);

    void reset()
    {
        mEvents.clear();
        mImageMemoryBarriers.clear();
    }

    void addDiagnosticsString(std::ostringstream &out) const;

  private:
    friend class EventBarrierArray;
    VkPipelineStageFlags mSrcStageMask;
    VkPipelineStageFlags mDstStageMask;
    VkAccessFlags mMemoryBarrierSrcAccess;
    VkAccessFlags mMemoryBarrierDstAccess;
    std::vector<VkEvent> mEvents;
    std::vector<VkImageMemoryBarrier> mImageMemoryBarriers;
};

class EventBarrierArray final
{
  public:
    bool isEmpty() const { return mBarriers.empty(); }

    void execute(Renderer *renderer, PrimaryCommandBuffer *primary);

    void addMemoryEvent(Context *context,
                        const RefCountedEvent &waitEvent,
                        VkPipelineStageFlags dstStageMask,
                        VkAccessFlags dstAccess);

    void addImageEvent(Context *context,
                       const RefCountedEvent &waitEvent,
                       VkPipelineStageFlags dstStageMask,
                       const VkImageMemoryBarrier &imageMemoryBarrier);

    void reset() { mBarriers.clear(); }

    void addDiagnosticsString(std::ostringstream &out) const;

  private:
    std::vector<EventBarrier> mBarriers;
};

VkPipelineStageFlags GetRefCountedEventStageMask(Context *context,
                                                 const RefCountedEvent &event,
                                                 VkAccessFlags *accessMask);

}  // namespace vk
}  // namespace rx
#endif  // LIBANGLE_RENDERER_VULKAN_REFCOUNTED_EVENT_H_
