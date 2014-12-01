//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferD3D.cpp: Implements the DefaultAttachmentD3D and FramebufferD3D classes.

#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/RenderTarget.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"

namespace rx
{

DefaultAttachmentD3D::DefaultAttachmentD3D(RenderTarget *renderTarget)
    : mRenderTarget(renderTarget)
{
    ASSERT(mRenderTarget);
}

DefaultAttachmentD3D::~DefaultAttachmentD3D()
{
    SafeDelete(mRenderTarget);
}

DefaultAttachmentD3D *DefaultAttachmentD3D::makeDefaultAttachmentD3D(DefaultAttachmentImpl* impl)
{
    ASSERT(HAS_DYNAMIC_TYPE(DefaultAttachmentD3D*, impl));
    return static_cast<DefaultAttachmentD3D*>(impl);
}

GLsizei DefaultAttachmentD3D::getWidth() const
{
    return mRenderTarget->getWidth();
}

GLsizei DefaultAttachmentD3D::getHeight() const
{
    return mRenderTarget->getHeight();
}

GLenum DefaultAttachmentD3D::getInternalFormat() const
{
    return mRenderTarget->getInternalFormat();
}

GLenum DefaultAttachmentD3D::getActualFormat() const
{
    return mRenderTarget->getActualFormat();
}

GLsizei DefaultAttachmentD3D::getSamples() const
{
    return mRenderTarget->getSamples();
}

RenderTarget *DefaultAttachmentD3D::getRenderTarget() const
{
    return mRenderTarget;
}


FramebufferD3D::FramebufferD3D(RendererD3D *renderer)
    : mRenderer(renderer),
      mColorBuffers(renderer->getRendererCaps().maxColorAttachments),
      mDepthbuffer(nullptr),
      mStencilbuffer(nullptr),
      mDrawBuffers(renderer->getRendererCaps().maxDrawBuffers),
      mReadBuffer(GL_COLOR_ATTACHMENT0)
{
    ASSERT(mRenderer != nullptr);

    std::fill(mColorBuffers.begin(), mColorBuffers.end(), nullptr);

    ASSERT(mDrawBuffers.size() > 0);
    mDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    std::fill(mDrawBuffers.begin() + 1, mDrawBuffers.end(), GL_NONE);
}

FramebufferD3D::~FramebufferD3D()
{
}

void FramebufferD3D::setColorAttachment(size_t index, const gl::FramebufferAttachment *attachment)
{
    ASSERT(index < mColorBuffers.size());
    mColorBuffers[index] = attachment;
    updateRenderColorBuffers();
}

void FramebufferD3D::setDepthttachment(const gl::FramebufferAttachment *attachment)
{
    mDepthbuffer = attachment;
    updateRenderColorBuffers();
}

void FramebufferD3D::setStencilAttachment(const gl::FramebufferAttachment *attachment)
{
    mStencilbuffer = attachment;
    updateRenderColorBuffers();
}

void FramebufferD3D::setDepthStencilAttachment(const gl::FramebufferAttachment *attachment)
{
    mDepthbuffer = attachment;
    mStencilbuffer = attachment;
    updateRenderColorBuffers();
}

void FramebufferD3D::setDrawBuffers(size_t count, const GLenum *buffers)
{
    std::copy_n(buffers, count, mDrawBuffers.begin());
    std::fill(mDrawBuffers.begin() + count, mDrawBuffers.end(), GL_NONE);
    updateRenderColorBuffers();
}

void FramebufferD3D::setReadBuffer(GLenum buffer)
{
    mReadBuffer = buffer;
}

gl::Error FramebufferD3D::invalidate(size_t, const GLenum *)
{
    // No-op in D3D
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferD3D::invalidate(size_t, const GLenum *, const gl::Rectangle &)
{
    // No-op in D3D
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferD3D::clear(const gl::State &state, GLbitfield mask)
{
    gl::ClearParameters clearParams = state.getClearParameters(mask);
    return clear(state, clearParams);
}

gl::Error FramebufferD3D::clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values)
{
    // glClearBufferfv can be called to clear the color buffer or depth buffer
    gl::ClearParameters clearParams = state.getClearParameters(0);

    if (buffer == GL_COLOR)
    {
        for (unsigned int i = 0; i < ArraySize(clearParams.clearColor); i++)
        {
            clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
        }
        clearParams.colorFClearValue = gl::ColorF(values[0], values[1], values[2], values[3]);
        clearParams.colorClearType = GL_FLOAT;
    }

    if (buffer == GL_DEPTH)
    {
        clearParams.clearDepth = true;
        clearParams.depthClearValue = values[0];
    }

    return clear(state, clearParams);
}

gl::Error FramebufferD3D::clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values)
{
    // glClearBufferuiv can only be called to clear a color buffer
    gl::ClearParameters clearParams = state.getClearParameters(0);
    for (unsigned int i = 0; i < ArraySize(clearParams.clearColor); i++)
    {
        clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
    }
    clearParams.colorUIClearValue = gl::ColorUI(values[0], values[1], values[2], values[3]);
    clearParams.colorClearType = GL_UNSIGNED_INT;

    return clear(state, clearParams);
}

gl::Error FramebufferD3D::clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values)
{
    // glClearBufferiv can be called to clear the color buffer or stencil buffer
    gl::ClearParameters clearParams = state.getClearParameters(0);

    if (buffer == GL_COLOR)
    {
        for (unsigned int i = 0; i < ArraySize(clearParams.clearColor); i++)
        {
            clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
        }
        clearParams.colorIClearValue = gl::ColorI(values[0], values[1], values[2], values[3]);
        clearParams.colorClearType = GL_INT;
    }

    if (buffer == GL_STENCIL)
    {
        clearParams.clearStencil = true;
        clearParams.stencilClearValue = values[1];
    }

    return clear(state, clearParams);
}

