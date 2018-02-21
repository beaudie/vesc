//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferAttachmentObjectImpl.h:
//   Common ancenstor for all implementations of FBO attachable-objects.
//   This means Surfaces, Textures and Renderbuffers.
//

#ifndef LIBANGLE_RENDERER_FRAMEBUFFER_ATTACHMENT_OBJECT_IMPL_H_
#define LIBANGLE_RENDERER_FRAMEBUFFER_ATTACHMENT_OBJECT_IMPL_H_

#include "libANGLE/FramebufferAttachment.h"

namespace rx
{

class FramebufferAttachmentObjectImpl : angle::NonCopyable
{
  public:
    FramebufferAttachmentObjectImpl();
    virtual ~FramebufferAttachmentObjectImpl();

    virtual gl::Error getAttachmentRenderTarget(const gl::Context *context,
                                                GLenum binding,
                                                const gl::ImageIndex &imageIndex,
                                                FramebufferAttachmentRenderTarget **rtOut);

    virtual gl::Error initializeContents(const gl::Context *context,
                                         const gl::ImageIndex &imageIndex);

    void onAttach(gl::Framebuffer *framebuffer, GLenum bindingPoint);
    void onDetach(gl::Framebuffer *framebuffer);

    void onFramebufferAttachmentImplStateChange(const gl::Context *context);

  private:
    std::vector<std::pair<gl::Framebuffer *, size_t>> mParentFramebufferBindings;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_FRAMEBUFFER_ATTACHMENT_OBJECT_IMPL_H_
