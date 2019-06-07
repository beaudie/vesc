//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandVk.h:
//    Defines the class interface for PersistantCommandBuffer and PersistantCommandPool
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMMANDVK_H_
#define LIBANGLE_RENDERER_VULKAN_COMMANDVK_H_

#include <vector>

#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

namespace vk
{

class PersistantCommandPool;

class PersistantCommandBuffer final : public priv::CommandBuffer
{
    friend class PersistantCommandPool;

  public:
    PersistantCommandBuffer()
        : priv::CommandBuffer(),
          mInflight(false),
          mPoolAllocateId(kNullPollAllocatedId),
          mOwnerPool(nullptr)
    {}
    virtual ~PersistantCommandBuffer();

    // Only works to initialize empty objects
    PersistantCommandBuffer &operator=(PersistantCommandBuffer &&other)
    {
        ASSERT(!allocated());
        std::swap(mHandle, other.mHandle);
        std::swap(mInflight, other.mInflight);
        std::swap(mPoolAllocateId, other.mPoolAllocateId);
        std::swap(mOwnerPool, other.mOwnerPool);
        return *this;
    }
    PersistantCommandBuffer(PersistantCommandBuffer &&other) { *this = std::move(other); }

    bool allocated() const;

    void destroy(VkDevice device);

  private:
    PersistantCommandBuffer(PersistantCommandPool *pPool,
                            int32_t allocateId,
                            const VkCommandBuffer &vkBufRef)
        : priv::CommandBuffer(), mInflight(false), mPoolAllocateId(allocateId), mOwnerPool(pPool)
    {
        mHandle = vkBufRef;
    }

    bool mInflight;

    static constexpr int kNullPollAllocatedId = -1;
    int32_t mPoolAllocateId;

    PersistantCommandPool *mOwnerPool;
};

class PersistantCommandPool final : public WrappedObject<PersistantCommandPool, VkCommandPool>
{
  public:
    PersistantCommandPool();
    ~PersistantCommandPool();

    void destroy(VkDevice device);
    VkResult init(VkDevice device, uint32_t queueFamilyIndex);

    // Should only use to allocate primary CommandBuffer
    angle::Result alloc(VkDevice device, PersistantCommandBuffer *bufferOutput);
    void collect(PersistantCommandBuffer &&buffer);

  private:
    struct CommandBufferArrayHandle
    {
        VkCommandBuffer *array;
        uint32_t size;
    };
    void allocateBatch(VkDevice device) { allocateCommandBufferArray(device, kIncreaseBufferNum); }
    void allocateCommandBufferArray(VkDevice device, uint32_t count);

    uint32_t mAllocatedBufferCount;
    std::vector<CommandBufferArrayHandle> mAllocatedBufferArrays;

    std::vector<PersistantCommandBuffer> mFreeBuffers;

    const int kInitBufferNum;
    const int kIncreaseBufferNum;
};

}  // namespace vk
}  // namespace rx

#endif