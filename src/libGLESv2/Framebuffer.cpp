#include "precompiled.h"
//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer.cpp: Implements the gl::Framebuffer class. Implements GL framebuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4 page 105.

#include "libGLESv2/Framebuffer.h"

#include "libGLESv2/main.h"
#include "common/utilities.h"
#include "libGLESv2/formatutils.h"
#include "libGLESv2/Texture.h"
#include "libGLESv2/Context.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/Renderbuffer.h"

namespace gl
{

template <typename T>
static T *AttachmentPointerNoneNulled(T &value)
{
    return (value.type() == GL_NONE ? NULL : &value);
}

Framebuffer::Framebuffer(rx::Renderer *renderer)
    : mRenderer(renderer)
{
    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        mDrawBufferStates[colorAttachment] = GL_NONE;
    }
    mDrawBufferStates[0] = GL_COLOR_ATTACHMENT0_EXT;
    mReadBufferState = GL_COLOR_ATTACHMENT0_EXT;
}

Framebuffer::~Framebuffer()
{
    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        mColorbuffers[colorAttachment].setImplementation(NULL);
    }
    mDepthbuffer.setImplementation(NULL);
    mStencilbuffer.setImplementation(NULL);
}

FramebufferAttachmentImpl *Framebuffer::getAttachmentImpl(GLenum type, GLuint handle, GLint level, GLint layer) const
{
    if (handle == 0)
    {
        return NULL;
    }

    gl::Context *context = gl::getContext();

    switch (type)
    {
      case GL_NONE:
        return NULL;

      case GL_RENDERBUFFER:
        return new RenderbufferAttachment(context->getRenderbuffer(handle));

      case GL_TEXTURE_2D:
        {
            Texture *texture = context->getTexture(handle);
            if (texture && texture->getTarget() == GL_TEXTURE_2D)
            {
                Texture2D *tex2D = static_cast<Texture2D*>(texture);
                return new Texture2DAttachment(tex2D, level);
            }
            else
            {
                return NULL;
            }
        }

      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        {
            Texture *texture = context->getTexture(handle);
            if (texture && texture->getTarget() == GL_TEXTURE_CUBE_MAP)
            {
                TextureCubeMap *texCube = static_cast<TextureCubeMap*>(texture);
                return new TextureCubeMapAttachment(texCube, type, level);
            }
            else
            {
                return NULL;
            }
        }

      case GL_TEXTURE_3D:
        {
            Texture *texture = context->getTexture(handle);
            if (texture && texture->getTarget() == GL_TEXTURE_3D)
            {
                Texture3D *tex3D = static_cast<Texture3D*>(texture);
                return new Texture3DAttachment(tex3D, level, layer);
            }
            else
            {
                return NULL;
            }
        }

      case GL_TEXTURE_2D_ARRAY:
        {
            Texture *texture = context->getTexture(handle);
            if (texture && texture->getTarget() == GL_TEXTURE_2D_ARRAY)
            {
                Texture2DArray *tex2DArray = static_cast<Texture2DArray*>(texture);
                return new Texture2DArrayAttachment(tex2DArray, level, layer);
            }
            else
            {
                return NULL;
            }
        }

      default:
        UNREACHABLE();
        return NULL;
    }
}

void Framebuffer::setColorbuffer(unsigned int colorAttachment, GLenum type, GLuint colorbuffer, GLint level, GLint layer)
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    FramebufferAttachmentImpl *attachmentImpl = getAttachmentImpl(type, colorbuffer, level, layer);
    mColorbuffers[colorAttachment].setImplementation(attachmentImpl);
}

void Framebuffer::setDepthbuffer(GLenum type, GLuint depthbuffer, GLint level, GLint layer)
{
    FramebufferAttachmentImpl *attachmentImpl = getAttachmentImpl(type, depthbuffer, level, layer);
    mDepthbuffer.setImplementation(attachmentImpl);
}

void Framebuffer::setStencilbuffer(GLenum type, GLuint stencilbuffer, GLint level, GLint layer)
{
    FramebufferAttachmentImpl *attachmentImpl = getAttachmentImpl(type, stencilbuffer, level, layer);
    mStencilbuffer.setImplementation(attachmentImpl);
}

