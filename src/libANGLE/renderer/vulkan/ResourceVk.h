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
enum class ResourceUseType
{
    Read,       // Resource is being used/read by the GPU
    ReadWrite,  // Resource is being read and/or written by the GPU

    InvalidEnum,
    EnumCount = InvalidEnum,
};

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

    void add(const SharedResourceUse &resourceUse);

    void releaseResourceUses();
    void releaseResourceUsesAndUpdateSerials(Serial serial);

    bool empty() { return mResourceUses.empty(); }

  private:
    std::vector<SharedResourceUse> mResourceUses;
};

ANGLE_INLINE void ResourceUseList::add(const SharedResourceUse &resourceUse)
{
    SharedResourceUse newUse;
    newUse.set(resourceUse);
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

    // Ensures the driver is caught up to this resource and it is only in use by ANGLE.
    angle::Result finishRunningCommands(ContextVk *contextVk);

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk, const char *debugMessage);

    // Adds the resource to a resource use list.
    void retain(ResourceUseList *resourceUseList) const;

  protected:
    Resource();
    Resource(Resource &&other);

    // Current resource lifetime.
    SharedResourceUse mUse;
};

class ReadWriteResource : public Resource
{
  public:
    virtual ~ReadWriteResource() override;

    bool isCurrentlyInUseForWrite(Serial lastCompletedSerial) const
    {
        return mWriteUse.isCurrentlyInUse(lastCompletedSerial);
    }

    ANGLE_INLINE void retain(ResourceUseList *resourceUseList,
                             ResourceUseType resourceUseType) const
    {
        retain(resourceUseList);

        // Store reference in resource list.
        if (resourceUseType == ResourceUseType::ReadWrite)
        {
            resourceUseList->add(mWriteUse);
        }
    }

  protected:
    ReadWriteResource();
    ReadWriteResource(ReadWriteResource &&other);

    // Current resource lifetime.
    SharedResourceUse mWriteUse;

  private:
    // Make retain(ResourceUseList) private, so users are forced to call retain(ResourceUseList,
    // ResourceUseType) and explicitly decide whether the access is a Read or ReadWrite.
    using Resource::retain;
};

ANGLE_INLINE void Resource::retain(ResourceUseList *resourceUseList) const
{
    // Store reference in resource list.
    resourceUseList->add(mUse);
}

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
