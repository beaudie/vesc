//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicDescriptorPool:
//    Uses DescriptorPool to allocate descriptor sets as needed. If the descriptor pool
//    is full, we simply allocate a new pool to keep allocating descriptor sets as needed and
//    leave the renderer take care of the life time of the pools that become unused.
//

#include "DynamicDescriptorPool.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "vulkan/vulkan.hpp"

namespace rx
{
namespace
{

// TODO(jmadill): Pick non-arbitrary max.
constexpr uint32_t kMaxSets = 2048;
}  // anonymous namespace

DynamicDescriptorPool::DynamicDescriptorPool() : mUniformBufferPoolCount(0), mTexturePoolCount(0)
{
}

DynamicDescriptorPool::~DynamicDescriptorPool()
{
    ASSERT(!mCurrentDescriptorSetPool.valid());
    ASSERT(mContextVk == nullptr);
}

void DynamicDescriptorPool::destroy()
{
    ASSERT(mCurrentDescriptorSetPool.valid());
    mCurrentDescriptorSetPool.destroy(mContextVk->getDevice());
    mContextVk = nullptr;
}

vk::Error DynamicDescriptorPool::init(ContextVk *contextVk,
                                      uint32_t uniformBufferPoolCount,
                                      uint32_t texturePoolCount)
{
    mContextVk              = contextVk;
    mUniformBufferPoolCount = uniformBufferPoolCount;
    mTexturePoolCount       = texturePoolCount;

    ANGLE_TRY(allocateNewPool());
    return vk::NoError();
}

vk::Error DynamicDescriptorPool::allocateDescriptorSets(
    const VkDescriptorSetLayout *descriptorSetLayout,
    uint32_t descriptorSetCount,
    VkDescriptorSet *descriptorSetsOut)
{
    updateQueueSerial(mContextVk->getRenderer()->getCurrentQueueSerial());

    if (descriptorSetCount + mDescriptorSetsCountInPool > kMaxSets)
    {
        // We will bust the limit of descriptor set with this allocation so we need to get a new
        // pool for it.
        mContextVk->getRenderer()->releaseResource(*this, &mCurrentDescriptorSetPool);
        ANGLE_TRY(allocateNewPool());
    }

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext              = nullptr;
    allocInfo.descriptorPool     = mCurrentDescriptorSetPool.getHandle();
    allocInfo.descriptorSetCount = descriptorSetCount;
    allocInfo.pSetLayouts        = descriptorSetLayout;

    ANGLE_TRY(mCurrentDescriptorSetPool.allocateDescriptorSets(mContextVk->getDevice(), allocInfo,
                                                               descriptorSetsOut));
    mDescriptorSetsCountInPool += allocInfo.descriptorSetCount;
    return vk::NoError();
}

VkDescriptorPool DynamicDescriptorPool::getCurrentPoolHandle()
{
    return mCurrentDescriptorSetPool.getHandle();
}

vk::Error DynamicDescriptorPool::allocateNewPool()
{
    constexpr size_t kPoolSizeCount = 2;

    VkDescriptorPoolSize poolSizes[kPoolSizeCount];
    poolSizes[UniformBufferPool].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[UniformBufferPool].descriptorCount =
        mUniformBufferPoolCount * kMaxSets / kPoolSizeCount;
    poolSizes[TexturePool].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[TexturePool].descriptorCount = mTexturePoolCount * kMaxSets / kPoolSizeCount;

    VkDescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext   = nullptr;
    descriptorPoolInfo.flags   = 0;
    descriptorPoolInfo.maxSets = kMaxSets;

    // Reserve pools for uniform blocks and textures.
    descriptorPoolInfo.poolSizeCount = kPoolSizeCount;
    descriptorPoolInfo.pPoolSizes    = poolSizes;

    mDescriptorSetsCountInPool = 0;
    ANGLE_TRY(mCurrentDescriptorSetPool.init(mContextVk->getDevice(), descriptorPoolInfo));
    return vk::NoError();
}
}  // namespace rx
