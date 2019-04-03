// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObjectVkFd.h: Defines the class interface for MemoryObjectVkFd, implementing
// MemoryObjectImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_LINUX_MEMORYOBJECTVKFD_H_
#define LIBANGLE_RENDERER_VULKAN_LINUX_MEMORYOBJECTVKFD_H_

#include "libANGLE/renderer/vulkan/MemoryObjectVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

class MemoryObjectVkFd : public MemoryObjectVk
{
  public:
    MemoryObjectVkFd();
    ~MemoryObjectVkFd() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result importMemoryFd(gl::Context *context,
                                 GLuint64 size,
                                 GLenum handleType,
                                 GLint fd) override;

    angle::Result initImage(const gl::Context *context,
                            gl::TextureType type,
                            size_t levels,
                            GLenum internalFormat,
                            const gl::Extents &size,
                            GLuint64 offset,
                            vk::ImageHelper *image) override;

  private:
    angle::Result importMemoryOpaqueFd(gl::Context *context, GLuint64 size, GLint fd);

    size_t mSize;
    int mFd;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_LINUX_MEMORYOBJECTVKFD_H_
