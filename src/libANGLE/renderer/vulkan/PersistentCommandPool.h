//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PersistentCommandPool.h:
//    Defines the class interface for PersistentCommandBuffer
//

#ifndef LIBANGLE_RENDERER_VULKAN_PERSISTENTCOMMANDPOOL_H_
#define LIBANGLE_RENDERER_VULKAN_PERSISTENTCOMMANDPOOL_H_

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

    void destroy(const vk::Context *context);
    angle::Result init(vk::Context *context, uint32_t queueFamilyIndex);

    angle::Result alloc(vk::Context *context, vk::PrimaryCommandBuffer *bufferOutput);
    void collect(vk::PrimaryCommandBuffer &&buffer);

  private:
    angle::Result allocateCommandBuffers(vk::Context *context);
    uint32_t mAllocatedBufferCount;

    std::vector<vk::PrimaryCommandBuffer> mFreeBuffers;

    vk::CommandPool mCommandPool;

    static const int kInitBufferNum = 3;
};

}  // namespace vk

}  // namespace rx

#endif