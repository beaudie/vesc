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
StagingStorage::StagingStorage()
    : mStagingBuffer(kStagingBufferFlags, kStagingBufferSize),
      mFormat(nullptr),
      mCurrentBufferHandle(VK_NULL_HANDLE)
{
    mStagingBuffer.init(1);
}

StagingStorage::~StagingStorage()
{
}

void StagingStorage::release(RendererVk *renderer)
{
    mStagingBuffer.release(renderer);
}

void StagingStorage::initSizeAndFormat(const gl::Extents &extents, const vk::Format &format)
{
    mExtents = extents;
    mFormat  = &format;
}

const gl::Extents &StagingStorage::getExtents() const
{
    return mExtents;
}

const vk::Format &StagingStorage::getFormat() const
{
    return *mFormat;
}

gl::Error StagingStorage::stageSubImage(ContextVk *contextVk,
                                        const gl::InternalFormat &formatInfo,
                                        const gl::PixelUnpackState &unpack,
                                        GLenum type,
                                        const uint8_t *pixels)
{
    GLuint inputRowPitch = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeRowPitch(type, mExtents.width, unpack.alignment, unpack.rowLength),
        inputRowPitch);

    GLuint inputDepthPitch = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeDepthPitch(mExtents.height, unpack.imageHeight, inputRowPitch),
        inputDepthPitch);

    // TODO(jmadill): skip images for 3D Textures.
    bool applySkipImages = false;

    GLuint inputSkipBytes = 0;
    ANGLE_TRY_RESULT(
        formatInfo.computeSkipBytes(inputRowPitch, inputDepthPitch, unpack, applySkipImages),
        inputSkipBytes);

    const gl::InternalFormat &storageFormat =
        gl::GetSizedInternalFormatInfo(mFormat->textureFormat().glInternalFormat);
    GLuint outputRowPitch = 0;
    ANGLE_TRY_RESULT(storageFormat.computeRowPitch(storageFormat.type, mExtents.width,
                                                   unpack.alignment, unpack.rowLength),
                     outputRowPitch);

    GLuint outputDepthPitch = 0;
    ANGLE_TRY_RESULT(
        storageFormat.computeDepthPitch(mExtents.height, unpack.imageHeight, outputRowPitch),
        outputDepthPitch);

    uint8_t *stagingPointer = nullptr;
    bool newBufferAllocated = false;
    uint32_t stagingOffset  = 0;
    size_t allocationSize   = outputDepthPitch * mExtents.depth;
    mStagingBuffer.allocate(contextVk, allocationSize, &stagingPointer, &mCurrentBufferHandle,
                            &stagingOffset, &newBufferAllocated);

    const uint8_t *source = pixels + inputSkipBytes;

    auto loadFunction = mFormat->loadFunctions(type);

    loadFunction.loadFunction(mExtents.width, mExtents.height, mExtents.depth, source,
                              inputRowPitch, inputDepthPitch, stagingPointer, outputRowPitch,
                              outputDepthPitch);

    GLuint pixelBytes = storageFormat.computePixelBytes(storageFormat.type);

    mCurrentCopyRegion.bufferOffset                    = static_cast<VkDeviceSize>(stagingOffset);
    mCurrentCopyRegion.bufferRowLength                 = outputRowPitch / pixelBytes;
    mCurrentCopyRegion.bufferImageHeight               = outputDepthPitch / outputRowPitch;
    mCurrentCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    mCurrentCopyRegion.imageSubresource.mipLevel       = 0;
    mCurrentCopyRegion.imageSubresource.baseArrayLayer = 0;
    mCurrentCopyRegion.imageSubresource.layerCount     = 1;

    gl_vk::GetOffset(gl::Offset(), &mCurrentCopyRegion.imageOffset);
    gl_vk::GetExtent(mExtents, &mCurrentCopyRegion.imageExtent);

    return gl::NoError();
}

vk::Error StagingStorage::flushToImage(RendererVk *renderer,
                                       const vk::ImageHelper &image,
                                       vk::CommandBuffer *commandBuffer)
{
    if (mCurrentBufferHandle != VK_NULL_HANDLE)
    {
        ANGLE_TRY(mStagingBuffer.flush(renderer->getDevice()));
        commandBuffer->copyBufferToImage(mCurrentBufferHandle, image.getImage(),
                                         image.getCurrentLayout(), 1, &mCurrentCopyRegion);
        mCurrentBufferHandle = VK_NULL_HANDLE;
    }

    return vk::NoError();
}

// TextureVk implementation.
TextureVk::TextureVk(const gl::TextureState &state) : TextureImpl(state)
{
    mRenderTarget.image     = &mImage;
    mRenderTarget.imageView = &mImageView;
    mRenderTarget.resource  = this;
}

TextureVk::~TextureVk()
{
}

