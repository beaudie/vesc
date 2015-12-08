//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager9.cpp: Defines a class for caching D3D9 state

#include "libANGLE/renderer/d3d/d3d9/StateManager9.h"

#include "common/BitSetIterator.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"
#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"

namespace rx
{

StateManager9::StateManager9()
    : mCurBlendState(),
      mCurBlendColor(0, 0, 0, 0),
      mCurSampleMask(0),
      mCurDepthStencilState(),
      mCurStencilRef(0),
      mCurStencilBackRef(0),
      mCurFrontFaceCCW(0),
      mCurStencilSize(0),
      mDevice(nullptr),
      mAdapterIdentifier(nullptr),
      mDirtyBits()
{
}

StateManager9::~StateManager9()
{
}

void StateManager9::initialize(IDirect3DDevice9 *device, D3DADAPTER_IDENTIFIER9 *adapterIdentifier)
{
    mDevice            = device;
    mAdapterIdentifier = adapterIdentifier;
}

void StateManager9::forceSetBlendState()
{
    mDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
    mDirtyBits.set(DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE);
    mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
    mDirtyBits.set(DIRTY_BIT_DITHER);
    mDirtyBits.set(DIRTY_BIT_SAMPLE_MASK);
}

void StateManager9::forceSetDepthStencilState()
{
    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_MASK);
    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
    mDirtyBits.set(DIRTY_BIT_STENCIL_TEST_ENABLED);
    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
}

void StateManager9::updateStencilSizeIfChanged(bool depthStencilInitialized,
                                               unsigned int stencilSize)
{
    if (!depthStencilInitialized || stencilSize != mCurStencilSize)
    {
        mCurStencilSize = stencilSize;
        forceSetDepthStencilState();
    }
}

