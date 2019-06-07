//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PersistentCommandPool.h:
//    Defines the class interface for PersistentCommandBuffer and PersistentCommandPool
//

#ifndef LIBANGLE_RENDERER_VULKAN_PERSISTENTCOMMANDPOLL_H_
#define LIBANGLE_RENDERER_VULKAN_PERSISTENTCOMMANDPOLL_H_

#include <vector>

#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

namespace vk
{

class PersistentCommandPool final
{
  public:
    PersistentCommandPool();
    ~PersistentCommandPool();

    void destroy(VkDevice device);
    VkResult init(VkDevice device, uint32_t queueFamilyIndex);

    angle::Result alloc(VkDevice device, vk::PrimaryCommandBuffer *bufferOutput);
    void collect(vk::PrimaryCommandBuffer &&buffer);

  private:
    void allocateCommandBuffers(VkDevice device, uint32_t count);

    uint32_t mAllocatedBufferCount;
    std::vector<std::vector<VkCommandBuffer>> mAllocatedBufferBatches;

    std::vector<vk::PrimaryCommandBuffer> mFreeBuffers;

    vk::CommandPool mCommandPool;

    // The allocate number could be further adjusted to have better performance
    static const int kInitBufferNum     = 3;
    static const int kIncreaseBufferNum = 1;
};

}  // namespace vk
}  // namespace rx

#endif