void Framebuffer::setDepthStencilBuffer(GLenum type, GLuint depthStencilBuffer, GLint level, GLint layer)
{
    FramebufferAttachmentImpl *attachmentImpl = getAttachmentImpl(type, depthStencilBuffer, level, layer);
    mDepthbuffer.setImplementation(attachmentImpl);
    mStencilbuffer.setImplementation(attachmentImpl);

    // ensure this is a legitimate depth+stencil format, and if not, unset it
    int clientVersion = mRenderer->getCurrentClientVersion();
    if (mDepthbuffer.getDepthSize(clientVersion) == 0 || mStencilbuffer.getStencilSize(clientVersion) == 0)
    {
        mDepthbuffer.setImplementation(NULL);
        mStencilbuffer.setImplementation(NULL);
    }
}

void Framebuffer::detachTexture(GLuint textureId)
{
    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (mColorbuffers[colorAttachment].id() == textureId && mColorbuffers[colorAttachment].isTexture())
        {
            mColorbuffers[colorAttachment].setImplementation(NULL);
        }
    }

    if (mDepthbuffer.id() == textureId && mDepthbuffer.isTexture())
    {
        mDepthbuffer.setImplementation(NULL);
    }

    if (mStencilbuffer.id() == textureId && mStencilbuffer.isTexture())
    {
        mStencilbuffer.setImplementation(NULL);
    }
}

void Framebuffer::detachRenderbuffer(GLuint renderbufferId)
{
    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (mColorbuffers[colorAttachment].id() == renderbufferId && mColorbuffers[colorAttachment].isTexture())
        {
            mColorbuffers[colorAttachment].setImplementation(NULL);
        }
    }

    if (mDepthbuffer.id() == renderbufferId && !mDepthbuffer.isTexture())
    {
        mDepthbuffer.setImplementation(NULL);
    }

    if (mStencilbuffer.id() == renderbufferId && !mStencilbuffer.isTexture())
    {
        mStencilbuffer.setImplementation(NULL);
    }
}

unsigned int Framebuffer::getRenderTargetSerial(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return mColorbuffers[colorAttachment].getSerial();
}

unsigned int Framebuffer::getDepthbufferSerial() const
{
    return mDepthbuffer.getSerial();
}

unsigned int Framebuffer::getStencilbufferSerial() const
{
    return mStencilbuffer.getSerial();
}

FramebufferAttachment *Framebuffer::getColorbuffer(unsigned int colorAttachment)
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return AttachmentPointerNoneNulled(mColorbuffers[colorAttachment]);
}

const FramebufferAttachment *Framebuffer::getColorbuffer(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return AttachmentPointerNoneNulled(mColorbuffers[colorAttachment]);
}

FramebufferAttachment *Framebuffer::getDepthbuffer()
{
    return AttachmentPointerNoneNulled(mDepthbuffer);
}

FramebufferAttachment *Framebuffer::getStencilbuffer()
{
    return AttachmentPointerNoneNulled(mStencilbuffer);
}

FramebufferAttachment *Framebuffer::getDepthStencilBuffer()
{
    return (mDepthbuffer.id() == mStencilbuffer.id()) ? AttachmentPointerNoneNulled(mDepthbuffer) : NULL;
}

FramebufferAttachment *Framebuffer::getDepthOrStencilbuffer()
{
    FramebufferAttachment *depthstencilbuffer = AttachmentPointerNoneNulled(mDepthbuffer);
    
    if (!depthstencilbuffer)
    {
        depthstencilbuffer = AttachmentPointerNoneNulled(mStencilbuffer);
    }

    return depthstencilbuffer;
}

FramebufferAttachment *Framebuffer::getReadColorbuffer()
{
    // Will require more logic if glReadBuffers is supported
    return AttachmentPointerNoneNulled(mColorbuffers[0]);
}

GLenum Framebuffer::getReadColorbufferType() const
{
    // Will require more logic if glReadBuffers is supported
    return mColorbuffers[0].type();
}

