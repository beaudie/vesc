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
      mCurStencilSize(0),
      mBlendColor(0, 0, 0, 0),
      mStencilRef(0),
      mStencilBackRef(0)
{
    mBlendState.blend                 = false;
    mBlendState.sourceBlendRGB        = GL_ONE;
    mBlendState.destBlendRGB          = GL_ZERO;
    mBlendState.sourceBlendAlpha      = GL_ONE;
    mBlendState.destBlendAlpha        = GL_ZERO;
    mBlendState.blendEquationRGB      = GL_FUNC_ADD;
    mBlendState.blendEquationAlpha    = GL_FUNC_ADD;
    mBlendState.colorMaskRed          = true;
    mBlendState.colorMaskBlue         = true;
    mBlendState.colorMaskGreen        = true;
    mBlendState.colorMaskAlpha        = true;
    mBlendState.sampleAlphaToCoverage = false;
    mBlendState.dither                = false;

    mDepthStencilState.depthTest                = false;
    mDepthStencilState.depthFunc                = GL_LESS;
    mDepthStencilState.depthMask                = true;
    mDepthStencilState.stencilTest              = false;
    mDepthStencilState.stencilMask              = true;
    mDepthStencilState.stencilFail              = GL_KEEP;
    mDepthStencilState.stencilPassDepthFail     = GL_KEEP;
    mDepthStencilState.stencilPassDepthPass     = GL_KEEP;
    mDepthStencilState.stencilWritemask         = static_cast<GLuint>(-1);
    mDepthStencilState.stencilBackFunc          = GL_ALWAYS;
    mDepthStencilState.stencilBackMask          = static_cast<GLuint>(-1);
    mDepthStencilState.stencilBackFail          = GL_KEEP;
    mDepthStencilState.stencilBackPassDepthFail = GL_KEEP;
    mDepthStencilState.stencilBackPassDepthPass = GL_KEEP;
    mDepthStencilState.stencilBackWritemask     = static_cast<GLuint>(-1);

    mRasterizerState.rasterizerDiscard   = false;
    mRasterizerState.cullFace            = false;
    mRasterizerState.cullMode            = GL_BACK;
    mRasterizerState.frontFace           = GL_CCW;
    mRasterizerState.polygonOffsetFill   = false;
    mRasterizerState.polygonOffsetFactor = 0.0f;
    mRasterizerState.polygonOffsetUnits  = 0.0f;
    mRasterizerState.pointDrawMode       = false;
    mRasterizerState.multiSample         = false;
}

StateManagerD3D::~StateManagerD3D()
{
}

const gl::State::DirtyBitType StateManagerD3D::mBlendStateDirtyBits[] = {
    gl::State::DIRTY_BIT_BLEND_EQUATIONS, gl::State::DIRTY_BIT_BLEND_FUNCS,
    gl::State::DIRTY_BIT_BLEND_ENABLED,   gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED,
    gl::State::DIRTY_BIT_DITHER_ENABLED,  gl::State::DIRTY_BIT_COLOR_MASK,
    gl::State::DIRTY_BIT_BLEND_COLOR};

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

const gl::State::DirtyBitType StateManagerD3D::mRasterizerStateBits[] = {
    gl::State::DIRTY_BIT_CULL_FACE_ENABLED, gl::State::DIRTY_BIT_CULL_FACE,
    gl::State::DIRTY_BIT_FRONT_FACE,        gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED,
    gl::State::DIRTY_BIT_POLYGON_OFFSET,    gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED};

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

bool StateManagerD3D::isRasterizerStateDirty(const gl::State::DirtyBits &dirtyBits)
{
    for (auto bitType : mRasterizerStateBits)
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

void StateManagerD3D::setCurDepthSize(unsigned int size)
{
    mCurDepthSize = size;
}

void StateManagerD3D::setRasterizerScissorEnabled(bool scissorEnabled)
{
    mScissorEnabled = scissorEnabled;
}

const gl::RasterizerState &StateManagerD3D::getCurRasterizerState()
{
    return mRasterizerState;
}

unsigned int StateManagerD3D::getBlendStateMask(const gl::Framebuffer *framebufferObject,
                                                int samples,
                                                const gl::State &state) const
{
    unsigned int mask = 0;
    if (state.isSampleCoverageEnabled())
    {
        GLclampf coverageValue = state.getSampleCoverageValue();
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

        bool coverageInvert = state.getSampleCoverageInvert();
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

void StateManagerD3D::forceSetRasterizerState()
{
    mForceSetRasterizerState = true;
}

gl::Error StateManagerD3D::syncState(const gl::Data &data, const gl::State::DirtyBits &dirtyBits)
{
    gl::Error error = gl::Error(GL_NO_ERROR);

    const gl::Framebuffer *framebufferObject = data.state->getDrawFramebuffer();
    int samples                              = framebufferObject->getSamples(data);

    unsigned int sampleMask = getBlendStateMask(framebufferObject, samples, *(data.state));

    error = setBlendState(framebufferObject, data.state->getBlendState(),
                          data.state->getBlendColor(), sampleMask, dirtyBits);

    if (error.isError())
    {
        return error;
    }

    error = setDepthStencilState(data.state->getDepthStencilState(), data.state->getStencilRef(),
                                 data.state->getStencilBackRef(),
                                 data.state->getRasterizerState().frontFace == GL_CCW, dirtyBits);

    if (error.isError())
    {
        return error;
    }

    return error;
}
}