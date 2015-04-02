//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AttachmentImpl.h:
//   Helper class for FBO attachment classes.
//

#ifndef LIBANGLE_RENDERER_ATTACHMENTIMPL_H_
#define LIBANGLE_RENDERER_ATTACHMENTIMPL_H_

#include "libANGLE/Error.h"

namespace gl
{
struct AttachmentSubResource;
}

namespace rx
{
typedef void *AttachmentRenderTarget;

class AttachmentImpl : angle::NonCopyable
{
  public:
    AttachmentImpl() {}
    virtual ~AttachmentImpl() {}

    virtual gl::Error getAttachmentRenderTarget(const gl::AttachmentSubResource &subResource,
                                                AttachmentRenderTarget *rtOut)
    {
        // Default: unsupported
        return gl::Error(GL_INVALID_OPERATION, "Internal RT not supported");
    }
};

}

#endif // LIBANGLE_RENDERER_ATTACHMENTIMPL_H_
