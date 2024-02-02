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

// reference counted event
struct EventAndStageMask
{
    bool valid() const { return event.valid(); }
    Event event;
    ImageLayout imageLayout;
};
using RefCountedEventAndStageMaskHandle = AtomicRefCounted<EventAndStageMask> *;
using EventCollector                    = std::vector<Event>;

class RefCountedEvent final
    : public WrappedObject<RefCountedEvent, RefCountedEventAndStageMaskHandle>
{
  public:
    RefCountedEvent() = default;
    RefCountedEvent(RefCountedEvent &&other)
    {
        mHandle       = other.mHandle;
        other.mHandle = nullptr;
    }

    RefCountedEvent(const RefCountedEvent &other)
    {
        mHandle = other.mHandle;
        if (mHandle != nullptr)
        {
            mHandle->addRef();
        }
    }

    RefCountedEvent &operator=(RefCountedEvent &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    RefCountedEvent &operator=(const RefCountedEvent &other)
    {
        addRef(other);
        return *this;
    }

    bool operator==(const RefCountedEvent &other) const { return mHandle == other.mHandle; }

    void init(VkDevice device, ImageLayout layout);

    void destroy(VkDevice device)
    {
        // Since the underline object is refcounted, we never actually destroy it, but always
        // decrement the refcount and destroy only when refcount goes down to 0.
        mHandle->releaseRef();
        if (!mHandle->isReferenced())
        {
            mHandle->get().event.destroy(device);
            SafeDelete(mHandle);
        }
        else
        {
            mHandle = nullptr;
        }
    }

    void release(EventCollector &collector)
    {
        // Since the underline object is refcounted, we never actually destroy it, but always
        // decrement the refcount and destroy only when refcount goes down to 0.
        mHandle->releaseRef();
        if (!mHandle->isReferenced())
        {
            collector.emplace_back(std::move(mHandle->get().event));
            SafeDelete(mHandle);
        }
        else
        {
            mHandle = nullptr;
        }
    }

    void addRef(const RefCountedEvent &other)
    {
        ASSERT(!valid());
        ASSERT(other.valid());
        mHandle = other.mHandle;
        mHandle->addRef();
    }

    bool valid() const { return mHandle != nullptr && getEvent().valid(); }
    const Event &getEvent() const { return mHandle->get().event; }
    ImageLayout getImageLayout() const { return mHandle->get().imageLayout; }
};

template <>
struct HandleTypeHelper<RefCountedEvent>
{
    constexpr static HandleType kHandleType = HandleType::RefCountedEvent;
};

void ReleaseRefcountedEvent(VkDevice device,
                            RefCountedEventAndStageMaskHandle atomicRefCountedEvent);

class RefCountedEventCollector final
{
  public:
    void collect(ImageHelper *image);
    void collect(RefCountedEvent *event);
    void collect(std::vector<RefCountedEvent> *events);
    void release(RendererVk *renderer, const QueueSerial &queueSerial);

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

    void execute(RendererVk *renderer, PrimaryCommandBuffer *primary);

    void addMemoryEvent(VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags dstStageMask,
                        VkAccessFlags srcAccess,
                        VkAccessFlags dstAccess,
                        const RefCountedEvent &event)
    {
        ASSERT(event.valid());
        mBarriers.emplace_back(srcStageMask, dstStageMask, srcAccess, dstAccess,
                               event.getEvent().getHandle());
    }

    void addImageEvent(Context *context,
                       const RefCountedEvent &writeEvent,
                       const std::vector<RefCountedEvent> &readOnlyEvents,
                       VkPipelineStageFlags dstStageMask,
                       const VkImageMemoryBarrier &imageMemoryBarrier);

    void reset() { mBarriers.clear(); }

    void addDiagnosticsString(std::ostringstream &out) const;

  private:
    std::vector<EventBarrier> mBarriers;
};

VkPipelineStageFlags GetImageLayoutDstStageMask(Context *context, const RefCountedEvent &event);

}  // namespace vk
}  // namespace rx
#endif  // LIBANGLE_RENDERER_VULKAN_REFCOUNTED_EVENT_H_
