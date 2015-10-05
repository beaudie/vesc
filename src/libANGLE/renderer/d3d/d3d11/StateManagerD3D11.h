//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D11.h: Defines a class for caching D3D11 state

#ifndef LIBANGLE_RENDERER_D3D11_STATEMANAGERD3D11_H_
#define LIBANGLE_RENDERER_D3D11_STATEMANAGERD3D11_H_

#include "libANGLE/renderer/d3d/StateManagerD3D.h"

#include "libANGLE/renderer/d3d/d3d11/RenderStateCache.h"

namespace rx
{

class StateManagerD3D11 final : public StateManagerD3D
{
  public:
    explicit StateManagerD3D11(ID3D11DeviceContext *deviceContext, RenderStateCache &stateCache);

    virtual ~StateManagerD3D11();

    virtual gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                                    const gl::BlendState &blendState,
                                    const gl::ColorF &blendColor,
                                    unsigned int sampleMask,
                                    const gl::State::DirtyBits &dirtyBits);

    virtual gl::Error setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                           int stencilRef,
                                           int stencilBackRef,
                                           bool frontFaceCCW,
                                           const gl::State::DirtyBits &dirtyBits);

    virtual gl::Error setRasterizerState(const gl::RasterizerState &rasterizerState,
                                         const gl::State::DirtyBits &dirtyBits);

  private:
    ID3D11DeviceContext *mDeviceContext;
    RenderStateCache &mStateCache;
};
}
#endif