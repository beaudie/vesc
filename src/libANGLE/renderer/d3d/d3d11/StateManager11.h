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

class Renderer11;

class StateManager11 final : angle::NonCopyable
{
  public:
    StateManager11(Renderer11 *renderer11);

    ~StateManager11();

    gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                            const gl::BlendState &blendState,
                            const gl::ColorF &blendColor,
                            unsigned int sampleMask);

    gl::Error setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                   int stencilRef,
                                   int stencilBackRef,
                                   bool frontFaceCCW);

    void forceSetBlendState() { mForceSetBlendState = true; }
    void setBlendStateIsDirty() { mBlendStateIsDirty = true; }

    void forceSetDepthStencilState() { mForceSetDepthStencilState = true; }
    void setDepthStencilStateIsDirty() { mDepthStencilStateIsDirty = true; }

    void setCurDepthStencilSize(unsigned int curStencilSize) { mCurStencilSize = curStencilSize; }
    bool stencilSizeChanged(unsigned int size) { return (mCurStencilSize != size); }

  private:
    // Blend State
    bool mForceSetBlendState;
    // TODO(dianx) temporary representation of a dirty bit. once we move enough states in,
    // try experimenting with dirty bit instead of a bool
    bool mBlendStateIsDirty;
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    // Currently applied depth stencil state
    bool mForceSetDepthStencilState;
    bool mDepthStencilStateIsDirty;
    gl::DepthStencilState mCurDepthStencilState;
    int mCurStencilRef;
    int mCurStencilBackRef;
    unsigned int mCurStencilSize;

    Renderer11 *mRenderer11;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D11_STATEMANAGER11_H_