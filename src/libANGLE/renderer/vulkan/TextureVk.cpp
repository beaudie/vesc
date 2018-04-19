//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureVk.cpp:
//    Implements the class methods for TextureVk.
//

#include "libANGLE/renderer/vulkan/TextureVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"

namespace rx
{
namespace
{
VkFilter GetVkFilter(const GLenum filter)
{
    switch (filter)
    {
        case GL_LINEAR:
            return VK_FILTER_LINEAR;
        case GL_NEAREST:
            return VK_FILTER_NEAREST;
        default:
            UNIMPLEMENTED();
            return VK_FILTER_MAX_ENUM;
    }
}

VkSamplerAddressMode GetVkSamplerAddressMode(const GLenum wrap)
{
    switch (wrap)
    {
        case GL_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GL_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case GL_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case GL_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:
            UNIMPLEMENTED();
            return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }
}

void MapSwizzleState(GLenum internalFormat,
                     const gl::SwizzleState &swizzleState,
                     gl::SwizzleState *swizzleStateOut)
{
    switch (internalFormat)
    {
        case GL_LUMINANCE8_OES:
            swizzleStateOut->swizzleRed   = swizzleState.swizzleRed;
            swizzleStateOut->swizzleGreen = swizzleState.swizzleRed;
            swizzleStateOut->swizzleBlue  = swizzleState.swizzleRed;
            swizzleStateOut->swizzleAlpha = GL_ONE;
            break;
        case GL_LUMINANCE8_ALPHA8_OES:
            swizzleStateOut->swizzleRed   = swizzleState.swizzleRed;
            swizzleStateOut->swizzleGreen = swizzleState.swizzleRed;
            swizzleStateOut->swizzleBlue  = swizzleState.swizzleRed;
            swizzleStateOut->swizzleAlpha = swizzleState.swizzleGreen;
            break;
        case GL_ALPHA8_OES:
            swizzleStateOut->swizzleRed   = GL_ZERO;
            swizzleStateOut->swizzleGreen = GL_ZERO;
            swizzleStateOut->swizzleBlue  = GL_ZERO;
            swizzleStateOut->swizzleAlpha = swizzleState.swizzleRed;
            break;
        default:
            *swizzleStateOut = swizzleState;
            break;
    }
}

constexpr VkBufferUsageFlags kStagingBufferFlags =
    (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
constexpr size_t kStagingBufferSize = 1024 * 16;
}  // anonymous namespace

// StagingStorage implementation.
PixelBuffer::PixelBuffer() : mStagingBuffer(kStagingBufferFlags, kStagingBufferSize)
{
    // vkCmdCopyBufferToImage must have an offset that is a multiple of 4.
    // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBufferImageCopy.html
    mStagingBuffer.init(4);
}

PixelBuffer::~PixelBuffer()
{
}

void PixelBuffer::release(RendererVk *renderer)
{
    mStagingBuffer.release(renderer);
}

gl::Error PixelBuffer::stageSubresourceUpdate(ContextVk *contextVk,
                                              const gl::ImageIndex &index,
                                              const gl::Extents &extents,
                                              const gl::InternalFormat &formatInfo,
                                              const gl::PixelUnpackState &unpack,
                                              GLenum type,
                                              const uint8_t *pixels)
{
    GLuint inputRowPitch = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeRowPitch(type, extents.width, unpack.alignment, unpack.rowLength),
        inputRowPitch);

    GLuint inputDepthPitch = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeDepthPitch(extents.height, unpack.imageHeight, inputRowPitch),
        inputDepthPitch);

    // TODO(jmadill): skip images for 3D Textures.
    bool applySkipImages = false;

