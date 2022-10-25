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
// Estimated maximum command buffers in a normal session. Beyond this we will lose performance.
constexpr size_t kMaxFastCommandBuffers = 64;

struct CommandBufferID
{
    GLuint value;
};

inline bool operator==(CommandBufferID lhs, CommandBufferID rhs)
{
    return lhs.value == rhs.value;
}

using CommandBufferHandleAllocator = gl::HandleAllocator;

// Tracks open command buffers for a resource.
// We could optimize this with a hybrid packed bitset/list for applications that use many contexts.
using ResourceCommandBuffers = angle::FlatUnorderedSet<CommandBufferID, kMaxFastCommandBuffers>;

// Tracks how a resource is used by ANGLE and by a VkQueue. The reference count indicates the number
// of times a resource is retained by ANGLE. The serial indicates the most recent use of a resource
// in the VkQueue. The reference count and serial together can determine if a resource is currently
// in use.
struct ResourceUse
{
    ResourceUse() = default;
    ResourceUse(const ResourceUse &other)
        : commandBuffers(other.commandBuffers), serials(other.serials)
    {}

    // Open command buffers using this resource.
    ResourceCommandBuffers commandBuffers;

    // The most recent time of use in a VkQueue.
    Serials serials;
};

class SharedResourceUse final
{
  public:
    SharedResourceUse() : mUse(nullptr) {}
    ~SharedResourceUse() { ASSERT(!valid()); }
    SharedResourceUse(const SharedResourceUse &other) { mUse = new ResourceUse(*other.mUse); }
    SharedResourceUse(SharedResourceUse &&rhs) : mUse(rhs.mUse) { rhs.mUse = nullptr; }
    SharedResourceUse &operator=(SharedResourceUse &&rhs)
    {
        std::swap(mUse, rhs.mUse);
        return *this;
    }

    ANGLE_INLINE bool valid() const { return mUse != nullptr; }

    void init()
    {
        ASSERT(!mUse);
        mUse = new ResourceUse;
    }

    ANGLE_INLINE void release()
    {
        ASSERT(valid());
        delete mUse;
        mUse = nullptr;
    }

    ANGLE_INLINE void set(const SharedResourceUse &rhs)
    {
        ASSERT(rhs.valid());
        ASSERT(!valid());
        mUse = rhs.mUse;
    }

    // The base counter value for a live resource is "1". Any value greater than one indicates
    // the resource is in use by a command buffer.
    ANGLE_INLINE bool usedInRecordedCommand(SerialIndex index, Serial lastSubmittedSerial) const
    {
        ASSERT(valid());
        return mUse->serials[index] > lastSubmittedSerial;
    }

    ANGLE_INLINE bool usedInRecordedCommands(const Serials &lastSubmittedSerials) const
    {
        ASSERT(valid());
        for (SerialIndex index(0); index < mUse->serials.size(); ++index)
        {
            if (mUse->serials[SerialIndex(index)] > lastSubmittedSerials[SerialIndex(index)])
            {
                return true;
            }
        }
        return false;
    }

    ANGLE_INLINE bool usedInRunningCommand(SerialIndex index, Serial lastCompletedSerial) const
    {
        ASSERT(valid());
        return mUse->serials[index] > lastCompletedSerial;
    }

    ANGLE_INLINE bool usedInRunningCommands(const Serials &lastCompletedSerials) const
    {
        ASSERT(valid());
        for (SerialIndex index(0); index < mUse->serials.size(); ++index)
        {
            if (mUse->serials[SerialIndex(index)] > lastCompletedSerials[SerialIndex(index)])
            {
                return true;
            }
        }
        return false;
    }

    ANGLE_INLINE bool isCurrentlyInUse(const Serials &lastCompletedSerials) const
    {
        return usedInRunningCommands(lastCompletedSerials);
    }

    ANGLE_INLINE Serial getSerial(SerialIndex index) const
    {
        ASSERT(valid());
        return mUse->serials[index];
    }

    ANGLE_INLINE const Serials &getSerials() const { return mUse->serials; }

    ANGLE_INLINE void setCommandBuffer(CommandBufferID commandBufferID)
    {
        if (!mUse->commandBuffers.contains(commandBufferID))
        {
            mUse->commandBuffers.insert(commandBufferID);
        }
    }

    ANGLE_INLINE void clearCommandBuffer(CommandBufferID commandBufferID)
    {
        if (mUse->commandBuffers.contains(commandBufferID))
        {
            mUse->commandBuffers.remove(commandBufferID);
        }
    }

    ANGLE_INLINE bool usedByCommandBuffer(CommandBufferID commandBufferID) const
    {
        return mUse->commandBuffers.contains(commandBufferID);
    }

