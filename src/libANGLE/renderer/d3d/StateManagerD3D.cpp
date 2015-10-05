//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D.cpp: Defines a class for caching D3D state

#include "libANGLE/renderer/d3d/StateManagerD3D.h"

#include "common/BitSetIterator.h"
#include "libANGLE/Framebuffer.h"

namespace rx
{

StateManagerD3D::StateManagerD3D()
    : mForceSetBlendState(false),
      mForceSetDepthStencilState(false),
      mSampleMask(0),
      mCurStencilSize(0)
{
}

StateManagerD3D::~StateManagerD3D()
{
}

const gl::State::DirtyBitType StateManagerD3D::mBlendStateDirtyBits[] = {
    gl::State::DIRTY_BIT_BLEND_EQUATIONS,
    gl::State::DIRTY_BIT_BLEND_FUNCS,
    gl::State::DIRTY_BIT_BLEND_ENABLED,   
    gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED,
    gl::State::DIRTY_BIT_DITHER_ENABLED,
    gl::State::DIRTY_BIT_COLOR_MASK,
    gl::State::DIRTY_BIT_BLEND_COLOR
};

const gl::State::DirtyBitType StateManagerD3D::mDepthStencilStateBits[] = {
    gl::State::DIRTY_BIT_DEPTH_MASK,
    gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED,
    gl::State::DIRTY_BIT_DEPTH_FUNC,
    gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED,
    gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT,
    gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK,
    gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT,
    gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK,
    gl::State::DIRTY_BIT_STENCIL_OPS_FRONT,
    gl::State::DIRTY_BIT_STENCIL_OPS_BACK,
};

bool StateManagerD3D::isBlendStateDirty(const gl::State::DirtyBits &dirtyBits)
{
    for (auto bitType : mBlendStateDirtyBits)
    {
        if (dirtyBits.test(bitType))
        {
            return true;
        }
    }

    return false;
}

bool StateManagerD3D::isDepthStencilStateDirty(const gl::State::DirtyBits &dirtyBits)
{
    for (auto bitType : mDepthStencilStateBits)
    {
        if (dirtyBits.test(bitType))
        {
            return true;
        }
    }

    return false;
}

void StateManagerD3D::setCurStencilSize(unsigned int size)
{
    mCurStencilSize = size;
}

unsigned int StateManagerD3D::getBlendStateMask(const gl::Data &data) const
{
    // TODO(dianx) see if we can use samples already calculated before this call
    const gl::Framebuffer *framebufferObject = data.state->getDrawFramebuffer();
    int samples                              = framebufferObject->getSamples(data);

    unsigned int mask = 0;
    if (data.state->isSampleCoverageEnabled())
    {
        GLclampf coverageValue = data.state->getSampleCoverageValue();
        if (coverageValue != 0)
        {
            float threshold = 0.5f;

            for (int i = 0; i < samples; ++i)
            {
                mask <<= 1;

                if ((i + 1) * coverageValue >= threshold)
                {
                    threshold += 1.0f;
                    mask |= 1;
                }
            }
        }

        bool coverageInvert = data.state->getSampleCoverageInvert();
        if (coverageInvert)
        {
            mask = ~mask;
        }
    }
    else
    {
        mask = 0xFFFFFFFF;
    }

    return mask;
}

void StateManagerD3D::forceSetBlendState()
{
    mForceSetBlendState = true;
}

void StateManagerD3D::forceSetDepthStencilState()
{
    mForceSetDepthStencilState = true;
}

gl::Error StateManagerD3D::syncState(const gl::Data &data, const gl::State::DirtyBits &dirtyBits)
{
    gl::Error error = gl::Error(GL_NO_ERROR);

    unsigned int sampleMask = getBlendStateMask(data);

    error = setBlendState(data.state->getDrawFramebuffer(), 
                          data.state->getBlendState(),
                          data.state->getBlendColor(), 
                          sampleMask, 
                          dirtyBits);

    if (error.isError())
    {
        return error;
    }

    error = setDepthStencilState(data.state->getDepthStencilState(),
                                 data.state->getStencilRef(),
                                 data.state->getStencilBackRef(),
                                 data.state->getRasterizerState().frontFace == GL_CCW, 
                                 dirtyBits);

    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}
}