gl::Error FramebufferD3D::clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    // glClearBufferfi can only be called to clear a depth stencil buffer
    gl::ClearParameters clearParams = state.getClearParameters(0);
    clearParams.clearDepth = true;
    clearParams.depthClearValue = depth;
    clearParams.clearStencil = true;
    clearParams.stencilClearValue = stencil;

    return clear(state, clearParams);
}

GLenum FramebufferD3D::getPreferredReadFormat() const
{
    // Will require more logic if glReadBuffers is supported
    if (mColorBuffers[0] == nullptr)
    {
        return GL_NONE;
    }

    GLenum actualFormat = mColorBuffers[0]->getActualFormat();
    const gl::InternalFormat &actualFormatInfo = gl::GetInternalFormatInfo(actualFormat);

    return actualFormatInfo.format;
}

GLenum FramebufferD3D::getPreferredReadType() const
{
    // Will require more logic if glReadBuffers is supported
    if (mColorBuffers[0] == nullptr)
    {
        return GL_NONE;
    }

    GLenum actualFormat = mColorBuffers[0]->getActualFormat();
    const gl::InternalFormat &actualFormatInfo = gl::GetInternalFormatInfo(actualFormat);

    return actualFormatInfo.type;
}

gl::Error FramebufferD3D::readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type, GLvoid *pixels) const
{
    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(format, type);
    const gl::InternalFormat &sizedFormatInfo = gl::GetInternalFormatInfo(sizedInternalFormat);
    GLuint outputPitch = sizedFormatInfo.computeRowPitch(type, area.width, state.getPackAlignment());

    return readPixels(area, format, type, outputPitch, state.getPackState(), reinterpret_cast<uint8_t*>(pixels));
}

gl::Error FramebufferD3D::blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                               GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer)
{
    bool blitRenderTarget = false;
    if ((mask & GL_COLOR_BUFFER_BIT) &&
        sourceFramebuffer->getReadColorbuffer() != nullptr &&
        std::any_of(mColorBuffers.begin(), mColorBuffers.end(), [](const gl::FramebufferAttachment* attachment){ return attachment != nullptr; }))
    {
        blitRenderTarget = true;
    }

    bool blitStencil = false;
    if ((mask & GL_STENCIL_BUFFER_BIT) &&
        sourceFramebuffer->getStencilbuffer() != nullptr &&
        mStencilbuffer != nullptr)
    {
        blitStencil = true;
    }

    bool blitDepth = false;
    if ((mask & GL_DEPTH_BUFFER_BIT) &&
        sourceFramebuffer->getDepthbuffer() != nullptr &&
        mDepthbuffer != nullptr)
    {
        blitDepth = true;
    }

    if (blitRenderTarget || blitDepth || blitStencil)
    {
        const gl::Rectangle *scissor = state.isScissorTestEnabled() ? &state.getScissor() : NULL;
        gl::Error error = blit(sourceArea, destArea, scissor, blitRenderTarget, blitDepth, blitStencil,
                               filter, sourceFramebuffer);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

GLenum FramebufferD3D::getStatus() const
{
    // D3D11 does not allow for overlapping RenderTargetViews, so ensure uniqueness
    for (size_t colorAttachment = 0; colorAttachment < mColorBuffers.size(); colorAttachment++)
    {
        const gl::FramebufferAttachment *attachment = mColorBuffers[colorAttachment];
        if (attachment != nullptr)
        {
            for (size_t prevColorAttachment = 0; prevColorAttachment < colorAttachment; prevColorAttachment++)
            {
                const gl::FramebufferAttachment *prevAttachment = mColorBuffers[prevColorAttachment];
                if (prevAttachment != nullptr &&
                    (attachment->id() == prevAttachment->id() &&
                     attachment->type() == prevAttachment->type()))
                {
                    return GL_FRAMEBUFFER_UNSUPPORTED;
                }
            }
        }
    }

    return GL_FRAMEBUFFER_COMPLETE;
}

const ColorbufferInfoVector &FramebufferD3D::getColorbuffersForRender() const
{
    return mRenderColorBuffers;
}

void FramebufferD3D::updateRenderColorBuffers()
{
    bool mrtPerfWorkaround = mRenderer->getWorkarounds().mrtPerfWorkaround;

    mRenderColorBuffers.clear();
    for (size_t colorAttachment = 0; colorAttachment < mColorBuffers.size(); ++colorAttachment)
    {
        GLenum drawBufferState = mDrawBuffers[colorAttachment];
        const gl::FramebufferAttachment *colorbuffer = mColorBuffers[colorAttachment];

        if (colorbuffer != NULL && drawBufferState != GL_NONE)
        {
            ASSERT(drawBufferState == GL_BACK || drawBufferState == (GL_COLOR_ATTACHMENT0_EXT + colorAttachment));
            mRenderColorBuffers.push_back(colorbuffer);
        }
        else if (!mrtPerfWorkaround)
        {
            mRenderColorBuffers.push_back(NULL);
        }
    }
}

}
