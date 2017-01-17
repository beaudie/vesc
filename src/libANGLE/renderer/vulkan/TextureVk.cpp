//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureVk.cpp:
//    Implements the class methods for TextureVk.
//

#include "libANGLE/renderer/vulkan/TextureVk.h"

#include "common/BitSetIterator.h"
#include "common/debug.h"
//#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

namespace
{

VkSamplerAddressMode ConvertWrapMode(GLenum mode)
{
    switch (mode)
    {
        case GL_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case GL_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GL_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            UNREACHABLE();
    }
}

VkCompareOp ConvertCompareFunc(GLenum func)
{
    switch (func)
    {
        case GL_LEQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GL_GEQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case GL_LESS:
            return VK_COMPARE_OP_LESS;
        case GL_GREATER:
            return VK_COMPARE_OP_GREATER;
        case GL_EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case GL_NOTEQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case GL_ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        case GL_NEVER:
            return VK_COMPARE_OP_NEVER;
        default:
            UNREACHABLE();
    }
}

void ConvertSampler(const gl::SamplerState &state, VkSamplerCreateInfo &createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0u;

    switch (state.magFilter)
    {
        case GL_NEAREST:
            createInfo.magFilter = VK_FILTER_NEAREST;
            break;
        case GL_LINEAR:
            createInfo.magFilter = VK_FILTER_LINEAR;
            break;
        default:
            UNREACHABLE();
    }

    switch (state.minFilter)
    {
        case GL_NEAREST:
        case GL_NEAREST_MIPMAP_NEAREST:
            createInfo.minFilter  = VK_FILTER_NEAREST;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
            createInfo.minFilter  = VK_FILTER_LINEAR;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case GL_NEAREST_MIPMAP_LINEAR:
            createInfo.minFilter  = VK_FILTER_NEAREST;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case GL_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            createInfo.minFilter  = VK_FILTER_LINEAR;
            createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        default:
            UNREACHABLE();
    }

    createInfo.addressModeU = ConvertWrapMode(state.wrapS);
    createInfo.addressModeV = ConvertWrapMode(state.wrapT);
    createInfo.addressModeW = ConvertWrapMode(state.wrapR);

    createInfo.mipLodBias = 0.0f;

    // TODO(Jie): Figure out the EXT_texture_filter_anisotropic
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy    = state.maxAnisotropy;

    createInfo.compareEnable = (state.compareMode == GL_NONE ? VK_FALSE : VK_TRUE);
    createInfo.compareOp     = ConvertCompareFunc(state.compareFunc);

    createInfo.minLod = state.minLod;
    createInfo.maxLod = state.maxLod;

    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

    createInfo.unnormalizedCoordinates = VK_FALSE;
}

VkComponentSwizzle ConvertSwizzle(GLenum swizzle)
{
    switch (swizzle)
    {
        case GL_RED:
            return VK_COMPONENT_SWIZZLE_R;
        case GL_GREEN:
            return VK_COMPONENT_SWIZZLE_G;
        case GL_BLUE:
            return VK_COMPONENT_SWIZZLE_B;
        case GL_ALPHA:
            return VK_COMPONENT_SWIZZLE_A;
        case GL_ZERO:
            return VK_COMPONENT_SWIZZLE_ZERO;
        case GL_ONE:
            return VK_COMPONENT_SWIZZLE_ONE;
        default:
            UNREACHABLE();
    }
}

}  // anonymous namespace

TextureVk::TextureVk(ContextImpl *context, const gl::TextureState &state)
    : TextureImpl(state), mStagingImage(nullptr), mSamplerDirty(true), mImageViewDirty(true)
{
    mContext = GetAs<ContextVk>(context);
}

TextureVk::~TextureVk()
{
    if (mStagingImage != nullptr)
        delete mStagingImage;
}

gl::Error TextureVk::setImage(GLenum target,
                              size_t level,
                              GLenum internalFormat,
                              const gl::Extents &size,
                              GLenum format,
                              GLenum type,
                              const gl::PixelUnpackState &unpack,
                              const uint8_t *pixels)
{
    // TODO(Jie): More targets and levels
    ASSERT(target == GL_TEXTURE_2D && level == 0);

    mFormat = vk::Format::Get(internalFormat).native;
    if (mStagingImage != nullptr)
        delete mStagingImage;
    auto device   = mContext->getDevice();
    mStagingImage = new vk::StagingImage(device);

    VkImageCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext         = nullptr;
    createInfo.flags         = 0;
    createInfo.imageType     = VK_IMAGE_TYPE_2D;
    createInfo.format        = mFormat;
    createInfo.extent.width  = static_cast<uint32_t>(size.width);
    createInfo.extent.height = static_cast<uint32_t>(size.height);
    createInfo.extent.depth  = static_cast<uint32_t>(size.depth);
    createInfo.mipLevels     = 1;
    createInfo.arrayLayers   = 1;
    createInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling        = VK_IMAGE_TILING_LINEAR;
    createInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    ANGLE_TRY(mStagingImage->init(createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                  mContext->getRenderer()->getPhysicalDevice()));

    VkImageSubresource subResource;
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subResource.mipLevel   = level;
    subResource.arrayLayer = 0;

    ANGLE_TRY(mStagingImage->unpackPixels(subResource, size, unpack, pixels));

    mImageViewDirty = true;

    return gl::NoError();
}

