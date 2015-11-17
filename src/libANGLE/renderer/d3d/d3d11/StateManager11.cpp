//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"

#include "common/BitSetIterator.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

namespace rx
{

StateManager11::StateManager11()
    : mBlendStateIsDirty(false),
      mForceSetBlendState(false),
      mCurBlendColor(0, 0, 0, 0),
      mCurSampleMask(0),
      mForceSetDepthStencilState(false),
      mDepthStencilStateIsDirty(false),
      mCurStencilRef(0),
      mCurStencilBackRef(0),
      mCurStencilSize(0),
      mForceSetRasterState(false),
      mRasterStateIsDirty(false),
      mCurScissorEnabled(false)
{
    mCurBlendState.blend                 = false;
    mCurBlendState.sourceBlendRGB        = GL_ONE;
    mCurBlendState.destBlendRGB          = GL_ZERO;
    mCurBlendState.sourceBlendAlpha      = GL_ONE;
    mCurBlendState.destBlendAlpha        = GL_ZERO;
    mCurBlendState.blendEquationRGB      = GL_FUNC_ADD;
    mCurBlendState.blendEquationAlpha    = GL_FUNC_ADD;
    mCurBlendState.colorMaskRed          = true;
    mCurBlendState.colorMaskBlue         = true;
    mCurBlendState.colorMaskGreen        = true;
    mCurBlendState.colorMaskAlpha        = true;
    mCurBlendState.sampleAlphaToCoverage = false;
    mCurBlendState.dither                = false;

    mCurDepthStencilState.depthTest                = false;
    mCurDepthStencilState.depthFunc                = GL_LESS;
    mCurDepthStencilState.depthMask                = true;
    mCurDepthStencilState.stencilTest              = false;
    mCurDepthStencilState.stencilMask              = true;
    mCurDepthStencilState.stencilFail              = GL_KEEP;
    mCurDepthStencilState.stencilPassDepthFail     = GL_KEEP;
    mCurDepthStencilState.stencilPassDepthPass     = GL_KEEP;
    mCurDepthStencilState.stencilWritemask         = static_cast<GLuint>(-1);
    mCurDepthStencilState.stencilBackFunc          = GL_ALWAYS;
    mCurDepthStencilState.stencilBackMask          = static_cast<GLuint>(-1);
    mCurDepthStencilState.stencilBackFail          = GL_KEEP;
    mCurDepthStencilState.stencilBackPassDepthFail = GL_KEEP;
    mCurDepthStencilState.stencilBackPassDepthPass = GL_KEEP;
    mCurDepthStencilState.stencilBackWritemask     = static_cast<GLuint>(-1);

    mCurRasterState.rasterizerDiscard   = false;
    mCurRasterState.cullFace            = false;
    mCurRasterState.cullMode            = GL_BACK;
    mCurRasterState.frontFace           = GL_CCW;
    mCurRasterState.polygonOffsetFill   = false;
    mCurRasterState.polygonOffsetFactor = 0.0f;
    mCurRasterState.polygonOffsetUnits  = 0.0f;
    mCurRasterState.pointDrawMode       = false;
    mCurRasterState.multiSample         = false;
}

StateManager11::~StateManager11()
{
}

void StateManager11::initialize(ID3D11DeviceContext *deviceContext, RenderStateCache *stateCache)
{
    mDeviceContext = deviceContext;
    mStateCache    = stateCache;
}

void StateManager11::updateStencilSizeIfChanged(bool depthStencilInitialized,
                                                unsigned int stencilSize)
{
    if (!depthStencilInitialized || stencilSize != mCurStencilSize)
    {
        mCurStencilSize            = stencilSize;
        mForceSetDepthStencilState = true;
    }
}

void StateManager11::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    for (unsigned int dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        switch (dirtyBit)
        {
            case gl::State::DIRTY_BIT_BLEND_EQUATIONS:
            case gl::State::DIRTY_BIT_BLEND_FUNCS:
            case gl::State::DIRTY_BIT_BLEND_ENABLED:
            case gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
            case gl::State::DIRTY_BIT_DITHER_ENABLED:
            case gl::State::DIRTY_BIT_COLOR_MASK:
            case gl::State::DIRTY_BIT_BLEND_COLOR:
                mBlendStateIsDirty = true;
                break;
            case gl::State::DIRTY_BIT_DEPTH_MASK:
            case gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED:
            case gl::State::DIRTY_BIT_DEPTH_FUNC:
            case gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED:
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT:
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK:
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
            case gl::State::DIRTY_BIT_STENCIL_OPS_FRONT:
            case gl::State::DIRTY_BIT_STENCIL_OPS_BACK:
                mDepthStencilStateIsDirty = true;
                break;
            case gl::State::DIRTY_BIT_CULL_FACE_ENABLED:
            case gl::State::DIRTY_BIT_CULL_FACE:
            case gl::State::DIRTY_BIT_FRONT_FACE:
            case gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
            case gl::State::DIRTY_BIT_POLYGON_OFFSET:
            case gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                mRasterStateIsDirty = true;
                break;
            default:
                break;
        }
    }
}