gl::Error TextureVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    mImage.release(renderer->getCurrentQueueSerial(), renderer);

    renderer->releaseResource(*this, &mImageView);
    renderer->releaseResource(*this, &mSampler);

    mStagingStorage.release(renderer);

    onStateChange(context, angle::SubjectMessage::DEPENDENT_DIRTY_BITS);

    return gl::NoError();
}

gl::Error TextureVk::setImage(const gl::Context *context,
                              gl::TextureTarget target,
                              size_t level,
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
    ASSERT(level == 0);

    if (mImage.valid())
    {
        const gl::ImageDesc &desc = mState.getImageDesc(target, level);

        // TODO(jmadill): Consider comparing stored vk::Format.
        if (desc.size != size ||
            !gl::Format::SameSized(desc.format, gl::Format(internalFormat, type)))
        {
            mImage.release(renderer->getCurrentQueueSerial(), renderer);
            renderer->releaseResource(*this, &mImageView);

            onStateChange(context, angle::SubjectMessage::DEPENDENT_DIRTY_BITS);
        }
    }

    // Early-out on empty textures, don't create a zero-sized storage.
    if (size.empty())
    {
        return gl::NoError();
    }

    // TODO(jmadill): Cube map textures. http://anglebug.com/2318
    if (target != gl::TextureTarget::_2D)
    {
        UNIMPLEMENTED();
        return gl::InternalError();
    }

    if (!mSampler.valid())
    {
        // Create a simple sampler. Force basic parameter settings.
        // TODO(jmadill): Sampler parameters.
        VkSamplerCreateInfo samplerInfo;
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext                   = nullptr;
        samplerInfo.flags                   = 0;
        samplerInfo.magFilter               = VK_FILTER_NEAREST;
        samplerInfo.minFilter               = VK_FILTER_NEAREST;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = 1.0f;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 1.0f;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        ANGLE_TRY(mSampler.init(device, samplerInfo));
    }

    // Convert internalFormat to sized internal format.
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat, type);
    mStagingStorage.initSizeAndFormat(size, renderer->getFormat(formatInfo.sizedInternalFormat));

    // Create a new graph node to store image initialization commands.
    getNewWritingNode(renderer);

    // Handle initial data.
    if (pixels)
    {
        ANGLE_TRY(mStagingStorage.stageSubImage(contextVk, formatInfo, unpack, type, pixels));
    }

    return gl::NoError();
}

gl::Error TextureVk::setSubImage(const gl::Context *context,
                                 gl::TextureTarget target,
                                 size_t level,
                                 const gl::Box &area,
                                 GLenum format,
                                 GLenum type,
                                 const gl::PixelUnpackState &unpack,
                                 const uint8_t *pixels)
{
    ContextVk *contextVk                 = vk::GetImpl(context);
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, type);
    ANGLE_TRY(mStagingStorage.stageSubImage(contextVk, formatInfo, unpack, type, pixels));
    return gl::NoError();
}

gl::Error TextureVk::setCompressedImage(const gl::Context *context,
                                        gl::TextureTarget target,
                                        size_t level,
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
                                           gl::TextureTarget target,
                                           size_t level,
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
                               gl::TextureTarget target,
                               size_t level,
                               const gl::Rectangle &sourceArea,
                               GLenum internalFormat,
                               gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error TextureVk::copySubImage(const gl::Context *context,
                                  gl::TextureTarget target,
                                  size_t level,
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
    ASSERT(imageIndex.type == gl::TextureType::_2D);

    // Non-zero mip level attachments are an ES 3.0 feature.
    ASSERT(imageIndex.mipIndex == 0 && imageIndex.layerIndex == gl::ImageIndex::ENTIRE_LEVEL);

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
        const gl::Extents &extents = mStagingStorage.getExtents();
        const vk::Format &format   = mStagingStorage.getFormat();

        VkImageUsageFlags usage =
            (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        ANGLE_TRY(mImage.init2D(device, extents, format, 1, usage));

        VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        ANGLE_TRY(mImage.initMemory(device, renderer->getMemoryProperties(), flags));

        gl::SwizzleState mappedSwizzle;
        MapSwizzleState(format.internalFormat, mState.getSwizzleState(), &mappedSwizzle);

        ANGLE_TRY(
            mImage.initImageView(device, VK_IMAGE_ASPECT_COLOR_BIT, mappedSwizzle, &mImageView));

        // TODO(jmadill): Fold this into the RenderPass load/store ops. http://anglebug.com/2361

        VkClearColorValue black = {{0}};
        mImage.clearColor(black, commandBuffer);
    }

    ANGLE_TRY(mStagingStorage.flushToImage(renderer, mImage, commandBuffer));
    return vk::NoError();
}

void TextureVk::syncState(const gl::Texture::DirtyBits &dirtyBits)
{
    // TODO(jmadill): Texture sync state.
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

}  // namespace rx
