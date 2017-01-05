//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StreamingBufferManager:
//    Create, map and flush buffers as needed to hold data, returning a handle and offset for each
//    chunk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_MANAGER_H_
#define LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_MANAGER_H_

#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

class StreamingBufferManager : public ResourceVk
{
  public:
    StreamingBufferManager(VkBufferUsageFlags usage, size_t minSize);
    ~StreamingBufferManager();
    gl::Error allocate(ContextVk *context,
                       size_t amount,
                       uint8_t **ptrOut,
                       VkBuffer *handleOut,
                       VkDeviceSize *offsetOut);
    gl::Error flush(ContextVk *context);

  private:
    VkBufferUsageFlags mUsage;
    size_t mMinSize;
    vk::Buffer mBuffer;
    vk::DeviceMemory mMemory;
    VkDeviceSize mNextWriteOffset;
    VkDeviceSize mLastFlushOffset;
    size_t mSize;
    size_t mAlignment;
    uint8_t *mMapping;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_MANAGER_H_
