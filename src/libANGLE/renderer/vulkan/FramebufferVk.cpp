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

gl::ErrorOrResult<const gl::InternalFormat *> GetReadAttachmentInfo(
    const gl::FramebufferAttachment *readAttachment)
{
    const RenderTargetVk *renderTarget = nullptr;
    ANGLE_TRY(readAttachment->getRenderTarget(&renderTarget));

    GLenum implFormat = renderTarget->getFormat().format().fboImplementationInternalFormat;
    return &gl::GetInternalFormatInfo(implFormat);
}

}  // anonymous namespace

FramebufferVk::FramebufferVk(const gl::FramebufferState &state)
    : FramebufferImpl(state), mDirty(true)
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
    vk::StagingImage stagingImage;
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
    ANGLE_TRY(stagingImage.getMemory().map(0, stagingImage.getSize(), 0, &mapPointer));

    const auto &angleFormat = renderTarget->getFormat().format();

    // TODO(jmadill): Use pixel bytes from the ANGLE format directly.
    const auto &glFormat = gl::GetInternalFormatInfo(angleFormat.glInternalFormat);
    int inputPitch       = glFormat.pixelBytes * area.width;

    PackPixelsParams params;
    params.area        = area;
    params.format      = format;
    params.type        = type;
    params.outputPitch = inputPitch;
    params.pack        = glState.getPackState();

    PackPixels(params, angleFormat, inputPitch, mapPointer, reinterpret_cast<uint8_t *>(pixels));

    stagingImage.getMemory().unmap();

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

}  // namespace rx
