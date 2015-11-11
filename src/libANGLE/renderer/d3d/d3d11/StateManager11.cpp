//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"

namespace rx
{

StateManager11::StateManager11(ID3D11DeviceContext *deviceContext, RenderStateCache *stateCache)
    : mDeviceContext(deviceContext), mStateCache(stateCache)
{
}

StateManager11::~StateManager11()
{
}

// TODO(dianx) Separate out the blend and stencil states so that each dirty bit
// changes only one thing rather than all for D3D11
gl::Error StateManager11::setBlendState(const gl::Framebuffer *framebuffer,
                                        const gl::BlendState &blendState,
                                        const gl::ColorF &blendColor,
                                        unsigned int sampleMask,
                                        const gl::State::DirtyBits &dirtyBits)
{
    if (mForceSetBlendState || sampleMask != mCurSampleMask ||
        (IsBlendStateDirty(mExternalDirtyBits) &&
             memcmp(&blendState, &mCurBlendState, sizeof(gl::BlendState)) != 0 ||
         memcmp(&blendColor, &mCurBlendColor, sizeof(gl::ColorF)) != 0))
    {
        ID3D11BlendState *dxBlendState = NULL;
        gl::Error error = mStateCache->getBlendState(framebuffer, blendState, &dxBlendState);
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

    mForceSetBlendState = false;

    return gl::Error(GL_NO_ERROR);
}

}  // namespace rx
