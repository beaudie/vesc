//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D.h: Defines a class for caching D3D state

#ifndef LIBANGLE_RENDERER_D3D_STATEMANAGERD3D_H_
#define LIBANGLE_RENDERER_D3D_STATEMANAGERD3D_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"

namespace rx
{

class StateManagerD3D : angle::NonCopyable
{
  public:
    explicit StateManagerD3D();

    virtual ~StateManagerD3D();

    virtual gl::Error syncState(const gl::Data &data, const gl::State::DirtyBits &dirtyBits);

    virtual gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                                    const gl::BlendState &blendState,
                                    const gl::ColorF &blendColor,
                                    unsigned int sampleMask,
                                    const gl::State::DirtyBits &dirtyBits) = 0;

    virtual gl::Error setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                           int stencilRef,
                                           int stencilBackRef,
                                           bool frontFaceCCW,
                                           const gl::State::DirtyBits &dirtyBits) = 0;

    virtual gl::Error setRasterizerState(const gl::RasterizerState &rasterizerState,
                                         const gl::State::DirtyBits &dirtyBits) = 0;

    virtual void setCurStencilSize(unsigned int size);
    void setCurDepthSize(unsigned int size);
    void setRasterizerScissorEnabled(bool scissorEnabled);

    void forceSetBlendState();
    void forceSetDepthStencilState();
    void forceSetRasterizerState();

    const gl::RasterizerState &getCurRasterizerState();

  protected:
    static bool isBlendStateDirty(const gl::State::DirtyBits &dirtyBits);
    static bool isDepthStencilStateDirty(const gl::State::DirtyBits &dirtyBits);
    static bool isRasterizerStateDirty(const gl::State::DirtyBits &dirtyBits);

    bool mForceSetBlendState;
    bool mForceSetDepthStencilState;
    bool mForceSetRasterizerState;

    // Blend State
    gl::BlendState mBlendState;
    gl::ColorF mBlendColor;
    unsigned int mSampleMask;

    // Depth Stencil State
    gl::DepthStencilState mDepthStencilState;
    int mStencilRef;
    int mStencilBackRef;
    unsigned int mCurStencilSize;

    // RasterizserState
    gl::RasterizerState mRasterizerState;
    bool mScissorEnabled;
    unsigned int mCurDepthSize;

  private:
    unsigned int getBlendStateMask(const gl::Framebuffer *framebufferObject,
                                   int samples,
                                   const gl::State &state) const;

    static const gl::State::DirtyBitType mBlendStateDirtyBits[];
    static const gl::State::DirtyBitType mDepthStencilStateBits[];
    static const gl::State::DirtyBitType mRasterizerStateBits[];
};
}
#endif