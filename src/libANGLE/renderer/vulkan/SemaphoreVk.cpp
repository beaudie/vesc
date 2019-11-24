// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SemaphoreVk.cpp: Defines the class interface for SemaphoreVk, implementing
// SemaphoreImpl.

#include "libANGLE/renderer/vulkan/SemaphoreVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

namespace
{
vk::ImageLayout GetVulkanImageLayout(GLenum layout)
{
    switch (enum)
    {
        case GL_NONE:
            return vk::ImageLayout::Undefined;
        case GL_LAYOUT_GENERAL_EXT:
            return vk::ImageLayout::ExternalShadersWrite;
        case GL_LAYOUT_COLOR_ATTACHMENT_EXT:
            return vk::ImageLayout::ColorAttachment;
        case GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT:
        case GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT:
        case GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT:
        case GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT:
            // Note: once VK_KHR_separate_depth_stencil_layouts becomes core or ubiquitous, we
            // should optimize depth/stencil image layout transitions to only be performed on the
            // aspect that needs transition.  In that case, these four layouts can be distinguished
            // and optimized.  Note that the exact equivalent of these layouts are specified in
            // VK_KHR_maintenance2, which are also usable, granted we transition the pair of
            // depth/stencil layouts accordingly elsewhere in ANGLE.
            return vk::ImageLayout::DepthStencilAttachment;
        case GL_LAYOUT_SHADER_READ_ONLY_EXT:
            return vk::ImageLayout::ExternalShadersReadOnly;
        case GL_LAYOUT_TRANSFER_SRC_EXT:
            return vk::ImageLayout::TransferSrc;
        case GL_LAYOUT_TRANSFER_DST_EXT:
            return vk::ImageLayout::TransferDst;
        default:
            UNREACHABLE();
            return vk::ImageLayout::Undefined;
    }
}  // anonymous namespace

SemaphoreVk::SemaphoreVk() = default;

SemaphoreVk::~SemaphoreVk() = default;

void SemaphoreVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->addGarbage(&mSemaphore);
}

angle::Result SemaphoreVk::importFd(gl::Context *context, gl::HandleType handleType, GLint fd)
{
    switch (handleType)
    {
        case gl::HandleType::OpaqueFd:
            return importOpaqueFd(context, fd);

        default:
            ANGLE_VK_UNREACHABLE(vk::GetImpl(context));
            return angle::Result::Stop;
    }
}

angle::Result SemaphoreVk::wait(gl::Context *context,
                                const gl::BufferBarrierVector &bufferBarriers,
                                const gl::TextureBarrierVector &textureBarriers)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (!bufferBarriers.empty())
    {
        // Create one global memory barrier to cover all buffer barriers.
        contextVk->getCommandGraph()->syncExternalMemoryPreFrame();
    }

    if (!textureBarriers.empty())
    {
        for (const gl::TextureAndLayout &textureAndLayout : textureBarriers)
        {
            TextureVk *textureVk   = vk::GetImpl(textureAndLayout.texture);
            vk::ImageLayout layout = GetVulkanImageLayout(textureAndLayout.layout);

            textureVk->getImage().onExternalLayoutChange(layout);
        }
    }

    contextVk->insertWaitSemaphore(&mSemaphore);
    return angle::Result::Continue;
}

angle::Result SemaphoreVk::signal(gl::Context *context,
                                  const gl::BufferBarrierVector &bufferBarriers,
                                  const gl::TextureBarrierVector &textureBarriers)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (!textureBarriers.empty())
    {
        for (const gl::TextureAndLayout &textureAndLayout : textureBarriers)
        {
            TextureVk *textureVk   = vk::GetImpl(textureAndLayout.texture);
            vk::ImageLayout layout = GetVulkanImageLayout(textureAndLayout.layout);

            // Don't transition to Undefined layout.  If external wants to transition the image away
            // from Undefined after this operation, it's perfectly fine to keep the layout as is in
            // ANGLE.  Note that vk::ImageHelper doesn't expect transitions to Undefined.
            if (layout != vk::ImageLayout::Undefined)
            {
                vk::ImageHelper &image = textureVk->getImage();

                vk::CommandBuffer *layoutChange;
                ANGLE_TRY(image.recordCommands(contextVk, &layoutChange));

                image.changeLayout(image.getAspectFlags(), layout, layoutChange);
            }
        }
    }

    // Note: bufferBarriers are handled after textureBarriers, to avoid creating small
    // single-image-layout-transition nodes after the graph barrier inserted here.  Do not reorder.
    if (!bufferBarriers.empty())
    {
        contextVk->getCommandGraph()->syncExternalMemoryPostFrame();
    }

    return contextVk->flushImpl(&mSemaphore);
}

angle::Result SemaphoreVk::importOpaqueFd(gl::Context *context, GLint fd)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    if (!mSemaphore.valid())
    {
        mSemaphore.init(renderer->getDevice());
    }

    ASSERT(mSemaphore.valid());

    VkImportSemaphoreFdInfoKHR importSemaphoreFdInfo = {};
    importSemaphoreFdInfo.sType      = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    importSemaphoreFdInfo.semaphore  = mSemaphore.getHandle();
    importSemaphoreFdInfo.flags      = 0;
    importSemaphoreFdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    importSemaphoreFdInfo.fd         = fd;

    ANGLE_VK_TRY(contextVk, vkImportSemaphoreFdKHR(renderer->getDevice(), &importSemaphoreFdInfo));

    return angle::Result::Continue;
}

}  // namespace
