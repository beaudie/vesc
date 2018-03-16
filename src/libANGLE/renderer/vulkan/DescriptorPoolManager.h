//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DescriptorPoolManager:
//    Uses DescriptorPool to allocate descriptor sets as needed. If the descriptor pool
//    is full, we simply allocate a new pool to keep allocating descriptor sets as needed and
//    leave the renderer take care of the life time of the pools that become unused.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DESCRIPTOR_POOL_MANAGER_H_
#define LIBANGLE_RENDERER_VULKAN_DESCRIPTOR_POOL_MANAGER_H_

#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

enum DescriptorPoolIndex : uint8_t
{
    UniformBufferPool = 0,
    TexturePool       = 1,
};

class DescriptorPoolManager : public ResourceVk
{
  public:
    DescriptorPoolManager();
    ~DescriptorPoolManager();
    void destroy();
    vk::Error init(ContextVk *contextVk);
    vk::Error allocateDescriptorSets(const VkDescriptorSetLayout*, uint32_t descriptorSetCount,
                                 VkDescriptorSet *descriptorSetsOut);
    VkDescriptorPool getCurrentPoolHandle();

private:
    vk::Error allocateNewPool();

    vk::DescriptorPool mCurrentDescriptorSetPool;
    size_t mDescriptorSetsCountInPool;
    ContextVk *mContextVk;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DESCRIPTOR_POOL_MANAGER_H_