    ANGLE_INLINE void setSerial(const QueueSerial &queueSerial)
    {
        mUse->serials[queueSerial.index] = queueSerial.serial;
    }

    ANGLE_INLINE void setSerial(SerialIndex index, Serial serial) { mUse->serials[index] = serial; }

  private:
    ResourceUse *mUse;
};

class SharedGarbage
{
  public:
    SharedGarbage();
    SharedGarbage(SharedGarbage &&other);
    SharedGarbage(SharedResourceUse &&use, std::vector<GarbageObject> &&garbage);
    ~SharedGarbage();
    SharedGarbage &operator=(SharedGarbage &&rhs);

    bool destroyIfComplete(RendererVk *renderer);
    bool usedInRecordedCommands(RendererVk *renderer) const;

  private:
    SharedResourceUse mLifetime;
    std::vector<GarbageObject> mGarbage;
};

using SharedGarbageList = std::queue<SharedGarbage>;

// This is a helper class for back-end objects used in Vk command buffers. They keep a record
// of their use in ANGLE and VkQueues via SharedResourceUse.
class Resource : angle::NonCopyable
{
  public:
    virtual ~Resource();

    // Returns true if the resource is used by ANGLE in an unflushed command buffer.
    bool usedInRecordedCommand(Context *context) const
    {
        return mUse.usedInRecordedCommand(context->getSerialIndex(),
                                          context->getLastSubmittedSerial());
    }

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
    ANGLE_INLINE void setSerial(const QueueSerial &queueSerial) { mUse.setSerial(queueSerial); }

    // Adds the resource to the list and also records command buffer use.
    void retainCommands(CommandBufferID commandBufferID, const QueueSerial &queueSerial);

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(CommandBufferID commandBufferID) const
    {
        return mUse.usedByCommandBuffer(commandBufferID);
    }

  protected:
    Resource();
    Resource(Resource &&other);
    Resource &operator=(Resource &&rhs);

    // Current resource lifetime.
    SharedResourceUse mUse;
};

ANGLE_INLINE void Resource::retainCommands(CommandBufferID commandBufferID,
                                           const QueueSerial &queueSerial)
{
    mUse.setCommandBuffer(commandBufferID);
    mUse.setSerial(queueSerial);
}

// Similar to |Resource| above, this tracks object usage. This includes additional granularity to
// track whether an object is used for read-only or read/write access.
class ReadWriteResource : public angle::NonCopyable
{
  public:
    virtual ~ReadWriteResource();

    // Returns true if the resource is used by ANGLE in an unflushed command buffer.
    bool usedInRecordedCommand(Context *context) const
    {
        return mReadOnlyUse.usedInRecordedCommand(context->getSerialIndex(),
                                                  context->getLastSubmittedSerial());
    }
    bool usedInRecordedCommands(const Serials &lastSubmittedSerials) const
    {
        return mReadOnlyUse.usedInRecordedCommands(lastSubmittedSerials);
    }

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
        mReadOnlyUse.setSerial(queueSerial);
    }

    // Adds the resource to a resource use list.
    void retainReadOnly(CommandBufferID commandBufferID, const QueueSerial &queueSerial);
    void retainReadWrite(CommandBufferID commandBufferID, const QueueSerial &queueSerial);

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(CommandBufferID commandBufferID) const
    {
        return mReadOnlyUse.usedByCommandBuffer(commandBufferID);
    }

    bool writtenByCommandBuffer(CommandBufferID commandBufferID) const
    {
        return mReadWriteUse.usedByCommandBuffer(commandBufferID);
    }

  protected:
    ReadWriteResource();
    ReadWriteResource(ReadWriteResource &&other);
    ReadWriteResource &operator=(ReadWriteResource &&other);

    // Track any use of the object. Always updated on every retain call.
    SharedResourceUse mReadOnlyUse;
    // Track read/write use of the object. Only updated for retainReadWrite().
    SharedResourceUse mReadWriteUse;
};

ANGLE_INLINE void ReadWriteResource::retainReadOnly(CommandBufferID commandBufferID,
                                                    const QueueSerial &queueSerial)
{
    // Store reference in resource list.
    mReadOnlyUse.setSerial(queueSerial);
    mReadOnlyUse.setCommandBuffer(commandBufferID);
}

ANGLE_INLINE void ReadWriteResource::retainReadWrite(CommandBufferID commandBufferID,
                                                     const QueueSerial &queueSerial)
{
    // Store reference in resource list.
    mReadOnlyUse.setSerial(queueSerial);
    mReadWriteUse.setSerial(queueSerial);

    mReadOnlyUse.setCommandBuffer(commandBufferID);
    mReadWriteUse.setCommandBuffer(commandBufferID);
}

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
