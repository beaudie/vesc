//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"

namespace rx
{

StateManager11::StateManager11(ID3D11DeviceContext *deviceContext, RenderStateCache &stateCache)
    : mDeviceContext(deviceContext), mStateCache(stateCache)
{
}

StateManager11::~StateManager11()
{
}

bool StateManager11::isForceSetBlendState() const
{
    return (mLocalDirtyBits & mBlendDirtyBits).any();
}

bool StateManager11::isForceSetDepthStencilState() const
{
    return (mLocalDirtyBits & mDepthStencilDirtyBits).any();
}

bool StateManager11::isForceSetRasterizerState() const
{
    return (mLocalDirtyBits & mRasterizerDirtyBits).any();
}

const gl::RasterizerState &StateManager11::getCurRasterizerState()
{
    return mCurRasterizerState;
}

// TODO(dianx) Separate out the blend and stencil states so that each dirty bit
// changes only one thing rather than all for D3D11
gl::Error StateManager11::setBlendState(const gl::Framebuffer *framebuffer,
                                        const gl::BlendState &blendState,
                                        const gl::ColorF &blendColor,
                                        unsigned int sampleMask,
                                        const gl::State::DirtyBits &dirtyBits)
{
    if (isForceSetBlendState() ||
        (isBlendStateDirty(dirtyBits) &&
         (memcmp(&blendState, &mCurBlendState, sizeof(gl::BlendState)) != 0 ||
          memcmp(&blendColor, &mCurBlendColor, sizeof(gl::ColorF)) != 0 ||
          sampleMask != mCurSampleMask)))
    {
        ID3D11BlendState *dxBlendState = NULL;
        gl::Error error = mStateCache.getBlendState(framebuffer, blendState, &dxBlendState);
        if (error.isError())
        {
            return error;
        }

        ASSERT(dxBlendState != NULL);

        float blendColors[4] = {0.0f};

        if (blendState.sourceBlendRGB != GL_CONSTANT_ALPHA &&
            blendState.sourceBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA &&
            blendState.destBlendRGB != GL_CONSTANT_ALPHA &&
            blendState.destBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA)
        {
            blendColors[0] = blendColor.red;
            blendColors[1] = blendColor.green;
            blendColors[2] = blendColor.blue;
            blendColors[3] = blendColor.alpha;
        }
        else
        {
            blendColors[0] = blendColor.alpha;
            blendColors[1] = blendColor.alpha;
            blendColors[2] = blendColor.alpha;
            blendColors[3] = blendColor.alpha;
        }

        mDeviceContext->OMSetBlendState(dxBlendState, blendColors, sampleMask);

        mCurBlendState = blendState;
        mCurBlendColor = blendColor;
        mCurSampleMask = sampleMask;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManager11::setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                               int stencilRef,
                                               int stencilBackRef,
                                               bool frontFaceCCW,
                                               const gl::State::DirtyBits &dirtyBits)
{
    if (isForceSetDepthStencilState() ||
        (isDepthStencilStateDirty(dirtyBits) &&
         (memcmp(&depthStencilState, &mCurDepthStencilState, sizeof(gl::DepthStencilState)) != 0 ||
          stencilRef != mCurStencilRef || stencilBackRef != mCurStencilBackRef)))
    {
        ASSERT(depthStencilState.stencilWritemask == depthStencilState.stencilBackWritemask);
        ASSERT(stencilRef == stencilBackRef);
        ASSERT(depthStencilState.stencilMask == depthStencilState.stencilBackMask);

        ID3D11DepthStencilState *dxDepthStencilState = NULL;
        gl::Error error = mStateCache.getDepthStencilState(depthStencilState, &dxDepthStencilState);
        if (error.isError())
        {
            return error;
        }

        ASSERT(dxDepthStencilState);

        // Max D3D11 stencil reference value is 0xFF,
        // corresponding to the max 8 bits in a stencil buffer
        // GL specifies we should clamp the ref value to the nearest
        // bit depth when doing stencil ops
        static_assert(D3D11_DEFAULT_STENCIL_READ_MASK == 0xFF,
                      "Unexpected value of D3D11_DEFAULT_STENCIL_READ_MASK");
        static_assert(D3D11_DEFAULT_STENCIL_WRITE_MASK == 0xFF,
                      "Unexpected value of D3D11_DEFAULT_STENCIL_WRITE_MASK");
        UINT dxStencilRef = std::min<UINT>(stencilRef, 0xFFu);

        mDeviceContext->OMSetDepthStencilState(dxDepthStencilState, dxStencilRef);

        mCurDepthStencilState = depthStencilState;
        mCurStencilRef        = stencilRef;
        mCurStencilBackRef    = stencilBackRef;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManager11::setRasterizerState(const gl::RasterizerState &rasterizerState,
                                             const gl::State::DirtyBits &dirtyBits)
{
    const gl::State::DirtyBits allDirtyBits = dirtyBits | mLocalDirtyBits;

    if (isForceSetRasterizerState() ||
        (isRasterizerStateDirty(allDirtyBits) &&
         (memcmp(&rasterizerState, &mCurRasterizerState, sizeof(gl::RasterizerState)) != 0)))
    {
        ID3D11RasterizerState *dxRasterState = NULL;
        gl::Error error =
            mStateCache.getRasterizerState(rasterizerState, mCurScissorTestEnabled, &dxRasterState);
        if (error.isError())
        {
            return error;
        }

        mDeviceContext->RSSetState(dxRasterState);

        mCurRasterizerState = rasterizerState;

        resetRasterizerForceBits();
    }

    return gl::Error(GL_NO_ERROR);
}

}  // namespace rx