//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ClientBufferTextureD3D.h: Implements ClientBufferSiblingD3D for containing D3D textures

#ifndef LIBANGLE_RENDERER_D3D_CLIENTBUFFERTEXTURED3D_H_
#define LIBANGLE_RENDERER_D3D_CLIENTBUFFERTEXTURED3D_H_

#include "libANGLE/renderer/d3d/EGLImageD3D.h"

namespace rx
{
    class RenderTargetD3D;

    class ClientBufferTextureD3D final : public ClientBufferSiblingD3D
    {
    public:
        // ClientBufferTextureD3D takes ownership of the provided RenderTargetD3D and will delete it upon destruction
        ClientBufferTextureD3D(RenderTargetD3D* renderTarget);
        ~ClientBufferTextureD3D() override;

        // ClientBufferSiblingImpl interface
        GLenum getInternalFormat() const override;
        size_t getWidth() const  override;
        size_t getHeight() const  override;
        size_t getSamples() const  override;

        // FramebufferAttachmentObjectImpl interface
        gl::Error getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
            FramebufferAttachmentRenderTarget **rtOut) override;

    private:
        RenderTargetD3D *mRenderTarget;
    };
}

#endif // LIBANGLE_RENDERER_D3D_CLIENTBUFFERTEXTURED3D_H_
