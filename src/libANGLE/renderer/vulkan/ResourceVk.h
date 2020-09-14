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

#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace vk
{
// Tracks how a resource is used by ANGLE and by a VkQueue. The reference count indicates the number
// of times a resource is retained by ANGLE. The serial indicates the most recent use of a resource
// in the VkQueue. The reference count and serial together can determine if a resource is currently
// in use.
struct ResourceUse
{
    ResourceUse() = default;

    // The number of times a resource is retained by ANGLE.
    uint32_t counter = 0;

    // The most recent time of use in a VkQueue.
    Serial serial;
};

class SharedResourceUse final : angle::NonCopyable
{
  public:
    SharedResourceUse() : mUse(nullptr) {}
    ~SharedResourceUse() { ASSERT(!valid()); }
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
        mUse->counter++;
    }

    // Specifically for use with command buffers that are used as one-offs.
    void updateSerialOneOff(Serial serial) { mUse->serial = serial; }

    ANGLE_INLINE void release()
    {
        ASSERT(valid());
        ASSERT(mUse->counter > 0);
        if (--mUse->counter == 0)
        {
            delete mUse;
        }
        mUse = nullptr;
    }

    ANGLE_INLINE void releaseAndUpdateSerial(Serial serial)
    {
        ASSERT(valid());
        ASSERT(mUse->counter > 0);
        ASSERT(mUse->serial <= serial);
        mUse->serial = serial;
        release();
    }

    ANGLE_INLINE void set(const SharedResourceUse &rhs)
    {
        ASSERT(rhs.valid());
        ASSERT(!valid());
        ASSERT(rhs.mUse->counter < std::numeric_limits<uint32_t>::max());
        mUse = rhs.mUse;
        mUse->counter++;
    }

    // The base counter value for a live resource is "1". Any value greater than one indicates
    // the resource is in use by a command buffer.
    ANGLE_INLINE bool usedInRecordedCommands() const
    {
        ASSERT(valid());
        return mUse->counter > 1;
    }

    ANGLE_INLINE bool usedInRunningCommands(Serial lastCompletedSerial) const
    {
        ASSERT(valid());
        return mUse->serial > lastCompletedSerial;
    }

    ANGLE_INLINE bool isCurrentlyInUse(Serial lastCompletedSerial) const
    {
        return usedInRecordedCommands() || usedInRunningCommands(lastCompletedSerial);
    }

    ANGLE_INLINE Serial getSerial() const
    {
        ASSERT(valid());
        return mUse->serial;
    }

  private:
    ResourceUse *mUse;
};

class SharedResourceUsePool
{
  public:
    SharedResourceUsePool() {}
    ~SharedResourceUsePool() { releaseSharedResouceUsePool(); }

    ANGLE_INLINE SharedResourceUse *acquireSharedResouceUse()
    {
        if (mSharedResourceUseFreeList.empty())
        {
            ensureCapacity();
        }

        SharedResourceUse *sharedResourceUse = mSharedResourceUseFreeList.back();
        mSharedResourceUseFreeList.pop_back();
        return sharedResourceUse;
    }

    ANGLE_INLINE void releaseSharedResouceUse(SharedResourceUse *sharedResourceUse)
    {
        mSharedResourceUseFreeList.push_back(sharedResourceUse);
    }

  private:
    void releaseSharedResouceUsePool()
    {
        mSharedResourceUseFreeList.clear();
        for (SharedResourceUseBlock &block : mSharedResourceUsePool)
        {
            block.clear();
        }
        mSharedResourceUsePool.clear();
    }

