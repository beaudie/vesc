//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VkImageImageSiblingVk.cpp: Implements VkImageImageSiblingVk.

#include "libANGLE/renderer/vulkan/VkImageImageSiblingVk.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

VkImageImageSiblingVk::VkImageImageSiblingVk(EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
    : mVkImage(reinterpret_cast<VkImage>(buffer))
{
    ASSERT(attribs.contains(EGL_VULKAN_IMAGE_CREATE_INFO_ANGLE));
    const VkImageCreateInfo *info =
        reinterpret_cast<VkImageCreateInfo *>(attribs.contains(EGL_VULKAN_IMAGE_CREATE_INFO_ANGLE));
    ASSERT(info->sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    mVkImageInfo = *info;
    // TODO(penghuang): support extensions.
    mVkImageInfo.pNext = nullptr;
}

VkImageImageSiblingVk::~VkImageImageSiblingVk() = default;

egl::Error VkImageImageSiblingVk::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), displayVk, EGL_BAD_PARAMETER);
}

angle::Result VkImageImageSiblingVk::initImpl(DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();

    const angle::FormatID intendedFormatID = vk::GetFormatIDFromVkFormat(mVkImageInfo.format);
    const vk::Format &vkFormat             = renderer->getFormat(intendedFormatID);
    const angle::FormatID actualImageFormatID =
        vkFormat.getActualImageFormatID(rx::vk::ImageAccess::SampleOnly);
    // mFormat = gl::Format(internalFormat);

    constexpr uint32_t kColorRenderableRequiredBits        = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    constexpr uint32_t kDepthStencilRenderableRequiredBits = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    mRenderable = renderer->hasImageFormatFeatureBits(vkFormat.getActualRenderableImageFormatID(),
                                                      kColorRenderableRequiredBits) ||
                  renderer->hasImageFormatFeatureBits(vkFormat.getActualRenderableImageFormatID(),
                                                      kDepthStencilRenderableRequiredBits);

    constexpr uint32_t kTextureableRequiredBits =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    mTextureable = renderer->hasImageFormatFeatureBits(vkFormat.getActualRenderableImageFormatID(),
                                                       kTextureableRequiredBits);
    // Create the image
    mImage = new vk::ImageHelper();

    constexpr bool kIsRobustInitEnabled = false;
    mImage->init2DWeakReference(displayVk, mVkImage, getSize(), false, intendedFormatID,
                                actualImageFormatID, 1, kIsRobustInitEnabled);

    return angle::Result::Continue;
}

void VkImageImageSiblingVk::onDestroy(const egl::Display *display)
{
    ASSERT(mImage == nullptr);
}

gl::Format VkImageImageSiblingVk::getFormat() const
{
    return mFormat;
}

bool VkImageImageSiblingVk::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool VkImageImageSiblingVk::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

gl::Extents VkImageImageSiblingVk::getSize() const
{
    return gl::Extents(mVkImageInfo.extent.width, mVkImageInfo.extent.height,
                       mVkImageInfo.extent.depth);
}

size_t VkImageImageSiblingVk::getSamples() const
{
    return static_cast<size_t>(mVkImageInfo.samples);
}

// ExternalImageSiblingVk interface
vk::ImageHelper *VkImageImageSiblingVk::getImage() const
{
    return mImage;
}

void VkImageImageSiblingVk::release(RendererVk *renderer)
{
    if (mImage != nullptr)
    {
        // TODO: Handle the case where the EGLImage is used in two contexts not in the same share
        // group.  https://issuetracker.google.com/169868803
        mImage->releaseImage(renderer);
        mImage->releaseStagingBuffer(renderer);
        SafeDelete(mImage);
    }
}

}  // namespace rx
