//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StreamingBuffer:
//    For streaming data to a series of VkBuffer, returning a handle and offset for each chunk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_H_
#define LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_H_

#include "libANGLE/renderer/vulkan/renderervk_utils.h"

namespace rx
{

class StreamingBuffer : public ResourceVk
{
  public:
    StreamingBuffer();
    ~StreamingBuffer();
    gl::Error allocate(ContextVk *context,
                       size_t amount,
                       uint8_t **ptrOut,
                       VkBuffer *handleOut,
                       VkDeviceSize *offsetOut);
    gl::Error flush(ContextVk *context);

  private:
    vk::Buffer mBuffer;
    vk::DeviceMemory mMemory;
    VkDeviceSize mOffset;
    VkDeviceSize mLastFlush;
    size_t mSize;
    uint8_t *mPtr;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_STREAMING_BUFFER_H_
