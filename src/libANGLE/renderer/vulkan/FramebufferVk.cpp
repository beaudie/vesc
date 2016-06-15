//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferVk.cpp:
//    Implements the class methods for FramebufferVk.
//

#include "libANGLE/renderer/vulkan/FramebufferVk.h"

#include <array>
#include <vulkan/vulkan.h>

#include "common/debug.h"
#include "image_util/imageformats.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/Format.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

namespace
{

// FIXME
template <typename sourceType, typename colorDataType>
inline void ReadColor(const uint8_t *source, uint8_t *dest)
{
    sourceType::readColor(reinterpret_cast<gl::Color<colorDataType> *>(dest),
                          reinterpret_cast<const sourceType *>(source));
}

VkSampleCountFlagBits ConvertSamples(GLint sampleCount)
{
    switch (sampleCount)
    {
        case 0:
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        default:
            UNREACHABLE();
            return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    }
}

gl::ErrorOrResult<const gl::InternalFormat *> GetReadAttachmentInfo(
    const gl::FramebufferAttachment *readAttachment)
{
    const RenderTargetVk *renderTarget = nullptr;
    ANGLE_TRY(readAttachment->getRenderTarget(&renderTarget));

    GLenum implFormat = renderTarget->getFormat().format.fboImplementationInternalFormat;
    return &gl::GetInternalFormatInfo(implFormat);
}

}  // anonymous namespace

FramebufferVk::FramebufferVk(const gl::FramebufferState &state)
    : FramebufferImpl(state), mDirty(true), mRenderPass(VK_NULL_HANDLE)
{
}

FramebufferVk::~FramebufferVk()
{
}

gl::Error FramebufferVk::discard(size_t count, const GLenum *attachments)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::invalidate(size_t count, const GLenum *attachments)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::invalidateSub(size_t count,
                                       const GLenum *attachments,
                                       const gl::Rectangle &area)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::clear(ContextImpl *context, GLbitfield mask)
{
    ContextVk *contextVk = GetAs<ContextVk>(context);

    if (mState.getDepthAttachment() && (mask & GL_DEPTH_BUFFER_BIT) != 0)
    {
        // TODO(jmadill): Depth clear
        UNIMPLEMENTED();
    }

    if (mState.getStencilAttachment() && (mask & GL_STENCIL_BUFFER_BIT) != 0)
    {
        // TODO(jmadill): Stencil clear
        UNIMPLEMENTED();
    }

    if ((mask & GL_COLOR_BUFFER_BIT) == 0)
    {
        return gl::NoError();
    }

    const auto &glState    = context->getGLState();
    const auto &clearColor = glState.getColorClearValue();
    VkClearColorValue clearColorValue;
    clearColorValue.float32[0] = clearColor.red;
    clearColorValue.float32[1] = clearColor.green;
    clearColorValue.float32[2] = clearColor.blue;
    clearColorValue.float32[3] = clearColor.alpha;

    // TODO(jmadill): Scissored clears.
    const auto *attachment = mState.getFirstNonNullAttachment();
    ASSERT(attachment && attachment->isAttached());
    const auto &size = attachment->getSize();
    const gl::Rectangle renderArea(0, 0, size.width, size.height);

    vk::CommandBuffer *commandBuffer = contextVk->getCommandBuffer();
    ANGLE_TRY(commandBuffer->begin());

    for (const auto &colorAttachment : mState.getColorAttachments())
    {
        if (colorAttachment.isAttached())
        {
            RenderTargetVk *renderTarget = nullptr;
            ANGLE_TRY(colorAttachment.getRenderTarget(&renderTarget));
            commandBuffer->clearSingleColorImage(renderTarget->getImage(), VK_IMAGE_LAYOUT_GENERAL,
                                                 clearColorValue);
        }
    }

    commandBuffer->end();

    ANGLE_TRY(contextVk->submitCommands(*commandBuffer));

    return gl::NoError();
}

gl::Error FramebufferVk::clearBufferfv(ContextImpl *context,
                                       GLenum buffer,
                                       GLint drawbuffer,
                                       const GLfloat *values)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::clearBufferuiv(ContextImpl *context,
                                        GLenum buffer,
                                        GLint drawbuffer,
                                        const GLuint *values)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::clearBufferiv(ContextImpl *context,
                                       GLenum buffer,
                                       GLint drawbuffer,
                                       const GLint *values)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferVk::clearBufferfi(ContextImpl *context,
                                       GLenum buffer,
                                       GLint drawbuffer,
                                       GLfloat depth,
                                       GLint stencil)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