FramebufferAttachment *Framebuffer::getFirstColorbuffer()
{
    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (mColorbuffers[colorAttachment].type() != GL_NONE)
        {
            return &mColorbuffers[colorAttachment];
        }
    }

    return NULL;
}

GLenum Framebuffer::getColorbufferType(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return mColorbuffers[colorAttachment].type();
}

GLenum Framebuffer::getDepthbufferType() const
{
    return mDepthbuffer.type();
}

GLenum Framebuffer::getStencilbufferType() const
{
    return mStencilbuffer.type();
}

GLenum Framebuffer::getDepthStencilbufferType() const
{
    return (mDepthbuffer.id() == mStencilbuffer.id()) ? mDepthbuffer.type() : GL_NONE;
}

GLuint Framebuffer::getColorbufferHandle(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return mColorbuffers[colorAttachment].id();
}

GLuint Framebuffer::getDepthbufferHandle() const
{
    return mDepthbuffer.id();
}

GLuint Framebuffer::getStencilbufferHandle() const
{
    return mStencilbuffer.id();
}

GLenum Framebuffer::getDepthStencilbufferHandle() const
{
    return (mDepthbuffer.id() == mStencilbuffer.id()) ? mDepthbuffer.id() : 0;
}

GLenum Framebuffer::getColorbufferMipLevel(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return mColorbuffers[colorAttachment].mipLevel();
}

GLenum Framebuffer::getDepthbufferMipLevel() const
{
    return mDepthbuffer.mipLevel();
}

GLenum Framebuffer::getStencilbufferMipLevel() const
{
    return mStencilbuffer.mipLevel();
}

GLenum Framebuffer::getDepthStencilbufferMipLevel() const
{
    return (mDepthbuffer.id() == mStencilbuffer.id()) ? mDepthbuffer.mipLevel() : 0;
}

GLenum Framebuffer::getColorbufferLayer(unsigned int colorAttachment) const
{
    ASSERT(colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS);
    return mColorbuffers[colorAttachment].layer();
}

GLenum Framebuffer::getDepthbufferLayer() const
{
    return mDepthbuffer.layer();
}

GLenum Framebuffer::getStencilbufferLayer() const
{
    return mStencilbuffer.layer();
}

GLenum Framebuffer::getDepthStencilbufferLayer() const
{
    return (mDepthbuffer.id() == mStencilbuffer.id()) ? mDepthbuffer.layer() : 0;
}

GLenum Framebuffer::getDrawBufferState(unsigned int colorAttachment) const
{
    return mDrawBufferStates[colorAttachment];
}

void Framebuffer::setDrawBufferState(unsigned int colorAttachment, GLenum drawBuffer)
{
    mDrawBufferStates[colorAttachment] = drawBuffer;
}

bool Framebuffer::isEnabledColorAttachment(unsigned int colorAttachment) const
{
    return (mColorbuffers[colorAttachment].type() != GL_NONE && mDrawBufferStates[colorAttachment] != GL_NONE);
}

bool Framebuffer::hasEnabledColorAttachment() const
{
    for (unsigned int colorAttachment = 0; colorAttachment < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (isEnabledColorAttachment(colorAttachment))
        {
            return true;
        }
    }

    return false;
}

bool Framebuffer::hasStencil() const
{
    if (mStencilbuffer.type() != GL_NONE)
    {
        int clientVersion = mRenderer->getCurrentClientVersion();
        return (mStencilbuffer.getStencilSize(clientVersion) > 0);
    }

    return false;
}

bool Framebuffer::usingExtendedDrawBuffers() const
{
    for (unsigned int colorAttachment = 1; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (isEnabledColorAttachment(colorAttachment))
        {
            return true;
        }
    }

    return false;
}

