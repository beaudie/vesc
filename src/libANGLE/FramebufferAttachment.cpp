//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferAttachment.cpp: the gl::FramebufferAttachment class and its derived classes
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#include "libANGLE/FramebufferAttachment.h"

#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace gl
{

////// FramebufferAttachment Implementation //////

FramebufferAttachment::FramebufferAttachment()
    : mType(GL_NONE),
      mSubResource(GL_NONE, ImageIndex::MakeInvalid())
{
}

FramebufferAttachment::FramebufferAttachment(GLenum type,
                                             GLenum binding,
                                             const ImageIndex &textureIndex,
                                             AttachableObject *resource)
    : mSubResource(binding, textureIndex),
      mType(type)
{
    mResource.set(resource);
}

void FramebufferAttachment::reset()
{
    mType = GL_NONE;
    mResource.set(nullptr);

    // not technically necessary, could omit for performance
    mSubResource = AttachmentSubResource(GL_NONE, ImageIndex::MakeInvalid());
}

void FramebufferAttachment::setResource(GLenum type,
                                        GLenum binding,
                                        const ImageIndex &textureIndex,
                                        AttachableObject *resource)
{
    mType = type;
    mSubResource = AttachmentSubResource(binding, textureIndex);
    mResource.set(resource);
}

FramebufferAttachment::~FramebufferAttachment()
{
    mResource.set(nullptr);
}

GLuint FramebufferAttachment::getRedSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).redBits;
}

GLuint FramebufferAttachment::getGreenSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).greenBits;
}

GLuint FramebufferAttachment::getBlueSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).blueBits;
}

GLuint FramebufferAttachment::getAlphaSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).alphaBits;
}

GLuint FramebufferAttachment::getDepthSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).depthBits;
}

GLuint FramebufferAttachment::getStencilSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).stencilBits;
}

GLenum FramebufferAttachment::getComponentType() const
{
    return GetInternalFormatInfo(getInternalFormat()).componentType;
}

GLenum FramebufferAttachment::getColorEncoding() const
{
    return GetInternalFormatInfo(getInternalFormat()).colorEncoding;
}

GLenum FramebufferAttachment::cubeMapFace() const
{
    ASSERT(mType == GL_TEXTURE);

    const auto &index = mSubResource.textureIndex();
    return IsCubeMapTextureTarget(index.type) ? index.type : GL_NONE;
}

GLint FramebufferAttachment::layer() const
{
    ASSERT(mType == GL_TEXTURE);

    const auto &index = mSubResource.textureIndex();

    if (index.type == GL_TEXTURE_2D_ARRAY || index.type == GL_TEXTURE_3D)
    {
        return index.layerIndex;
    }
    return 0;
}

}
