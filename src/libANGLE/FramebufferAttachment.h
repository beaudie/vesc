//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferAttachment.h: Defines the wrapper class gl::FramebufferAttachment, as well as the
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBANGLE_FRAMEBUFFERATTACHMENT_H_
#define LIBANGLE_FRAMEBUFFERATTACHMENT_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Surface.h"

namespace gl
{
// FramebufferAttachment implements a GL framebuffer attachment.
// Attachments are "light" containers, which store pointers to ref-counted GL objects.
// We support GL texture (2D/3D/Cube/2D array) and renderbuffer object attachments.
// Note: Our old naming scheme used the term "Renderbuffer" for both GL renderbuffers and for
// framebuffer attachments, which confused their usage.

class FramebufferAttachment final : angle::NonCopyable
{
  public:
    FramebufferAttachment(GLenum type,
                          GLenum binding,
                          const ImageIndex &textureIndex,
                          AttachableObject *resource);
    ~FramebufferAttachment();

    // Helper methods
    GLuint getRedSize() const;
    GLuint getGreenSize() const;
    GLuint getBlueSize() const;
    GLuint getAlphaSize() const;
    GLuint getDepthSize() const;
    GLuint getStencilSize() const;
    GLenum getComponentType() const;
    GLenum getColorEncoding() const;

    bool isTextureWithId(GLuint textureId) const { return mType == GL_TEXTURE && id() == textureId; }
    bool isRenderbufferWithId(GLuint renderbufferId) const { return mType == GL_RENDERBUFFER && id() == renderbufferId; }

    GLenum getBinding() const { return mSubResource.binding(); }
    GLuint id() const { return mResource.id(); }

    // These methods are only legal to call on Texture attachments
    const ImageIndex &getTextureImageIndex() const;
    GLenum cubeMapFace() const;
    GLint mipLevel() const;
    GLint layer() const;

    GLsizei getWidth() const { return mResource->getAttachmentWidth(mSubResource); }
    GLsizei getHeight() const { return mResource->getAttachmentHeight(mSubResource); }
    GLenum getInternalFormat() const { return mResource->getAttachmentInternalFormat(mSubResource); }
    GLsizei getSamples() const { return mResource->getAttachmentSamples(mSubResource); }
    GLenum type() const { return mType; }

    Renderbuffer *getRenderbuffer() const
    {
        ASSERT(mType == GL_RENDERBUFFER);
        return rx::GetAs<Renderbuffer>(mResource.get());
    }

    Texture *getTexture() const
    {
        ASSERT(mType == GL_TEXTURE);
        return rx::GetAs<Texture>(mResource.get());
    }

    const egl::Surface *getSurface() const
    {
        ASSERT(mType == GL_FRAMEBUFFER_DEFAULT);
        return rx::GetAs<egl::Surface>(mResource.get());
    }

  private:
    GLenum mType;
    AttachmentSubResource mSubResource;
    BindingPointer<AttachableObject> mResource;
};

}

#endif // LIBANGLE_FRAMEBUFFERATTACHMENT_H_
