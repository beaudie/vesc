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
// reference counted event
struct EventAndStageMask
{
    bool valid() const { return event.valid(); }
    Event event;
    VkPipelineStageFlags stageMask;
};
using RefCountedEventAndStageMaskHandle = AtomicRefCounted<EventAndStageMask> *;

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

    void init(VkDevice device)
    {
        ASSERT(mHandle == nullptr);
        mHandle                      = new AtomicRefCounted<EventAndStageMask>;
        VkEventCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        createInfo.flags             = 0;
        mHandle->get().event.init(device, createInfo);
        mHandle->addRef();
        mHandle->get().stageMask = 0;
    }

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

    void addRef(const RefCountedEvent &other)
    {
        ASSERT(!valid());
        ASSERT(other.valid());
        mHandle = other.mHandle;
        mHandle->addRef();
    }

    bool valid() const { return mHandle != nullptr; }
    const Event &getEvent() const { return mHandle->get().event; }
    VkPipelineStageFlags getStageMask() const { return mHandle->get().stageMask; }
    void addStageMask(VkPipelineStageFlags flags) { mHandle->get().stageMask |= flags; }
};
template <>
struct HandleTypeHelper<RefCountedEvent>
{
    constexpr static HandleType kHandleType = HandleType::RefCountedEvent;
};

void ReleaseRefcountedEvent(VkDevice device,
                            RefCountedEventAndStageMaskHandle atomicRefCountedEvent);
}  // namespace vk
}  // namespace rx
#endif  // LIBANGLE_RENDERER_VULKAN_REFCOUNTED_EVENT_H_