void StateManager9::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    for (unsigned int dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        switch (dirtyBit)
        {
            case gl::State::DIRTY_BIT_BLEND_ENABLED:
                if (state.getBlendState().blend != mCurBlendState.blend)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_ENABLED);
                    // BlendColor and funcs and equations has to be set if blend is enabled
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
                }
                break;
            case gl::State::DIRTY_BIT_BLEND_FUNCS:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.sourceBlendRGB != mCurBlendState.sourceBlendRGB ||
                    blendState.destBlendRGB != mCurBlendState.destBlendRGB ||
                    blendState.sourceBlendAlpha != mCurBlendState.sourceBlendAlpha ||
                    blendState.destBlendAlpha != mCurBlendState.destBlendAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
                    // BlendColor depends on the values of blend funcs
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
                }
                break;
            }
            case gl::State::DIRTY_BIT_BLEND_EQUATIONS:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.blendEquationRGB != mCurBlendState.blendEquationRGB ||
                    blendState.blendEquationAlpha != mCurBlendState.blendEquationAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_FUNCS_EQUATIONS);
                }
                break;
            }
            case gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                if (state.getBlendState().sampleAlphaToCoverage !=
                    mCurBlendState.sampleAlphaToCoverage)
                {
                    mDirtyBits.set(DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE);
                }
                break;
            case gl::State::DIRTY_BIT_COLOR_MASK:
            {
                const gl::BlendState &blendState = state.getBlendState();
                if (blendState.colorMaskRed != mCurBlendState.colorMaskRed ||
                    blendState.colorMaskGreen != mCurBlendState.colorMaskGreen ||
                    blendState.colorMaskBlue != mCurBlendState.colorMaskBlue ||
                    blendState.colorMaskAlpha != mCurBlendState.colorMaskAlpha)
                {
                    mDirtyBits.set(DIRTY_BIT_COLOR_MASK);
                }
                break;
            }
            case gl::State::DIRTY_BIT_DITHER_ENABLED:
                if (state.getBlendState().dither != mCurBlendState.dither)
                {
                    mDirtyBits.set(DIRTY_BIT_DITHER);
                }
                break;
            case gl::State::DIRTY_BIT_BLEND_COLOR:
                if (state.getBlendColor() != mCurBlendColor)
                {
                    mDirtyBits.set(DIRTY_BIT_BLEND_COLOR);
                }
                break;
            case gl::State::DIRTY_BIT_DEPTH_MASK:
                if (state.getDepthStencilState().depthMask != mCurDepthStencilState.depthMask)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_MASK);
                }
                break;
            case gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED:
                if (state.getDepthStencilState().depthTest != mCurDepthStencilState.depthTest)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
                }
                break;
            case gl::State::DIRTY_BIT_DEPTH_FUNC:
                if (state.getDepthStencilState().depthFunc != mCurDepthStencilState.depthFunc)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_DEPTH_FUNC);
                }
                break;
            case gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED:
                if (state.getDepthStencilState().stencilTest != mCurDepthStencilState.stencilTest)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_TEST_ENABLED);
                    // If we enable the stencil test, all of these must be set
                    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
                    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
                    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
                    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
                    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
                    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
                }
                break;
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT:
            {
                const gl::DepthStencilState &depthStencilState = state.getDepthStencilState();
                if (depthStencilState.stencilFunc != mCurDepthStencilState.stencilFunc ||
                    depthStencilState.stencilMask != mCurDepthStencilState.stencilMask ||
                    state.getStencilRef() != mCurStencilRef)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_FRONT);
                }
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK:
            {
                const gl::DepthStencilState &depthStencilState = state.getDepthStencilState();
                if (depthStencilState.stencilBackFunc != mCurDepthStencilState.stencilBackFunc ||
                    depthStencilState.stencilBackMask != mCurDepthStencilState.stencilBackMask ||
                    state.getStencilBackRef() != mCurStencilBackRef)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_FUNCS_BACK);
                }
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                if (state.getDepthStencilState().stencilWritemask !=
                    mCurDepthStencilState.stencilWritemask)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
                }
                break;
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                if (state.getDepthStencilState().stencilBackWritemask !=
                    mCurDepthStencilState.stencilBackWritemask)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_WRITEMASK_BACK);
                }
                break;
            case gl::State::DIRTY_BIT_STENCIL_OPS_FRONT:
            {
                const gl::DepthStencilState &depthStencilState = state.getDepthStencilState();
                if (depthStencilState.stencilFail != mCurDepthStencilState.stencilFail ||
                    depthStencilState.stencilPassDepthFail !=
                        mCurDepthStencilState.stencilPassDepthFail ||
                    depthStencilState.stencilPassDepthPass !=
                        mCurDepthStencilState.stencilPassDepthPass)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_FRONT);
                }
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_OPS_BACK:
            {
                const gl::DepthStencilState &depthStencilState = state.getDepthStencilState();
                if (depthStencilState.stencilBackFail != mCurDepthStencilState.stencilBackFail ||
                    depthStencilState.stencilBackPassDepthFail !=
                        mCurDepthStencilState.stencilBackPassDepthFail ||
                    depthStencilState.stencilBackPassDepthPass !=
                        mCurDepthStencilState.stencilBackPassDepthPass)
                {
                    mDirtyBits.set(DIRTY_BIT_STENCIL_OPS_BACK);
                }
                break;
            }
            default:
                break;
        }
    }
}

gl::Error StateManager9::setBlendState(const gl::Framebuffer *framebuffer,
                                       const gl::BlendState &blendState,
                                       const gl::ColorF &blendColor,
                                       unsigned int sampleMask)
{
    for (unsigned int dirtyBit : angle::IterateBitSet(mDirtyBits))
    {
        switch (dirtyBit)
        {
            case DIRTY_BIT_BLEND_ENABLED:
                setBlendEnabled(blendState.blend);
                break;
            case DIRTY_BIT_BLEND_COLOR:
                setBlendColor(blendState, blendColor);
                break;
            case DIRTY_BIT_BLEND_FUNCS_EQUATIONS:
                setBlendFuncsEquations(blendState);
                break;
            case DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE:
                setSampleAlphaToCoverage(blendState.sampleAlphaToCoverage);
                break;
            case DIRTY_BIT_COLOR_MASK:
                setColorMask(framebuffer, blendState.colorMaskRed, blendState.colorMaskBlue,
                             blendState.colorMaskGreen, blendState.colorMaskAlpha);
                break;
            case DIRTY_BIT_DITHER:
                setDither(blendState.dither);
                break;
            default:
                break;
        }
    }

    if (sampleMask != mCurSampleMask)
    {
        setSampleMask(sampleMask);
    }

    return gl::Error(GL_NO_ERROR);
}