gl::Error TextureVk::setSubImage(GLenum target,
                                 size_t level,
                                 const gl::Box &area,
                                 GLenum format,
                                 GLenum type,
                                 const gl::PixelUnpackState &unpack,
                                 const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::setCompressedImage(GLenum target,
                                        size_t level,
                                        GLenum internalFormat,
                                        const gl::Extents &size,
                                        const gl::PixelUnpackState &unpack,
                                        size_t imageSize,
                                        const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::setCompressedSubImage(GLenum target,
                                           size_t level,
                                           const gl::Box &area,
                                           GLenum format,
                                           const gl::PixelUnpackState &unpack,
                                           size_t imageSize,
                                           const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::copyImage(GLenum target,
                               size_t level,
                               const gl::Rectangle &sourceArea,
                               GLenum internalFormat,
                               const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::copySubImage(GLenum target,
                                  size_t level,
                                  const gl::Offset &destOffset,
                                  const gl::Rectangle &sourceArea,
                                  const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::setStorage(GLenum target,
                                size_t levels,
                                GLenum internalFormat,
                                const gl::Extents &size)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::setEGLImageTarget(GLenum target, egl::Image *image)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::setImageExternal(GLenum target,
                                      egl::Stream *stream,
                                      const egl::Stream::GLTextureDescription &desc)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureVk::generateMipmap()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

void TextureVk::setBaseLevel(GLuint baseLevel)
{
    UNIMPLEMENTED();
}

void TextureVk::bindTexImage(egl::Surface *surface)
{
    UNIMPLEMENTED();
}

void TextureVk::releaseTexImage()
{
    UNIMPLEMENTED();
}

gl::Error TextureVk::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                               FramebufferAttachmentRenderTarget **rtOut)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

void TextureVk::syncState(const gl::Texture::DirtyBits &dirtyBits)
{
    if (dirtyBits.none())
        return;

    for (auto dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        switch (dirtyBit)
        {
            case gl::Texture::DIRTY_BIT_MIN_FILTER:
            case gl::Texture::DIRTY_BIT_MAG_FILTER:
            case gl::Texture::DIRTY_BIT_WRAP_S:
            case gl::Texture::DIRTY_BIT_WRAP_T:
            case gl::Texture::DIRTY_BIT_WRAP_R:
            case gl::Texture::DIRTY_BIT_MAX_ANISOTROPY:
            case gl::Texture::DIRTY_BIT_MIN_LOD:
            case gl::Texture::DIRTY_BIT_MAX_LOD:
            case gl::Texture::DIRTY_BIT_COMPARE_MODE:
            case gl::Texture::DIRTY_BIT_COMPARE_FUNC:
            // TODO(Jie): Vulkan srgb decode
            case gl::Texture::DIRTY_BIT_SRGB_DECODE:
                mSamplerDirty = true;
                break;

            case gl::Texture::DIRTY_BIT_SWIZZLE_RED:
            case gl::Texture::DIRTY_BIT_SWIZZLE_GREEN:
            case gl::Texture::DIRTY_BIT_SWIZZLE_BLUE:
            case gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA:
                mImageViewDirty = true;
                break;

            case gl::Texture::DIRTY_BIT_BASE_LEVEL:
            case gl::Texture::DIRTY_BIT_MAX_LEVEL:
                // TODO(Jie): Sort out the impacts of base and max level.
                break;
            case gl::Texture::DIRTY_BIT_USAGE:
                break;

            default:
                UNREACHABLE();
        }
    }
}

gl::ErrorOrResult<VkDescriptorImageInfo *> TextureVk::getDescriptorImageInfo()
{

    auto device = mContext->getDevice();
    if (mSamplerDirty)
    {
        VkSamplerCreateInfo samplerCreateInfo;
        ConvertSampler(mState.getSamplerState(), samplerCreateInfo);
        vk::Sampler sampler(device);
        ANGLE_TRY(sampler.init(samplerCreateInfo));
        mSampler      = std::move(sampler);
        mSamplerDirty = false;
    }
    if (mImageViewDirty)
    {
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext        = nullptr;
        imageViewCreateInfo.image        = mStagingImage->getImage().getHandle();
        imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format       = mFormat;
        imageViewCreateInfo.components.r = ConvertSwizzle(mState.getSwizzleState().swizzleRed);
        imageViewCreateInfo.components.g = ConvertSwizzle(mState.getSwizzleState().swizzleGreen);
        imageViewCreateInfo.components.b = ConvertSwizzle(mState.getSwizzleState().swizzleBlue);
        imageViewCreateInfo.components.a = ConvertSwizzle(mState.getSwizzleState().swizzleAlpha);
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        vk::ImageView imageView(device);
        ANGLE_TRY(imageView.init(imageViewCreateInfo));
        mImageView      = std::move(imageView);
        mImageViewDirty = false;
    }

    mImageInfo.sampler     = mSampler.getHandle();
    mImageInfo.imageView   = mImageView.getHandle();
    mImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    return &mImageInfo;
}

}  // namespace rx