    GLuint inputSkipBytes = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeSkipBytes(inputRowPitch, inputDepthPitch, unpack, applySkipImages),
        inputSkipBytes);

    RendererVk *renderer = contextVk->getRenderer();

    const vk::Format &vkFormat         = renderer->getFormat(formatInfo.sizedInternalFormat);
    const angle::Format &storageFormat = vkFormat.textureFormat();

    size_t outputRowPitch   = storageFormat.pixelBytes * extents.width;
    size_t outputDepthPitch = outputRowPitch * extents.height;

    VkBuffer bufferHandle = VK_NULL_HANDLE;

    uint8_t *stagingPointer = nullptr;
    bool newBufferAllocated = false;
    uint32_t stagingOffset  = 0;
    size_t allocationSize   = outputDepthPitch * extents.depth;
    mStagingBuffer.allocate(renderer, allocationSize, &stagingPointer, &bufferHandle,
                            &stagingOffset, &newBufferAllocated);

    const uint8_t *source = pixels + inputSkipBytes;

    LoadImageFunctionInfo loadFunction = vkFormat.loadFunctions(type);

    loadFunction.loadFunction(extents.width, extents.height, extents.depth, source, inputRowPitch,
                              inputDepthPitch, stagingPointer, outputRowPitch, outputDepthPitch);

    VkBufferImageCopy copy;

    copy.bufferOffset                    = static_cast<VkDeviceSize>(stagingOffset);
    copy.bufferRowLength                 = extents.width;
    copy.bufferImageHeight               = extents.height;
    copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.mipLevel       = index.getLevelIndex();
    copy.imageSubresource.baseArrayLayer = index.hasLayer() ? index.getLayerIndex() : 0;
    copy.imageSubresource.layerCount     = index.getLayerCount();

    gl_vk::GetOffset(gl::Offset(), &copy.imageOffset);
    gl_vk::GetExtent(extents, &copy.imageExtent);

    mSubresourceUpdates.emplace_back(bufferHandle, copy);

    return gl::NoError();
}

vk::Error PixelBuffer::flushUpdatesToImage(RendererVk *renderer,
                                           vk::ImageHelper *image,
                                           vk::CommandBuffer *commandBuffer)
{
    if (mSubresourceUpdates.empty())
    {
        return vk::NoError();
    }

    // Conservatively flush all writes to the image. We could use a more restricted barrier.
    image->changeLayoutWithStages(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, commandBuffer);

    ANGLE_TRY(mStagingBuffer.flush(renderer->getDevice()));

    for (const SubresourceUpdate &update : mSubresourceUpdates)
    {
        ASSERT(update.bufferHandle != VK_NULL_HANDLE);
        commandBuffer->copyBufferToImage(update.bufferHandle, image->getImage(),
                                         image->getCurrentLayout(), 1, &update.copyRegion);
    }

    mSubresourceUpdates.clear();

    return vk::NoError();
}

PixelBuffer::SubresourceUpdate::SubresourceUpdate() : bufferHandle(VK_NULL_HANDLE)
{
}

PixelBuffer::SubresourceUpdate::SubresourceUpdate(VkBuffer bufferHandleIn,
                                                  const VkBufferImageCopy &copyRegionIn)
    : bufferHandle(bufferHandleIn), copyRegion(copyRegionIn)
{
}

PixelBuffer::SubresourceUpdate::SubresourceUpdate(const SubresourceUpdate &other) = default;

// TextureVk implementation.
TextureVk::TextureVk(const gl::TextureState &state) : TextureImpl(state)
{
    mRenderTarget.image     = &mImage;
    mRenderTarget.imageView = &mImageView;
    mRenderTarget.resource  = this;

    mAppliedSampler.minFilter = GL_NEAREST;
    mAppliedSampler.magFilter = GL_NEAREST;
    mAppliedSampler.wrapS = GL_CLAMP_TO_EDGE;
    mAppliedSampler.wrapT = GL_CLAMP_TO_EDGE;
    mAppliedSampler.wrapR = GL_CLAMP_TO_EDGE;
}

TextureVk::~TextureVk()
{
}

gl::Error TextureVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    releaseImage(context, renderer);
    renderer->releaseResource(*this, &mSampler);

    mPixelBuffer.release(renderer);

    return gl::NoError();
}

gl::Error TextureVk::syncSampler(VkDevice device)
{
    if (!mSampler.valid() || mIsSamplerDirty)
    {
        if (mSampler.valid())
        {
            // TODO: release instead of destroying.
            mSampler.destroy(device);
        }

        // Create a simple sampler. Force basic parameter settings.
        VkSamplerCreateInfo samplerInfo;
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext                   = nullptr;
        samplerInfo.flags                   = 0;
        samplerInfo.magFilter               = GetVkFilter(mAppliedSampler.magFilter);
        samplerInfo.minFilter               = GetVkFilter(mAppliedSampler.minFilter);
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU            = GetVkSamplerAddressMode(mAppliedSampler.wrapS);
        samplerInfo.addressModeV            = GetVkSamplerAddressMode(mAppliedSampler.wrapT);
        samplerInfo.addressModeW            = GetVkSamplerAddressMode(mAppliedSampler.wrapR);
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = 1.0f;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 1.0f;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        
        mIsSamplerDirty = false;

        ANGLE_TRY(mSampler.init(device, samplerInfo));
    }
    return gl::NoError();
}