void StateManager9::setDepthFunc(bool depthTest, GLenum depthFunc)
{
    if (depthTest)
    {
        mDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
        mDevice->SetRenderState(D3DRS_ZFUNC, gl_d3d9::ConvertComparison(depthFunc));
    }
    else
    {
        mDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    }

    mCurDepthStencilState.depthTest = depthTest;
    mCurDepthStencilState.depthFunc = depthFunc;
}

gl::Error StateManager9::setDepthStencilState(const gl::State &glState)
{
    const auto &depthStencilState = glState.getDepthStencilState();
    /*int stencilRef                = glState.getStencilRef();
    int stencilBackRef            = glState.getStencilBackRef();*/
    bool frontFaceCCW       = (glState.getRasterizerState().frontFace == GL_CCW);
    unsigned int maxStencil = (1 << mCurStencilSize) - 1;

    // All the depth stencil states depends on the front face ccw variable
    if (frontFaceCCW != mCurFrontFaceCCW)
    {
        forceSetDepthStencilState();
        mCurFrontFaceCCW = frontFaceCCW;
    }

    for (unsigned int dirtyBit : angle::IterateBitSet(mDirtyBits))
    {
        switch (dirtyBit)
        {
            case DIRTY_BIT_STENCIL_DEPTH_MASK:
                setDepthMask(depthStencilState.depthMask);
                break;
            case DIRTY_BIT_STENCIL_DEPTH_FUNC:
                setDepthFunc(depthStencilState.depthTest, depthStencilState.depthFunc);
                break;
            case DIRTY_BIT_STENCIL_TEST_ENABLED:
                setStencilTestEnabled(depthStencilState.stencilTest);
                break;
            case DIRTY_BIT_STENCIL_FUNCS_FRONT:
                setStencilFuncsFront(depthStencilState.stencilFunc, depthStencilState.stencilMask,
                                     glState.getStencilRef(), frontFaceCCW, maxStencil);
                break;
            case DIRTY_BIT_STENCIL_FUNCS_BACK:
                setStencilFuncsBack(depthStencilState.stencilBackFunc,
                                    depthStencilState.stencilBackMask, glState.getStencilBackRef(),
                                    frontFaceCCW, maxStencil);
                break;
            case DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                setStencilWriteMask(depthStencilState.stencilWritemask, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                setStencilBackWriteMask(depthStencilState.stencilBackWritemask, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_OPS_FRONT:
                setStencilOpsFront(depthStencilState.stencilFail,
                                   depthStencilState.stencilPassDepthFail,
                                   depthStencilState.stencilPassDepthPass, frontFaceCCW);
                break;
            case DIRTY_BIT_STENCIL_OPS_BACK:
                setStencilOpsBack(depthStencilState.stencilBackFail,
                                  depthStencilState.stencilBackPassDepthFail,
                                  depthStencilState.stencilBackPassDepthPass, frontFaceCCW);
                break;
            default:
                break;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void StateManager9::setStencilOpsFront(GLenum stencilFail,
                                       GLenum stencilPassDepthFail,
                                       GLenum stencilPassDepthPass,
                                       bool frontFaceCCW)
{
    // TODO(dianx) It may be slightly more efficient todo these and other similar areas
    // with separate dirty bits.
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                            gl_d3d9::ConvertStencilOp(stencilFail));
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                            gl_d3d9::ConvertStencilOp(stencilPassDepthFail));
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                            gl_d3d9::ConvertStencilOp(stencilPassDepthPass));

    mCurDepthStencilState.stencilFail          = stencilFail;
    mCurDepthStencilState.stencilPassDepthFail = stencilPassDepthFail;
    mCurDepthStencilState.stencilPassDepthPass = stencilPassDepthPass;
}

void StateManager9::setStencilOpsBack(GLenum stencilBackFail,
                                      GLenum stencilBackPassDepthFail,
                                      GLenum stencilBackPassDepthPass,
                                      bool frontFaceCCW)
{
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                            gl_d3d9::ConvertStencilOp(stencilBackFail));
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                            gl_d3d9::ConvertStencilOp(stencilBackPassDepthFail));
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                            gl_d3d9::ConvertStencilOp(stencilBackPassDepthPass));

    mCurDepthStencilState.stencilBackFail          = stencilBackFail;
    mCurDepthStencilState.stencilBackPassDepthFail = stencilBackPassDepthFail;
    mCurDepthStencilState.stencilBackPassDepthPass = stencilBackPassDepthPass;
}

