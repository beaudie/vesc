//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d11/StateManagerD3D11.h"

namespace rx
{

StateManagerD3D11::StateManagerD3D11(ID3D11DeviceContext *deviceContext,
                                     RenderStateCache &stateCache)
    : mDeviceContext(deviceContext), mStateCache(stateCache)
{
}

StateManagerD3D11::~StateManagerD3D11()
{
}

// TODO(dianx) Separate out the blend and stencil states so that each dirty bit
// changes only one thing rather than all for D3D11
gl::Error StateManagerD3D11::setBlendState(const gl::Framebuffer *framebuffer,
                                           const gl::BlendState &blendState,
                                           const gl::ColorF &blendColor,
                                           unsigned int sampleMask,
                                           const gl::State::DirtyBits &dirtyBits)
{
    if (mForceSetBlendState ||
        (isBlendStateDirty(dirtyBits) &&
         (memcmp(&blendState, &mBlendState, sizeof(gl::BlendState)) != 0 ||
          memcmp(&blendColor, &mBlendColor, sizeof(gl::ColorF)) != 0 || sampleMask != mSampleMask)))
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

        mBlendState = blendState;
        mBlendColor = blendColor;
        mSampleMask = sampleMask;

        mForceSetBlendState = false;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManagerD3D11::setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                                  int stencilRef,
                                                  int stencilBackRef,
                                                  bool frontFaceCCW,
                                                  const gl::State::DirtyBits &dirtyBits)
{
    if (mForceSetDepthStencilState ||
        (isDepthStencilStateDirty(dirtyBits) &&
         (memcmp(&depthStencilState, &mDepthStencilState, sizeof(gl::DepthStencilState)) != 0 ||
          stencilRef != mStencilRef || stencilBackRef != mStencilBackRef)))
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

        mForceSetDepthStencilState = false;
        mDepthStencilState         = depthStencilState;
        mStencilRef                = stencilRef;
        mStencilBackRef            = stencilBackRef;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManagerD3D11::setRasterizerState(const gl::RasterizerState &rasterizerState,
                                                const gl::State::DirtyBits &dirtyBits)
{
    if (mForceSetRasterizerState ||
        (isRasterizerStateDirty(dirtyBits) &&
         (memcmp(&rasterizerState, &mRasterizerState, sizeof(gl::RasterizerState)) != 0)))
    {
        ID3D11RasterizerState *dxRasterState = NULL;
        gl::Error error =
            mStateCache.getRasterizerState(rasterizerState, mScissorEnabled, &dxRasterState);
        if (error.isError())
        {
            return error;
        }

        mDeviceContext->RSSetState(dxRasterState);

        mRasterizerState = rasterizerState;

        mForceSetRasterizerState = false;
    }

    return gl::Error(GL_NO_ERROR);
}
}