    void ensureCapacity()
    {
        // Allocate a SharedResourceUse block
        constexpr size_t kSharedResourceUseBlockSize = 2048;
        size_t newSize                               = (mSharedResourceUsePool.empty())
                             ? kSharedResourceUseBlockSize
                             : mSharedResourceUsePool.back().capacity() * 2;
        SharedResourceUseBlock sharedResourceUseBlock;
        sharedResourceUseBlock.resize(newSize);

        // Append it to the SharedResourceUse pool
        mSharedResourceUsePool.emplace_back(std::move(sharedResourceUseBlock));

        // Add the newly allocated SharedResourceUse to the free list
        mSharedResourceUseFreeList.reserve(newSize);
        SharedResourceUseBlock &newSharedResourceUseBlock = mSharedResourceUsePool.back();
        for (SharedResourceUse &use : newSharedResourceUseBlock)
        {
            mSharedResourceUseFreeList.push_back(&use);
        }
    }

    using SharedResourceUseBlock = std::vector<SharedResourceUse>;
    std::vector<SharedResourceUseBlock> mSharedResourceUsePool;
    std::vector<SharedResourceUse *> mSharedResourceUseFreeList;
};

class SharedGarbage
{
  public:
    SharedGarbage();
    SharedGarbage(SharedGarbage &&other);
    SharedGarbage(SharedResourceUse &&use, std::vector<GarbageObject> &&garbage);
    ~SharedGarbage();
    SharedGarbage &operator=(SharedGarbage &&rhs);

    bool destroyIfComplete(RendererVk *renderer, Serial completedSerial);

  private:
    SharedResourceUse mLifetime;
    std::vector<GarbageObject> mGarbage;
};

using SharedGarbageList = std::vector<SharedGarbage>;

// Mixin to abstract away the resource use tracking.
class ResourceUseList final : angle::NonCopyable
{
  public:
    ResourceUseList();
    virtual ~ResourceUseList();

    ANGLE_INLINE void add(const SharedResourceUse &resourceUse,
                          SharedResourceUsePool *sharedResourceUsePool)
    {
        SharedResourceUse *newUse = sharedResourceUsePool->acquireSharedResouceUse();
        newUse->set(resourceUse);
        mResourceUses.emplace_back(newUse);
    }

    ANGLE_INLINE void releaseResourceUses(SharedResourceUsePool *sharedResourceUsePool)
    {
        for (SharedResourceUse *use : mResourceUses)
        {
            use->release();
            sharedResourceUsePool->releaseSharedResouceUse(use);
        }
        mResourceUses.clear();
    }
    ANGLE_INLINE void releaseResourceUsesAndUpdateSerials(
        Serial serial,
        SharedResourceUsePool *sharedResourceUsePool)
    {
        for (SharedResourceUse *use : mResourceUses)
        {
            use->releaseAndUpdateSerial(serial);
            sharedResourceUsePool->releaseSharedResouceUse(use);
        }
        mResourceUses.clear();
    }

  private:
    std::vector<SharedResourceUse *> mResourceUses;
};

// This is a helper class for back-end objects used in Vk command buffers. They keep a record
// of their use in ANGLE and VkQueues via SharedResourceUse.
class Resource : angle::NonCopyable
{
  public:
    virtual ~Resource();

    // Returns true if the resource is used by ANGLE in an unflushed command buffer.
    bool usedInRecordedCommands() const { return mUse.usedInRecordedCommands(); }

    // Determine if the driver has finished execution with this resource.
    bool usedInRunningCommands(Serial lastCompletedSerial) const
    {
        return mUse.usedInRunningCommands(lastCompletedSerial);
    }

    // Returns true if the resource is in use by ANGLE or the driver.
    bool isCurrentlyInUse(Serial lastCompletedSerial) const
    {
        return mUse.isCurrentlyInUse(lastCompletedSerial);
    }

    // Ensures the driver is caught up to this resource and it is only in use by ANGLE.
    angle::Result finishRunningCommands(ContextVk *contextVk);

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk, const char *debugMessage);

    // Adds the resource to a resource use list.
    ANGLE_INLINE void retain(ResourceUseList *resourceUseList,
                             SharedResourceUsePool *sharedResourceUsePool)
    {
        // Store reference in resource list.
        resourceUseList->add(mUse, sharedResourceUsePool);
    }

  protected:
    Resource();
    Resource(Resource &&other);

    // Current resource lifetime.
    SharedResourceUse mUse;
};
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