void StateManager9::setStencilBackWriteMask(GLuint stencilBackWriteMask, bool frontFaceCCW)
{
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK,
                            stencilBackWriteMask);

    mCurDepthStencilState.stencilBackWritemask = stencilBackWriteMask;
}

void StateManager9::setStencilFuncsBack(GLenum stencilBackFunc,
                                        GLuint stencilBackMask,
                                        GLint stencilBackRef,
                                        bool frontFaceCCW,
                                        unsigned int maxStencil)
{
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                            gl_d3d9::ConvertComparison(stencilBackFunc));
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                            (stencilBackRef < (int)maxStencil) ? stencilBackRef : maxStencil);
    mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK,
                            stencilBackMask);

    mCurDepthStencilState.stencilBackFunc = stencilBackFunc;
    mCurStencilBackRef                    = stencilBackRef;
    mCurDepthStencilState.stencilBackMask = stencilBackMask;
}

void StateManager9::setStencilWriteMask(GLuint stencilWriteMask, bool frontFaceCCW)
{
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK,
                            stencilWriteMask);
    mCurDepthStencilState.stencilWritemask = stencilWriteMask;
}

void StateManager9::setStencilFuncsFront(GLenum stencilFunc,
                                         GLuint stencilMask,
                                         GLint stencilRef,
                                         bool frontFaceCCW,
                                         unsigned int maxStencil)
{
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                            gl_d3d9::ConvertComparison(stencilFunc));
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                            (stencilRef < (int)maxStencil) ? stencilRef : maxStencil);
    mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK, stencilMask);

    mCurDepthStencilState.stencilFunc = stencilFunc;
    mCurStencilRef                    = stencilRef;
    mCurDepthStencilState.stencilMask = stencilMask;
}
void StateManager9::setStencilTestEnabled(bool stencilTestEnabled)
{
    if (stencilTestEnabled && mCurStencilSize > 0)
    {
        mDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
    }
    else
    {
        mDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }

    mCurDepthStencilState.stencilTest = stencilTestEnabled;
}

void StateManager9::setDepthMask(bool depthMask)
{
    mDevice->SetRenderState(D3DRS_ZWRITEENABLE, depthMask ? TRUE : FALSE);
    mCurDepthStencilState.depthMask = depthMask;
}

VendorID StateManager9::getVendorId() const
{
    return static_cast<VendorID>(mAdapterIdentifier->VendorId);
}

// TODO(dianx) one bit for sampleAlphaToCoverage
void StateManager9::setSampleAlphaToCoverage(bool enabled)
{
    if (enabled)
    {
        FIXME("Sample alpha to coverage is unimplemented.");
    }
}

void StateManager9::setBlendColor(const gl::BlendState &blendState, const gl::ColorF &blendColor)
{
    // if .blend changed, or src/dest blendRGB changed or blendColor changed
    if (blendState.blend)
    {
        if (blendState.sourceBlendRGB != GL_CONSTANT_ALPHA &&
            blendState.sourceBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA &&
            blendState.destBlendRGB != GL_CONSTANT_ALPHA &&
            blendState.destBlendRGB != GL_ONE_MINUS_CONSTANT_ALPHA)
        {
            mDevice->SetRenderState(D3DRS_BLENDFACTOR, gl_d3d9::ConvertColor(blendColor));
        }
        else
        {
            mDevice->SetRenderState(
                D3DRS_BLENDFACTOR,
                D3DCOLOR_RGBA(gl::unorm<8>(blendColor.alpha), gl::unorm<8>(blendColor.alpha),
                              gl::unorm<8>(blendColor.alpha), gl::unorm<8>(blendColor.alpha)));
        }
        mCurBlendColor = blendColor;
    }
}