GLenum Framebuffer::completeness() const
{
    int width = 0;
    int height = 0;
    unsigned int colorbufferSize = 0;
    int samples = -1;
    bool missingAttachment = true;
    GLuint clientVersion = mRenderer->getCurrentClientVersion();

    for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
    {
        if (mColorbuffers[colorAttachment].type() != GL_NONE)
        {
            const FramebufferAttachment *colorbuffer = &mColorbuffers[colorAttachment];

            if (colorbuffer->type() == GL_NONE)
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            if (colorbuffer->getWidth() == 0 || colorbuffer->getHeight() == 0)
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            if (mColorbuffers[colorAttachment].type() == GL_RENDERBUFFER)
            {
                if (!gl::IsColorRenderingSupported(colorbuffer->getInternalFormat(), mRenderer))
                {
                    return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                }
            }
            else if (IsInternalTextureTarget(mColorbuffers[colorAttachment].type(), mRenderer->getCurrentClientVersion()))
            {
                GLenum internalformat = colorbuffer->getInternalFormat();

                if (!gl::IsColorRenderingSupported(internalformat, mRenderer))
                {
                    return GL_FRAMEBUFFER_UNSUPPORTED;
                }

                if (gl::GetDepthBits(internalformat, clientVersion) > 0 ||
                    gl::GetStencilBits(internalformat, clientVersion) > 0)
                {
                    return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                }
            }
            else
            {
                UNREACHABLE();
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            if (!missingAttachment)
            {
                // all color attachments must have the same width and height
                if (colorbuffer->getWidth() != width || colorbuffer->getHeight() != height)
                {
                    return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
                }

                // APPLE_framebuffer_multisample, which EXT_draw_buffers refers to, requires that
                // all color attachments have the same number of samples for the FBO to be complete.
                if (colorbuffer->getSamples() != samples)
                {
                    return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT;
                }

                // in GLES 2.0, all color attachments attachments must have the same number of bitplanes
                // in GLES 3.0, there is no such restriction
                if (clientVersion < 3)
                {
                    if (gl::GetPixelBytes(colorbuffer->getInternalFormat(), clientVersion) != colorbufferSize)
                    {
                        return GL_FRAMEBUFFER_UNSUPPORTED;
                    }
                }

                // D3D11 does not allow for overlapping RenderTargetViews, so ensure uniqueness
                for (unsigned int previousColorAttachment = 0; previousColorAttachment < colorAttachment; previousColorAttachment++)
                {
                    const FramebufferAttachment &currentAttachment = mColorbuffers[colorAttachment];
                    const FramebufferAttachment &previousAttachment = mColorbuffers[previousColorAttachment];

                    if (currentAttachment.id() == previousAttachment.id() &&
                        currentAttachment.type() == previousAttachment.type())
                    {
                        return GL_FRAMEBUFFER_UNSUPPORTED;
                    }
                }
            }
            else
            {
                width = colorbuffer->getWidth();
                height = colorbuffer->getHeight();
                samples = colorbuffer->getSamples();
                colorbufferSize = gl::GetPixelBytes(colorbuffer->getInternalFormat(), clientVersion);
                missingAttachment = false;
            }
        }
    }

    if (mDepthbuffer.type() != GL_NONE)
    {
        if (mDepthbuffer.getWidth() == 0 || mDepthbuffer.getHeight() == 0)
        {
            return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
        }

        if (mDepthbuffer.type() == GL_RENDERBUFFER)
        {
            if (!gl::IsDepthRenderingSupported(mDepthbuffer.getInternalFormat(), mRenderer))
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }
        }
        else if (IsInternalTextureTarget(mDepthbuffer.type(), mRenderer->getCurrentClientVersion()))
        {
            GLenum internalformat = mDepthbuffer.getInternalFormat();

            // depth texture attachments require OES/ANGLE_depth_texture
            if (!mRenderer->getDepthTextureSupport())
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            if (gl::GetDepthBits(internalformat, clientVersion) == 0)
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }
        }
        else
        {
            UNREACHABLE();
            return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
        }

        if (missingAttachment)
        {
            width = mDepthbuffer.getWidth();
            height = mDepthbuffer.getHeight();
            samples = mDepthbuffer.getSamples();
            missingAttachment = false;
        }
        else if (width != mDepthbuffer.getWidth() || height != mDepthbuffer.getHeight())
        {
            return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
        }
        else if (samples != mDepthbuffer.getSamples())
        {
            return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE;
        }
    }

    if (mStencilbuffer.type() != GL_NONE)
    {
        if (mStencilbuffer.getWidth() == 0 || mStencilbuffer.getHeight() == 0)
        {
            return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
        }

        if (mStencilbuffer.type() == GL_RENDERBUFFER)
        {
            if (!gl::IsStencilRenderingSupported(mStencilbuffer.getInternalFormat(), mRenderer))
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }
        }
        else if (IsInternalTextureTarget(mStencilbuffer.type(), mRenderer->getCurrentClientVersion()))
        {
            GLenum internalformat = mStencilbuffer.getInternalFormat();

            // texture stencil attachments come along as part
            // of OES_packed_depth_stencil + OES/ANGLE_depth_texture
            if (!mRenderer->getDepthTextureSupport())
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }

            if (gl::GetStencilBits(internalformat, clientVersion) == 0)
            {
                return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            }
        }
        else
        {
            UNREACHABLE();
            return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
        }

        if (missingAttachment)
        {
            width = mStencilbuffer.getWidth();
            height = mStencilbuffer.getHeight();
            samples = mStencilbuffer.getSamples();
            missingAttachment = false;
        }
        else if (width != mStencilbuffer.getWidth() || height != mStencilbuffer.getHeight())
        {
            return GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
        }
        else if (samples != mStencilbuffer.getSamples())
        {
            return GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE;
        }
    }

    // if we have both a depth and stencil buffer, they must refer to the same object
    // since we only support packed_depth_stencil and not separate depth and stencil
    if (mDepthbuffer.type() != GL_NONE && mStencilbuffer.type() != GL_NONE &&
        !(mDepthbuffer.type() == mStencilbuffer.type() &&
          mDepthbuffer.id() == mStencilbuffer.id()))
    {
        return GL_FRAMEBUFFER_UNSUPPORTED;
    }

    // we need to have at least one attachment to be complete
    if (missingAttachment)
    {
        return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    }

    return GL_FRAMEBUFFER_COMPLETE;
}

