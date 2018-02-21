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

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"

namespace rx
{

namespace
{
size_t ConvertBindingToDirtyBit(GLenum binding)
{
    switch (binding)
    {
        case GL_DEPTH:
        case GL_DEPTH_ATTACHMENT:
            return gl::Framebuffer::DIRTY_BIT_DEPTH_ATTACHMENT;
        case GL_STENCIL:
        case GL_STENCIL_ATTACHMENT:
            return gl::Framebuffer::DIRTY_BIT_STENCIL_ATTACHMENT;
        case GL_DEPTH_STENCIL:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            return (gl::Framebuffer::DIRTY_BIT_DEPTH_ATTACHMENT |
                    gl::Framebuffer::DIRTY_BIT_STENCIL_ATTACHMENT);
        case GL_BACK:
            return gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0;
        default:
            ASSERT(binding >= GL_COLOR_ATTACHMENT0 &&
                   binding < GL_COLOR_ATTACHMENT0 + gl::IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS);
            return gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0 + (binding - GL_COLOR_ATTACHMENT0);
    }
}
}  // anonymous namespace

FramebufferAttachmentObjectImpl::FramebufferAttachmentObjectImpl()
{
}

FramebufferAttachmentObjectImpl::~FramebufferAttachmentObjectImpl()
{
    ASSERT(mParentFramebufferBindings.empty());
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

void FramebufferAttachmentObjectImpl::onFramebufferAttachmentImplStateChange(
    const gl::Context *context)
{
    for (auto &framebufferBinding : mParentFramebufferBindings)
    {
        context->dirtyFramebuffer(framebufferBinding.first, framebufferBinding.second);
    }
}

void FramebufferAttachmentObjectImpl::onAttach(gl::Framebuffer *framebuffer, GLenum bindingPoint)
{
    if (framebuffer)
    {
        mParentFramebufferBindings.push_back(
            std::make_pair(framebuffer, ConvertBindingToDirtyBit(bindingPoint)));
    }
}

void FramebufferAttachmentObjectImpl::onDetach(gl::Framebuffer *framebuffer)
{
    if (!framebuffer)
    {
        return;
    }

    for (size_t fbIndex = 0; fbIndex < mParentFramebufferBindings.size(); ++fbIndex)
    {
        if (mParentFramebufferBindings[fbIndex].first == framebuffer)
        {
            mParentFramebufferBindings.erase(mParentFramebufferBindings.begin() + fbIndex);
            return;
        }
    }

    UNREACHABLE();
}

}  // namespace rx
