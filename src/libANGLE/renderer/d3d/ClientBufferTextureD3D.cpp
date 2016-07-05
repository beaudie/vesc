//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ClientBufferTextureD3D.cpp: Implements ClientBufferSiblingD3D for containing D3D textures

#include "libANGLE/renderer/d3d/ClientBufferTextureD3D.h"

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{

    ClientBufferTextureD3D::ClientBufferTextureD3D(RenderTargetD3D* renderTarget)
        : mRenderTarget(renderTarget)
    {
        ASSERT(mRenderTarget != nullptr);
    }

    ClientBufferTextureD3D::~ClientBufferTextureD3D()
    {
        SafeDelete(mRenderTarget);
    }

    GLenum ClientBufferTextureD3D::getInternalFormat() const
    {
        return mRenderTarget->getInternalFormat();
    }

    size_t ClientBufferTextureD3D::getWidth() const
    {
        return mRenderTarget->getWidth();
    }

    size_t ClientBufferTextureD3D::getHeight() const
    {
        return mRenderTarget->getHeight();
    }

    size_t ClientBufferTextureD3D::getSamples() const
    {
        return mRenderTarget->getSamples();
    }

    gl::Error ClientBufferTextureD3D::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target, FramebufferAttachmentRenderTarget **rtOut)
    {
        *rtOut = mRenderTarget;
        return gl::NoError();
    }

}