gl::Error TextureVk::setImage(const gl::Context *context,
                              const gl::ImageIndex &index,
                              GLenum internalFormat,
                              const gl::Extents &size,
                              GLenum format,
                              GLenum type,
                              const gl::PixelUnpackState &unpack,
                              const uint8_t *pixels)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();
    VkDevice device      = contextVk->getDevice();

    // TODO(jmadill): support multi-level textures.
    if (index.getLevelIndex() != 0)
    {
        UNIMPLEMENTED();
        return gl::InternalError();
    }

    // Convert internalFormat to sized internal format.
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat, type);

    if (mImage.valid())
    {
        const gl::ImageDesc &desc  = mState.getImageDesc(index);
        const vk::Format &vkFormat = renderer->getFormat(formatInfo.sizedInternalFormat);
        if (desc.size != size || mImage.getFormat() != vkFormat)
        {
            releaseImage(context, renderer);
        }
    }

    // Early-out on empty textures, don't create a zero-sized storage.
    if (size.empty())
    {
        return gl::NoError();
    }

    ANGLE_TRY(syncSampler(device));

    // Create a new graph node to store image initialization commands.
    getNewWritingNode(renderer);

    // Handle initial data.
    if (pixels)
    {
        ANGLE_TRY(mPixelBuffer.stageSubresourceUpdate(contextVk, index, size, formatInfo, unpack,
                                                      type, pixels));
    }

    return gl::NoError();
}

gl::Error TextureVk::setSubImage(const gl::Context *context,
                                 const gl::ImageIndex &index,
                                 const gl::Box &area,
                                 GLenum format,
                                 GLenum type,
                                 const gl::PixelUnpackState &unpack,
                                 const uint8_t *pixels)
{
    ContextVk *contextVk                 = vk::GetImpl(context);
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, type);
    ANGLE_TRY(mPixelBuffer.stageSubresourceUpdate(contextVk, index,
                                                  gl::Extents(area.width, area.height, area.depth),
                                                  formatInfo, unpack, type, pixels));
    return gl::NoError();
}