DefaultFramebuffer::DefaultFramebuffer(rx::Renderer *renderer, Colorbuffer *colorbuffer, DepthStencilbuffer *depthStencil)
    : Framebuffer(renderer)
{
    Renderbuffer *colorRenderbuffer = new Renderbuffer(mRenderer, 0, colorbuffer);
    mColorbuffers[0].setImplementation(new RenderbufferAttachment(colorRenderbuffer));

    Renderbuffer *depthStencilRenderbuffer = new Renderbuffer(mRenderer, 0, depthStencil);
    FramebufferAttachmentImpl *depthStencilAttachment = new RenderbufferAttachment(depthStencilRenderbuffer);
    mDepthbuffer.setImplementation(depthStencilRenderbuffer->getDepthSize() != 0 ? depthStencilAttachment : NULL);
    mStencilbuffer.setImplementation(depthStencilRenderbuffer->getStencilSize() != 0 ? depthStencilAttachment : NULL);

    mDrawBufferStates[0] = GL_BACK;
    mReadBufferState = GL_BACK;
}

int Framebuffer::getSamples() const
{
    if (completeness() == GL_FRAMEBUFFER_COMPLETE)
    {
        // for a complete framebuffer, all attachments must have the same sample count
        // in this case return the first nonzero sample size
        for (unsigned int colorAttachment = 0; colorAttachment < IMPLEMENTATION_MAX_DRAW_BUFFERS; colorAttachment++)
        {
            if (mColorbuffers[colorAttachment].type() != GL_NONE)
            {
                return mColorbuffers[colorAttachment].getSamples();
            }
        }
    }

    return 0;
}

GLenum DefaultFramebuffer::completeness() const
{
    // The default framebuffer *must* always be complete, though it may not be
    // subject to the same rules as application FBOs. ie, it could have 0x0 size.
    return GL_FRAMEBUFFER_COMPLETE;
}

}
