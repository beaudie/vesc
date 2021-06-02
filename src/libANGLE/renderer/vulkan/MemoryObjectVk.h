// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObjectVk.h: Defines the class interface for MemoryObjectVk,
// implementing MemoryObjectImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_MEMORYOBJECTVK_H_
#define LIBANGLE_RENDERER_VULKAN_MEMORYOBJECTVK_H_

#include "common/platform.h"
#include "libANGLE/renderer/MemoryObjectImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#if defined(ANGLE_PLATFORM_FUCHSIA)
#include <zircon/types.h>
#endif

namespace rx
{

class MemoryObjectVk : public MemoryObjectImpl
{
  public:
    MemoryObjectVk();
    ~MemoryObjectVk() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result setDedicatedMemory(const gl::Context *context, bool dedicatedMemory) override;

    angle::Result importFd(gl::Context *context,
                           GLuint64 size,
                           gl::HandleType handleType,
                           GLint fd) override;

    angle::Result importZirconHandle(gl::Context *context,
                                     GLuint64 size,
                                     gl::HandleType handleType,
                                     GLuint handle) override;

    angle::Result createImage(ContextVk *context,
                              gl::TextureType type,
                              size_t levels,
                              GLenum internalFormat,
                              const gl::Extents &size,
                              GLuint64 offset,
                              vk::ImageHelper *image,
                              GLbitfield createFlags,
                              GLbitfield usageFlags);

  private:
    static constexpr int kInvalidFd = -1;
    angle::Result importOpaqueFd(ContextVk *contextVk, GLuint64 size, GLint fd);
    angle::Result importZirconVmo(ContextVk *contextVk, GLuint64 size, GLuint handle);

    // Imported memory object was a dedicated allocation.
    bool mDedicatedMemory = false;

    GLuint64 mSize             = 0;
    gl::HandleType mHandleType = gl::HandleType::InvalidEnum;
    int mFd                    = kInvalidFd;

#if defined(ANGLE_PLATFORM_FUCHSIA)
    zx_handle_t mZirconHandle = ZX_HANDLE_INVALID;
#endif
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MEMORYOBJECTVK_H_
