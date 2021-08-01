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

#include <stack>

namespace rx
{
namespace vk
{
enum class ResourceUseType
{
    Read,       // Resource is being used/read by the GPU
    ReadWrite,  // Resource is being read and/or written by the GPU

    InvalidEnum,
    EnumCount = InvalidEnum,
};

// Tracks how a resource is used by ANGLE and by a VkQueue. The reference count indicates the number
// of times a resource is retained by ANGLE. The accessSerial indicates the most recent use of a
// resource in the VkQueue. The reference count and accessSerial together can determine if a
// resource is currently in use.
struct ResourceUse
{
    ResourceUse() = default;

    // The number of times a resource is retained by ANGLE.
    uint32_t accessCounter = 0;
    std::stack<uint32_t> writeAccessCounts;

    // The most recent time of use in a VkQueue.
    Serial readSerial;
    Serial readWriteSerial;
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
        mUse->accessCounter++;
    }

    // Specifically for use with command buffers that are used as one-offs.
    void updateSerialOneOff(Serial serial)
    {
        ASSERT(valid());
        ASSERT(mUse->readSerial <= serial && mUse->readWriteSerial <= serial);
        mUse->readSerial = serial;

        // Only update the Serial for the matching command.
        if (!mUse->writeAccessCounts.empty() &&
            mUse->writeAccessCounts.top() == mUse->accessCounter)
        {
            mUse->readWriteSerial = serial;
            mUse->writeAccessCounts.pop();
        }
    }

    ANGLE_INLINE void release()
    {
        ASSERT(valid());
        // The accessCounter acts as the "lifetime" counter.
        ASSERT(mUse->accessCounter > 0);

        // Pop the access if the resource is released before the command using it is submitted.
        if (!mUse->writeAccessCounts.empty() &&
            mUse->writeAccessCounts.top() == mUse->accessCounter)
        {
            mUse->writeAccessCounts.pop();
        }

        --mUse->accessCounter;

        if (mUse->accessCounter == 0)
        {
            ASSERT(mUse->writeAccessCounts.empty());
            delete mUse;
        }
        mUse = nullptr;
    }

    ANGLE_INLINE void releaseAndUpdateSerial(Serial serial)
    {
        updateSerialOneOff(serial);
        release();
    }

    ANGLE_INLINE void set(const SharedResourceUse &rhs, ResourceUseType resourceUseType)
    {
        ASSERT(rhs.valid());
        ASSERT(!valid());
        ASSERT(rhs.mUse->accessCounter < std::numeric_limits<uint32_t>::max());
        mUse = rhs.mUse;

        // Always increment the accessCounter, since that also behaves as the "lifetime" counter.
        mUse->accessCounter++;
        ASSERT(mUse->accessCounter < std::numeric_limits<uint32_t>::max());

        switch (resourceUseType)
        {
            case ResourceUseType::Read:
                // Counter already incremented.
                break;
            case ResourceUseType::ReadWrite:
                mUse->writeAccessCounts.push(mUse->accessCounter);
                break;
            default:
                UNREACHABLE();
        }
    }

    // The base accessCounter value for a live resource is "1". Any value greater than one indicates
    // the resource is in use by a command buffer.
    ANGLE_INLINE bool usedInRecordedCommands() const
    {
        ASSERT(valid());
        return mUse->accessCounter > 1;
    }
    ANGLE_INLINE bool usedInRecordedCommandsForWrite() const
    {
        ASSERT(valid());
        return mUse->writeAccessCounts.size() > 0;
    }

    ANGLE_INLINE bool usedInRunningCommands(Serial lastCompletedSerial) const
    {
        ASSERT(valid());
        return mUse->readSerial > lastCompletedSerial ||
               mUse->readWriteSerial > lastCompletedSerial;
    }
    ANGLE_INLINE bool usedInRunningCommandsForWrite(Serial lastCompletedSerial) const
    {
        ASSERT(valid());
        return mUse->readWriteSerial > lastCompletedSerial;
    }

    ANGLE_INLINE bool isCurrentlyInUse(Serial lastCompletedSerial) const
    {
        return usedInRecordedCommands() || usedInRunningCommands(lastCompletedSerial);
    }
    ANGLE_INLINE bool isCurrentlyInUseForWrite(Serial lastCompletedSerial) const
    {
        return usedInRecordedCommandsForWrite() ||
               usedInRunningCommandsForWrite(lastCompletedSerial);
    }

    ANGLE_INLINE Serial getLatestSerial() const
    {
        ASSERT(valid());
        Serial serial =
            mUse->readSerial >= mUse->readWriteSerial ? mUse->readSerial : mUse->readWriteSerial;
        return serial;
    }

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
    ResourceUseList(ResourceUseList &&other);
    virtual ~ResourceUseList();
    ResourceUseList &operator=(ResourceUseList &&rhs);

    void add(const SharedResourceUse &resourceUse, ResourceUseType resourceUseType);

    void releaseResourceUses();
    void releaseResourceUsesAndUpdateSerials(Serial accessSerial);

    bool empty() { return mResourceUses.empty(); }

  private:
    std::vector<SharedResourceUse> mResourceUses;
};

ANGLE_INLINE void ResourceUseList::add(const SharedResourceUse &resourceUse,
                                       ResourceUseType resourceUseType)
{
    SharedResourceUse newUse;
    newUse.set(resourceUse, resourceUseType);
    mResourceUses.emplace_back(std::move(newUse));
}

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
    bool isCurrentlyInUseForWrite(Serial lastCompletedSerial) const
    {
        return mUse.isCurrentlyInUseForWrite(lastCompletedSerial);
    }

    // Ensures the driver is caught up to this resource and it is only in use by ANGLE.
    angle::Result finishRunningCommands(ContextVk *contextVk);

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk, const char *debugMessage);

    // Adds the resource to a resource use list.
    void retain(ResourceUseList *resourceUseList, ResourceUseType resourceUseType) const;

  protected:
    Resource();
    Resource(Resource &&other);

    // Current resource lifetime.
    SharedResourceUse mUse;
};

ANGLE_INLINE void Resource::retain(ResourceUseList *resourceUseList,
                                   ResourceUseType resourceUseType) const
{
    // Store reference in resource list.
    resourceUseList->add(mUse, resourceUseType);
}

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