gl::Error TextureVk::setCompressedImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        GLenum internalFormat,
                                        const gl::Extents &size,
                                        const gl::PixelUnpackState &unpack,
                                        size_t imageSize,
                                        const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::setCompressedSubImage(const gl::Context *context,
                                           const gl::ImageIndex &index,
                                           const gl::Box &area,
                                           GLenum format,
                                           const gl::PixelUnpackState &unpack,
                                           size_t imageSize,
                                           const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::copyImage(const gl::Context *context,
                               const gl::ImageIndex &index,
                               const gl::Rectangle &sourceArea,
                               GLenum internalFormat,
                               gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::copySubImage(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  const gl::Offset &destOffset,
                                  const gl::Rectangle &sourceArea,
                                  gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::setStorage(const gl::Context *context,
                                gl::TextureType type,
                                size_t levels,
                                GLenum internalFormat,
                                const gl::Extents &size)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::setEGLImageTarget(const gl::Context *context,
                                       gl::TextureType type,
                                       egl::Image *image)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::setImageExternal(const gl::Context *context,
                                      gl::TextureType type,
                                      egl::Stream *stream,
                                      const egl::Stream::GLTextureDescription &desc)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::generateMipmap(const gl::Context *context)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::releaseTexImage(const gl::Context *context)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::getAttachmentRenderTarget(const gl::Context *context,
                                               GLenum binding,
                                               const gl::ImageIndex &imageIndex,
                                               FramebufferAttachmentRenderTarget **rtOut)
{
    // TODO(jmadill): Handle cube textures. http://anglebug.com/2318
    ASSERT(imageIndex.getType() == gl::TextureType::_2D);

    // Non-zero mip level attachments are an ES 3.0 feature.
    ASSERT(imageIndex.getLevelIndex() == 0 && !imageIndex.hasLayer());

    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    ANGLE_TRY(ensureImageInitialized(renderer));

    *rtOut = &mRenderTarget;
    return gl::NoError();
}

vk::Error TextureVk::ensureImageInitialized(RendererVk *renderer)
{
    VkDevice device                  = renderer->getDevice();
    vk::CommandBuffer *commandBuffer = nullptr;

    updateQueueSerial(renderer->getCurrentQueueSerial());
    if (!hasChildlessWritingNode())
    {
        beginWriteResource(renderer, &commandBuffer);
    }
    else
    {
        vk::CommandGraphNode *node = getCurrentWritingNode();
        commandBuffer              = node->getOutsideRenderPassCommands();
        if (!commandBuffer->valid())
        {
            ANGLE_TRY(node->beginOutsideRenderPassRecording(device, renderer->getCommandPool(),
                                                            &commandBuffer));
        }
    }

    if (!mImage.valid())
    {
        const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
        const gl::Extents &extents         = baseLevelDesc.size;
        const vk::Format &format =
            renderer->getFormat(baseLevelDesc.format.info->sizedInternalFormat);

        VkImageUsageFlags usage =
            (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        ANGLE_TRY(mImage.init(device, mState.getType(), extents, format, 1, usage));

        VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        ANGLE_TRY(mImage.initMemory(device, renderer->getMemoryProperties(), flags));

        gl::SwizzleState mappedSwizzle;
        MapSwizzleState(format.internalFormat, mState.getSwizzleState(), &mappedSwizzle);

        // TODO(jmadill): Separate imageviews for RenderTargets and Sampling.
        ANGLE_TRY(mImage.initImageView(device, mState.getType(), VK_IMAGE_ASPECT_COLOR_BIT,
                                       mappedSwizzle, &mImageView));

        // TODO(jmadill): Fold this into the RenderPass load/store ops. http://anglebug.com/2361

        VkClearColorValue black = {{0}};
        mImage.clearColor(black, commandBuffer);
    }

    ANGLE_TRY(mPixelBuffer.flushUpdatesToImage(renderer, &mImage, commandBuffer));
    return vk::NoError();
}

gl::Error TextureVk::syncState(const gl::Context *context, const gl::Texture::DirtyBits &dirtyBits)
{
    if (dirtyBits.none())
    {
        return gl::NoError();
    }

    for (const size_t dirtyBit : dirtyBits)
    {
        switch(dirtyBit)
        {
            case gl::Texture::DIRTY_BIT_MIN_FILTER:
                mAppliedSampler.minFilter = mState.getSamplerState().minFilter;
                mIsSamplerDirty = true;
                break;
            case gl::Texture::DIRTY_BIT_MAG_FILTER:
                mAppliedSampler.magFilter = mState.getSamplerState().magFilter;
                mIsSamplerDirty = true;
                break;
            case gl::Texture::DIRTY_BIT_WRAP_R:
                mAppliedSampler.wrapR = mState.getSamplerState().wrapR;
                mIsSamplerDirty = true;
                break;
            case gl::Texture::DIRTY_BIT_WRAP_S:
                mAppliedSampler.wrapS = mState.getSamplerState().wrapS;
                mIsSamplerDirty = true;
                break;
            case gl::Texture::DIRTY_BIT_WRAP_T:
                mAppliedSampler.wrapT = mState.getSamplerState().wrapT;
                mIsSamplerDirty = true;
                break;
        }
    }

    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(syncSampler(contextVk->getDevice()));
    return gl::NoError();
}

gl::Error TextureVk::setStorageMultisample(const gl::Context *context,
                                           gl::TextureType type,
                                           GLsizei samples,
                                           GLint internalformat,
                                           const gl::Extents &size,
                                           bool fixedSampleLocations)
{
    UNIMPLEMENTED();
    return gl::InternalError() << "setStorageMultisample is unimplemented.";
}

gl::Error TextureVk::initializeContents(const gl::Context *context,
                                        const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return gl::NoError();
}

const vk::ImageHelper &TextureVk::getImage() const
{
    ASSERT(mImage.valid());
    return mImage;
}

const vk::ImageView &TextureVk::getImageView() const
{
    ASSERT(mImage.valid());
    return mImageView;
}

const vk::Sampler &TextureVk::getSampler() const
{
    ASSERT(mSampler.valid());
    return mSampler;
}

void TextureVk::releaseImage(const gl::Context *context, RendererVk *renderer)
{
    mImage.release(renderer->getCurrentQueueSerial(), renderer);
    renderer->releaseResource(*this, &mImageView);
    onStateChange(context, angle::SubjectMessage::DEPENDENT_DIRTY_BITS);
}

}  // namespace rx