GLenum FramebufferVk::getImplementationColorReadFormat() const
{
    auto errOrResult = GetReadAttachmentInfo(mState.getReadAttachment());

    // TODO(jmadill): Handle getRenderTarget error.
    if (errOrResult.isError())
    {
        ERR("Internal error in FramebufferVk::getImplementationColorReadFormat.");
        return GL_NONE;
    }

    return errOrResult.getResult()->format;
}

GLenum FramebufferVk::getImplementationColorReadType() const
{
    auto errOrResult = GetReadAttachmentInfo(mState.getReadAttachment());

    // TODO(jmadill): Handle getRenderTarget error.
    if (errOrResult.isError())
    {
        ERR("Internal error in FramebufferVk::getImplementationColorReadFormat.");
        return GL_NONE;
    }

    return errOrResult.getResult()->type;
}

gl::Error FramebufferVk::readPixels(ContextImpl *context,
                                    const gl::Rectangle &area,
                                    GLenum format,
                                    GLenum type,
                                    GLvoid *pixels) const
{
    const auto &glState         = context->getGLState();
    const auto *readFramebuffer = glState.getReadFramebuffer();
    const auto *readAttachment  = readFramebuffer->getReadColorbuffer();

    RenderTargetVk *renderTarget = nullptr;
    ANGLE_TRY(readAttachment->getRenderTarget(&renderTarget));

    ContextVk *contextVk = GetAs<ContextVk>(context);
    RendererVk *renderer = contextVk->getRenderer();

    VkImage readImage = renderTarget->getImage();
    StagingImage stagingImage;
    ANGLE_TRY_RESULT(
        renderer->createStagingImage(TextureDimension::TEX_2D, renderTarget->getFormat(),
                                     renderTarget->getExtents()),
        stagingImage);

    vk::CommandBuffer *commandBuffer = contextVk->getCommandBuffer();
    commandBuffer->begin();
    commandBuffer->changeImageLayout(stagingImage.getHandle(), VK_IMAGE_ASPECT_COLOR_BIT,
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    commandBuffer->end();

    ANGLE_TRY(renderer->submitAndFinishCommandBuffer(*commandBuffer, 1000ul));

    gl::Box copyRegion;
    copyRegion.x      = area.x;
    copyRegion.y      = area.y;
    copyRegion.z      = 0;
    copyRegion.width  = area.width;
    copyRegion.height = area.height;
    copyRegion.depth  = 1;

    commandBuffer->begin();
    commandBuffer->copySingleImage(readImage, VK_IMAGE_LAYOUT_GENERAL, stagingImage.getHandle(),
                                   VK_IMAGE_LAYOUT_GENERAL, copyRegion, VK_IMAGE_ASPECT_COLOR_BIT);
    commandBuffer->end();

    ANGLE_TRY(renderer->submitAndFinishCommandBuffer(*commandBuffer, 1000ul));

    // TODO(jmadill): parameters
    uint8_t *mapPointer = nullptr;
    ANGLE_VK_TRY(vkMapMemory(renderer->getDevice(), stagingImage.getMemory(), 0,
                             stagingImage.getSize(), 0, reinterpret_cast<void **>(&mapPointer)));

    const auto &angleFormat = renderTarget->getFormat().format;

    // TODO(jmadill): Use pixel bytes from the ANGLE format directly.
    const auto &glFormat = gl::GetInternalFormatInfo(angleFormat.glInternalFormat);
    int inputPitch       = glFormat.pixelBytes * area.width;

    PackPixelsParams params;
    params.area        = area;
    params.format      = format;
    params.type        = type;
    params.outputPitch = inputPitch;
    params.pack        = glState.getPackState();

    PackPixels(params, renderTarget->getFormat().format, inputPitch, mapPointer,
               reinterpret_cast<uint8_t *>(pixels));

    vkUnmapMemory(renderer->getDevice(), stagingImage.getMemory());

    return vk::VkSuccess();
}

gl::Error FramebufferVk::blit(ContextImpl *context,
                              const gl::Rectangle &sourceArea,
                              const gl::Rectangle &destArea,
                              GLbitfield mask,
                              GLenum filter)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

bool FramebufferVk::checkStatus() const
{
    UNIMPLEMENTED();
    return bool();
}

void FramebufferVk::syncState(const gl::Framebuffer::DirtyBits &dirtyBits)
{
    // TODO(jmadill): Smarter scheme for small changes.
    mDirty = true;
}

vk::Error FramebufferVk::getRenderPass(VkDevice device)
{
    if (!mDirty)
    {
        return vk::VkSuccess();
    }

    // TODO(jmadill): Can we use stack-only memory?
    std::vector<VkAttachmentDescription> attachmentDescs;
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    std::vector<VkImageView> attachments;
    gl::Extents attachmentsSize;

    const auto &colorAttachments = mState.getColorAttachments();
    for (size_t attachmentIndex = 0; attachmentIndex < colorAttachments.size(); ++attachmentIndex)
    {
        const auto &colorAttachment = colorAttachments[attachmentIndex];
        if (colorAttachment.isAttached())
        {
            VkAttachmentDescription colorDesc;
            VkAttachmentReference colorRef;

            RenderTargetVk *renderTarget = nullptr;
            ANGLE_TRY(colorAttachment.getRenderTarget<RenderTargetVk>(&renderTarget));

            // TODO(jmadill): We would only need this flag for duplicated attachments.
            colorDesc.flags   = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
            colorDesc.format  = renderTarget->getFormat();
            colorDesc.samples = ConvertSamples(colorAttachment.getSamples());

            // This flag will preserve the existing depth/color buffer data.
            // TODO(jmadill): Can use a smarter load/store ops?
            // FIXME(jmadill): should not be hard coded to clear.
            colorDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorDesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorDesc.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorDesc.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRef.attachment = static_cast<uint32_t>(colorAttachments.size());
            colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachmentDescs.push_back(colorDesc);
            colorAttachmentRefs.push_back(colorRef);
            attachments.push_back(renderTarget->getImageView());

            ASSERT(attachmentsSize.empty() || attachmentsSize == colorAttachment.getSize());
            attachmentsSize = colorAttachment.getSize();
        }
    }

    const auto *depthStencilAttachment = mState.getDepthStencilAttachment();
    VkAttachmentReference depthStencilAttachmentRef;
    bool useDepth = depthStencilAttachment && depthStencilAttachment->isAttached();

    if (useDepth)
    {
        VkAttachmentDescription depthStencilDesc;

        RenderTargetVk *renderTarget = nullptr;
        ANGLE_TRY(depthStencilAttachment->getRenderTarget<RenderTargetVk>(&renderTarget));

        depthStencilDesc.flags          = 0;
        depthStencilDesc.format         = renderTarget->getFormat();
        depthStencilDesc.samples        = ConvertSamples(depthStencilAttachment->getSamples());
        depthStencilDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthStencilDesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depthStencilDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilDesc.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilDesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depthStencilAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescs.size());
        depthStencilAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachmentDescs.push_back(depthStencilDesc);
        attachments.push_back(renderTarget->getImageView());

        ASSERT(attachmentsSize.empty() || attachmentsSize == depthStencilAttachment->getSize());
        attachmentsSize = depthStencilAttachment->getSize();
    }

    ASSERT(!attachmentDescs.empty() && !attachments.empty());

    VkSubpassDescription subpassDesc;

    subpassDesc.flags                   = 0;
    subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount    = 0;
    subpassDesc.pInputAttachments       = nullptr;
    subpassDesc.colorAttachmentCount    = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpassDesc.pColorAttachments       = colorAttachmentRefs.data();
    subpassDesc.pResolveAttachments     = nullptr;
    subpassDesc.pDepthStencilAttachment = (useDepth ? &depthStencilAttachmentRef : nullptr);
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments    = nullptr;

    VkRenderPassCreateInfo renderPassInfo;

    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext           = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
    renderPassInfo.pAttachments    = attachmentDescs.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDesc;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies   = nullptr;

    vk::RenderPass renderPass(device);
    ANGLE_TRY(renderPass.init(renderPassInfo));

    mRenderPass = std::move(renderPass);

    VkFramebufferCreateInfo framebufferInfo;

    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext           = nullptr;
    framebufferInfo.flags           = 0;
    framebufferInfo.renderPass      = mRenderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = static_cast<uint32_t>(attachmentsSize.width);
    framebufferInfo.height          = static_cast<uint32_t>(attachmentsSize.depth);
    framebufferInfo.layers          = 1;

    vk::Framebuffer framebuffer(device);
    ANGLE_TRY(framebuffer.init(framebufferInfo));

    mFramebuffer = std::move(framebuffer);

    mDirty = false;
    return vk::VkSuccess();
}

}  // namespace rx