void StateManager9::setBlendFuncsEquations(const gl::BlendState &blendState)
{
    if (blendState.blend)
    {
        mDevice->SetRenderState(D3DRS_SRCBLEND,
                                gl_d3d9::ConvertBlendFunc(blendState.sourceBlendRGB));
        mDevice->SetRenderState(D3DRS_DESTBLEND,
                                gl_d3d9::ConvertBlendFunc(blendState.destBlendRGB));
        mDevice->SetRenderState(D3DRS_BLENDOP,
                                gl_d3d9::ConvertBlendOp(blendState.blendEquationRGB));

        if (blendState.sourceBlendRGB != blendState.sourceBlendAlpha ||
            blendState.destBlendRGB != blendState.destBlendAlpha ||
            blendState.blendEquationRGB != blendState.blendEquationAlpha)
        {
            mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

            mDevice->SetRenderState(D3DRS_SRCBLENDALPHA,
                                    gl_d3d9::ConvertBlendFunc(blendState.sourceBlendAlpha));
            mDevice->SetRenderState(D3DRS_DESTBLENDALPHA,
                                    gl_d3d9::ConvertBlendFunc(blendState.destBlendAlpha));
            mDevice->SetRenderState(D3DRS_BLENDOPALPHA,
                                    gl_d3d9::ConvertBlendOp(blendState.blendEquationAlpha));
        }
        else
        {
            mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
        }

        mCurBlendState.sourceBlendRGB     = blendState.sourceBlendRGB;
        mCurBlendState.destBlendRGB       = blendState.destBlendRGB;
        mCurBlendState.blendEquationRGB   = blendState.blendEquationRGB;
        mCurBlendState.blendEquationAlpha = blendState.blendEquationAlpha;
    }
}

void StateManager9::setBlendEnabled(bool enabled)
{
    mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enabled ? TRUE : FALSE);
    mCurBlendState.blend = enabled;
}

void StateManager9::setDither(bool dither)
{
    mDevice->SetRenderState(D3DRS_DITHERENABLE, dither ? TRUE : FALSE);
    mCurBlendState.dither = dither;
}

// TODO(dianx) one bit for color mask
void StateManager9::setColorMask(const gl::Framebuffer *framebuffer,
                                 bool red,
                                 bool blue,
                                 bool green,
                                 bool alpha)
{
    // Set the color mask
    bool zeroColorMaskAllowed = getVendorId() != VENDOR_ID_AMD;
    // Apparently some ATI cards have a bug where a draw with a zero color
    // write mask can cause later draws to have incorrect results. Instead,
    // set a nonzero color write mask but modify the blend state so that no
    // drawing is done.
    // http://code.google.com/p/angleproject/issues/detail?id=169

    const gl::FramebufferAttachment *attachment = framebuffer->getFirstColorbuffer();
    GLenum internalFormat                       = attachment ? attachment->getInternalFormat() : GL_NONE;

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
    DWORD colorMask                      = gl_d3d9::ConvertColorMask(
        formatInfo.redBits > 0 && red, formatInfo.greenBits > 0 && green,
        formatInfo.blueBits > 0 && blue, formatInfo.alphaBits > 0 && alpha);
    if (colorMask == 0 && !zeroColorMaskAllowed)
    {
        // Enable green channel, but set blending so nothing will be drawn.
        mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_GREEN);
        mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        mDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        mDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    }
    else
    {
        mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorMask);
    }

    mCurBlendState.colorMaskRed   = red;
    mCurBlendState.colorMaskGreen = green;
    mCurBlendState.colorMaskBlue  = blue;
    mCurBlendState.colorMaskAlpha = alpha;
}

void StateManager9::setSampleMask(unsigned int sampleMask)
{
    // Set the multisample mask
    mDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    mDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, static_cast<DWORD>(sampleMask));

    mCurSampleMask = sampleMask;
}
}