// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObjectVkFd.cpp: Defines the class interface for MemoryObjectVkFd, implementing
// MemoryObjectImpl.

#include "libANGLE/renderer/vulkan/linux/MemoryObjectVkFd.h"

#include <unistd.h>
#include <vulkan/vulkan.h>

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

MemoryObjectVkFd::MemoryObjectVkFd() : mSize(0), mFd(-1) {}

MemoryObjectVkFd::~MemoryObjectVkFd() = default;

void MemoryObjectVkFd::onDestroy(const gl::Context *context)
{
    if (mFd != -1)
    {
        close(mFd);
        mFd = -1;
    }
}

angle::Result MemoryObjectVkFd::importMemoryFd(gl::Context *context,
                                               GLuint64 size,
                                               GLenum handleType,
                                               GLint fd)
{
    switch (handleType)
    {
        case GL_HANDLE_TYPE_OPAQUE_FD_EXT:
            return importMemoryOpaqueFd(context, size, fd);

        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

angle::Result MemoryObjectVkFd::importMemoryOpaqueFd(gl::Context *context, GLuint64 size, GLint fd)
{
    ASSERT(mFd == -1);
    mFd   = fd;
    mSize = size;
    return angle::Result::Continue;
}

angle::Result MemoryObjectVkFd::initImage(const gl::Context *context,
                                          gl::TextureType type,
                                          size_t levels,
                                          GLenum internalFormat,
                                          const gl::Extents &size,
                                          GLuint64 offset,
                                          vk::ImageHelper *image)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    const vk::Format &vkFormat = renderer->getFormat(internalFormat);

    // TODO(spang): Check capabilities; this must == supported usage flags.
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    ANGLE_TRY(image->initExternal(contextVk, type, size, vkFormat, 1, usage,
                                  vk::ImageLayout::ExternalPreInitialized,
                                  &externalMemoryImageCreateInfo, levels, 1));

    VkMemoryRequirements externalMemoryRequirements;
    vkGetImageMemoryRequirements(renderer->getDevice(), image->getImage().getHandle(),
                                 &externalMemoryRequirements);

    ASSERT(mFd != -1);
    VkImportMemoryFdInfoKHR importMemoryFdInfo = {};
    importMemoryFdInfo.sType                   = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
    importMemoryFdInfo.handleType              = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    importMemoryFdInfo.fd                      = dup(mFd);

    // TODO(jmadill, spang): Memory sub-allocation. http://anglebug.com/2162
    ASSERT(offset == 0);
    ASSERT(externalMemoryRequirements.size == mSize);

    VkMemoryPropertyFlags flags = 0;
    ANGLE_TRY(image->initExternalMemory(contextVk, renderer->getMemoryProperties(),
                                        externalMemoryRequirements, &importMemoryFdInfo,
                                        VK_QUEUE_FAMILY_EXTERNAL, flags));

    return angle::Result::Continue;
}

}  // namespace rx
