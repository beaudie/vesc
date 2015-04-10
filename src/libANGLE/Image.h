//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.h: Defines the egl::Image class representing the EGLimage object.

#ifndef LIBANGLE_IMAGE_H_
#define LIBANGLE_IMAGE_H_

#include "common/angleutils.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"

namespace rx
{
class ImageImpl;
}

namespace egl
{

class Image final : angle::NonCopyable
{
  public:
    Image(rx::ImageImpl *impl, EGLenum target, gl::Texture *buffer, const AttributeMap &attribs);
    ~Image();

    void reference(gl::Texture *sibling);
    void orphan(gl::Texture *sibling);

    GLenum getInternalFormat() const;
    size_t getWidth() const;
    size_t getHeight() const;
    size_t getSamples() const;

    rx::ImageImpl *getImplementation();
    const rx::ImageImpl *getImplementation() const;

  private:
    rx::ImageImpl *mImplementation;

    GLenum mInternalFormat;
    size_t mWidth;
    size_t mHeight;
    size_t mSamples;

    std::vector<BindingPointer<gl::Texture>> mSiblings;
};

}

#endif // LIBANGLE_IMAGE_H_
