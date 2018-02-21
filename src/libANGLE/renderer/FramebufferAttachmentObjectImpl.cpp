//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferAttachmentObjectImpl.cpp:
//   Common ancenstor for all implementations of FBO attachable-objects.
//   This means Surfaces, Textures and Renderbuffers.
//

#include "libANGLE/renderer/FramebufferAttachmentObjectImpl.h"

namespace rx
{
FramebufferAttachmentObjectImpl::FramebufferAttachmentObjectImpl()
{
}

FramebufferAttachmentObjectImpl::~FramebufferAttachmentObjectImpl()
{
}

gl::Error FramebufferAttachmentObjectImpl::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    FramebufferAttachmentRenderTarget **rtOut)
{
    UNIMPLEMENTED();
    return gl::OutOfMemory() << "getAttachmentRenderTarget not supported.";
}

gl::Error FramebufferAttachmentObjectImpl::initializeContents(const gl::Context *context,
                                                              const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return gl::OutOfMemory() << "initialize not supported.";
}

}  // namespace rx
