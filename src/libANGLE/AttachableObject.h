//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AttachableObject.h:
//   A base class for API objects that can be used as FBO attachments.
//

#ifndef LIBANGLE_ATTACHABLEOBJECT_H_
#define LIBANGLE_ATTACHABLEOBJECT_H_

#include "libANGLE/ImageIndex.h"
#include "libANGLE/RefCountObject.h"

namespace gl
{

// A framebuffer attachment points to one of three resources:
//   - a Renderbuffer, which has a unique subresource
//   - a Texture, which "textureIndex" points to (mip level + faces of a cube map)
//   - a Surface, which "binding" points to (either color or depth/stencil)

struct AttachmentSubResource
{
    AttachmentSubResource(GLenum binding, const ImageIndex &imageIndex)
        : mBinding(binding),
          mTextureIndex(imageIndex)
    {
    }

    AttachmentSubResource(const AttachmentSubResource &other)
        : mBinding(other.mBinding),
          mTextureIndex(other.mTextureIndex)
    {
    }

    AttachmentSubResource &operator=(const AttachmentSubResource &other)
    {
        this->mBinding = other.mBinding;
        this->mTextureIndex = other.mTextureIndex;
        return *this;
    }

    GLenum binding() const { return mBinding; }
    const ImageIndex &textureIndex() const { return mTextureIndex; }

  private:
    GLenum mBinding;
    ImageIndex mTextureIndex;
};

class AttachableObject : public RefCountObject
{
  public:
    AttachableObject(GLuint id) : RefCountObject(id) {}

    virtual GLsizei getAttachmentWidth(const AttachmentSubResource &subResource) const = 0;
    virtual GLsizei getAttachmentHeight(const AttachmentSubResource &subResource) const = 0;
    virtual GLenum getAttachmentInternalFormat(const AttachmentSubResource &subResource) const = 0;
    virtual GLsizei getAttachmentSamples(const AttachmentSubResource &subResource) const = 0;
};

}

#endif // LIBANGLE_ATTACHABLEOBJECT_H_
