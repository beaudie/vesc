//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.h: Defines a class for caching D3D11 state

#ifndef LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_
#define LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/d3d/d3d11/RenderStateCache.h"

namespace rx
{

class StateManager11
{
  public:
    StateManager11(ID3D11DeviceContext *deviceContext, RenderStateCache *stateCache);

    ~StateManager11();

    gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                            const gl::BlendState &blendState,
                            const gl::ColorF &blendColor,
                            unsigned int sampleMask,
                            const gl::State::DirtyBits &dirtyBits);

    void forceSetBlendState() { mForceSetBlendState = true; }
    void setBlendStateIsDirty() { mBlendStateIsDirty = true; }

  private:
    // Blend State
    bool mBlendStateIsDirty;
    // TODO(dianx) temporary representation of a dirty bit. once we move enough states in,
    // try experimenting with dirty bit instead of a bool
    bool mForceSetBlendState;
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    ID3D11DeviceContext *mDeviceContext;
    RenderStateCache *mStateCache;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_