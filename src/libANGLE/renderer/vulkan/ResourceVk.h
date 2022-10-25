//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceVk:
//    Resource lifetime tracking in the Vulkan back-end.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
#define LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_

#include "libANGLE/HandleAllocator.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include <queue>

namespace rx
{
namespace vk
{
// Serials is just an array of serials. The array may expand if needed. Since it owned by Resource
// object which is protected by shared lock. When it passes to renderer, we will make a copy. The
// array size is expected to be small. But in future if we run into situation that array size is too
// big, we can change to packed array of QueueSerials.
static constexpr size_t kMaxFastQueueSerials = 4;
using Serials                                = angle::FastVector<Serial, kMaxFastQueueSerials>;

// Tracks how a resource is used by ANGLE and by a VkQueue. The reference count indicates the number
// of times a resource is retained by ANGLE. The serial indicates the most recent use of a resource
// in the VkQueue. The reference count and serial together can determine if a resource is currently
// in use.
class ResourceUse final
{
  public:
    ResourceUse()  = default;
    ~ResourceUse() = default;

    ResourceUse(const QueueSerial &queueSerial)
    {
        setSerial(queueSerial.index, queueSerial.serial);
    }

    // Copy constructor
    ResourceUse(const ResourceUse &other) : mSerials(other.mSerials) {}
    ResourceUse &operator=(const ResourceUse &other)
    {
        mSerials = other.mSerials;
        return *this;
    }

    // Move constructor
    ResourceUse(ResourceUse &&other) : mSerials(other.mSerials) { other.mSerials.clear(); }
    ResourceUse &operator=(ResourceUse &&other)
    {
        mSerials = other.mSerials;
        other.mSerials.clear();
        return *this;
    }

    ANGLE_INLINE bool valid() const { return mSerials.size() > 0; }

    ANGLE_INLINE void reset() { mSerials.clear(); }

    ANGLE_INLINE Serial getSerial(SerialIndex index) const
    {
        ASSERT(index < mSerials.size());
        return mSerials[index];
    }

    ANGLE_INLINE void setSerial(SerialIndex index, Serial serial)
    {
        ASSERT(index != kInvalidQueueSerialIndex);
        ASSERT(serial.valid());
        if (mSerials.size() <= index)
        {
            mSerials.resize(index + 1);
        }
        mSerials[index] = serial;
    }

    ANGLE_INLINE const Serials &getSerials() const { return mSerials; }

    ANGLE_INLINE bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        ASSERT(commandBufferQueueSerial.valid());
        return mSerials.size() > commandBufferQueueSerial.index &&
               mSerials[commandBufferQueueSerial.index] == commandBufferQueueSerial.serial;
    }

  private:
    Serials mSerials;
};

class SharedGarbage
{
  public:
    SharedGarbage();
    SharedGarbage(SharedGarbage &&other);
    SharedGarbage(const ResourceUse &use, std::vector<GarbageObject> &&garbage);
    ~SharedGarbage();
    SharedGarbage &operator=(SharedGarbage &&rhs);

    bool destroyIfComplete(RendererVk *renderer);
    bool hasUnsubmittedUse(RendererVk *renderer) const;

  private:
    ResourceUse mLifetime;
    GarbageList mGarbage;
};

using SharedGarbageList = std::queue<SharedGarbage>;

// This is a helper class for back-end objects used in Vk command buffers. They keep a record
// of their use in ANGLE and VkQueues via ResourceUse.
class Resource : angle::NonCopyable
{
  public:
    virtual ~Resource();

    // Returns true if the resource is used by ANGLE in an unflushed command buffer.
    bool usedInRecordedCommands(Context *context) const;

    // Determine if the driver has finished execution with this resource.
    bool usedInRunningCommands(RendererVk *renderer) const;

    // Returns true if the resource is in use by ANGLE or the driver.
    bool isCurrentlyInUse(RendererVk *renderer) const;

    // Ensures the driver is caught up to this resource and it is only in use by ANGLE.
    angle::Result finishRunningCommands(ContextVk *contextVk);

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk,
                              const char *debugMessage,
                              RenderPassClosureReason reason);

    // Adds queue serial to the resource.
    ANGLE_INLINE void setSerial(SerialIndex index, Serial serial) { mUse.setSerial(index, serial); }

    ANGLE_INLINE void setSerial(const QueueSerial &queueSerial)
    {
        mUse.setSerial(queueSerial.index, queueSerial.serial);
    }

    // Adds the resource to the list and also records command buffer use.
    void retainCommands(const QueueSerial &queueSerial);

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mUse.usedByCommandBuffer(commandBufferQueueSerial);
    }

  protected:
    Resource();
    Resource(Resource &&other);
    Resource &operator=(Resource &&rhs);

    // Current resource lifetime.
    ResourceUse mUse;
};

ANGLE_INLINE void Resource::retainCommands(const QueueSerial &queueSerial)
{
    mUse.setSerial(queueSerial.index, queueSerial.serial);
}

// Similar to |Resource| above, this tracks object usage. This includes additional granularity to
// track whether an object is used for read-only or read/write access.
class ReadWriteResource : public angle::NonCopyable
{
  public:
    virtual ~ReadWriteResource();

    // Returns true if the resource is used by ANGLE in an unflushed command buffer.
    bool usedInRecordedCommands(Context *context) const;

    // Determine if the driver has finished execution with this resource.
    bool usedInRunningCommands(RendererVk *renderer) const;

    // Returns true if the resource is in use by ANGLE or the driver.
    bool isCurrentlyInUse(RendererVk *renderer) const;
    bool isCurrentlyInUseForWrite(RendererVk *renderer) const;

    // Ensures the driver is caught up to this resource and it is only in use by ANGLE.
    angle::Result finishRunningCommands(ContextVk *contextVk);

    // Ensures the GPU write commands is completed.
    angle::Result finishGPUWriteCommands(ContextVk *contextVk);

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk,
                              const char *debugMessage,
                              RenderPassClosureReason reason);

    // Adds queue serial to the resource.
    ANGLE_INLINE void setReadOnlySerial(const QueueSerial &queueSerial)
    {
        mReadOnlyUse.setSerial(queueSerial.index, queueSerial.serial);
    }

    // Adds the resource to a resource use list.
    void retainReadOnly(const QueueSerial &queueSerial);
    void retainReadWrite(const QueueSerial &queueSerial);

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mReadOnlyUse.usedByCommandBuffer(commandBufferQueueSerial);
    }

    bool writtenByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mReadWriteUse.usedByCommandBuffer(commandBufferQueueSerial);
    }

  protected:
    ReadWriteResource();
    ReadWriteResource(ReadWriteResource &&other);
    ReadWriteResource &operator=(ReadWriteResource &&other);

    // Track any use of the object. Always updated on every retain call.
    ResourceUse mReadOnlyUse;
    // Track read/write use of the object. Only updated for retainReadWrite().
    ResourceUse mReadWriteUse;
};

ANGLE_INLINE void ReadWriteResource::retainReadOnly(const QueueSerial &queueSerial)
{
    // Store reference in resource list.
    mReadOnlyUse.setSerial(queueSerial.index, queueSerial.serial);
}

ANGLE_INLINE void ReadWriteResource::retainReadWrite(const QueueSerial &queueSerial)
{
    // Store reference in resource list.
    mReadOnlyUse.setSerial(queueSerial.index, queueSerial.serial);
    mReadWriteUse.setSerial(queueSerial.index, queueSerial.serial);
}

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
