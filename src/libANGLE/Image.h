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
    Image(rx::ImageImpl *impl, EGLenum target, EGLClientBuffer buffer, const AttributeMap &attribs);
    ~Image();

    void orphan(gl::FramebufferAttachmentObject *sibling);

    rx::ImageImpl *getImplementation();
    const rx::ImageImpl *getImplementation() const;

  private:
    rx::ImageImpl *mImplementation;

    BindingPointer<gl::FramebufferAttachmentObject> mSource;
    std::vector<BindingPointer<gl::FramebufferAttachmentObject>> mTargets;
};

}

#endif // LIBANGLE_IMAGE_H_