gl::Error StateManager11::setBlendState(const gl::Framebuffer *framebuffer,
                                        const gl::BlendState &blendState,
                                        const gl::ColorF &blendColor,
                                        unsigned int sampleMask)
{
    if (mForceSetBlendState || sampleMask != mCurSampleMask ||
        (mBlendStateIsDirty && (memcmp(&blendState, &mCurBlendState, sizeof(gl::BlendState)) != 0 ||
                                memcmp(&blendColor, &mCurBlendColor, sizeof(gl::ColorF)) != 0)))
    {
        ID3D11BlendState *dxBlendState = nullptr;
        gl::Error error = mStateCache->getBlendState(framebuffer, blendState, &dxBlendState);
        if (error.isError())
        {
            return error;
        }

        ASSERT(dxBlendState != nullptr);

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

        mForceSetBlendState = false;
        mBlendStateIsDirty  = false;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManager11::setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                               int stencilRef,
                                               int stencilBackRef)
{
    if (mForceSetDepthStencilState ||
        (mDepthStencilStateIsDirty &&
         (memcmp(&depthStencilState, &mCurDepthStencilState, sizeof(gl::DepthStencilState)) != 0 ||
          stencilRef != mCurStencilRef || stencilBackRef != mCurStencilBackRef)))
    {
        // get the maximum size of the stencil ref
        unsigned int maxStencil = 0;
        if (depthStencilState.stencilTest && mCurStencilSize > 0)
        {
            maxStencil = (1 << mCurStencilSize) - 1;
        }
        ASSERT((depthStencilState.stencilWritemask & maxStencil) ==
               (depthStencilState.stencilBackWritemask & maxStencil));
        ASSERT(stencilRef == stencilBackRef);
        ASSERT((depthStencilState.stencilMask & maxStencil) ==
               (depthStencilState.stencilBackMask & maxStencil));

        ID3D11DepthStencilState *dxDepthStencilState = NULL;
        gl::Error error =
            mStateCache->getDepthStencilState(depthStencilState, &dxDepthStencilState);
        if (error.isError())
        {
            return error;
        }

        ASSERT(dxDepthStencilState);

        // Max D3D11 stencil reference value is 0xFF, corresponding to the max 8 bits in a stencil
        // buffer
        // GL specifies we should clamp the ref value to the nearest bit depth when doing stencil
        // ops
        static_assert(D3D11_DEFAULT_STENCIL_READ_MASK == 0xFF,
                      "Unexpected value of D3D11_DEFAULT_STENCIL_READ_MASK");
        static_assert(D3D11_DEFAULT_STENCIL_WRITE_MASK == 0xFF,
                      "Unexpected value of D3D11_DEFAULT_STENCIL_WRITE_MASK");
        UINT dxStencilRef = std::min<UINT>(stencilRef, 0xFFu);

        mDeviceContext->OMSetDepthStencilState(dxDepthStencilState, dxStencilRef);

        mCurDepthStencilState = depthStencilState;
        mCurStencilRef        = stencilRef;
        mCurStencilBackRef    = stencilBackRef;

        mForceSetDepthStencilState = false;
        mDepthStencilStateIsDirty  = false;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error StateManager11::setRasterizerState(const gl::RasterizerState &rasterState)
{
    if (mForceSetRasterState ||
        (mRasterStateIsDirty &&
         memcmp(&rasterState, &mCurRasterState, sizeof(gl::RasterizerState)) != 0))
    {
        ID3D11RasterizerState *dxRasterState = NULL;
        gl::Error error =
            mStateCache->getRasterizerState(rasterState, mCurScissorEnabled, &dxRasterState);
        if (error.isError())
        {
            return error;
        }

        mDeviceContext->RSSetState(dxRasterState);

        mCurRasterState      = rasterState;
        mForceSetRasterState = false;
        mRasterStateIsDirty  = false;
    }

    return gl::Error(GL_NO_ERROR);
}

}  // namespace rx
