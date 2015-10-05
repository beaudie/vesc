//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d9/StateManagerD3D9.h"

#include "common/BitSetIterator.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

namespace rx
{

StateManagerD3D9::StateManagerD3D9(IDirect3DDevice9 *device,
                                   D3DADAPTER_IDENTIFIER9 &adapterIdentifier)
    : mDevice(device), mAdapterIdentifier(adapterIdentifier), mMaxStencil(0)
{
}

StateManagerD3D9::~StateManagerD3D9()
{
}

VendorID StateManagerD3D9::getVendorId() const
{
    return static_cast<VendorID>(mAdapterIdentifier.VendorId);
}

void StateManagerD3D9::setCurStencilSize(unsigned int size)
{
    mCurStencilSize = size;
    mMaxStencil     = (1 << mCurStencilSize) - 1;
}

gl::Error StateManagerD3D9::setBlendState(const gl::Framebuffer *framebuffer,
                                          const gl::BlendState &blendState,
                                          const gl::ColorF &blendColor,
                                          unsigned int sampleMask,
                                          const gl::State::DirtyBits &dirtyBits)
{
    if (dirtyBits.test(gl::State::DIRTY_BIT_BLEND_ENABLED) ||
        dirtyBits.test(gl::State::DIRTY_BIT_BLEND_FUNCS) ||
        dirtyBits.test(gl::State::DIRTY_BIT_BLEND_EQUATIONS) || mForceSetBlendState)
    {
        setBlendEnableFuncsEquations(blendState, blendColor);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_BLEND_COLOR) || mForceSetBlendState)
    {
        setBlendColor(blendColor, blendState);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED) ||
        mForceSetBlendState)
    {
        setSampleAlphaToCoverageEnabled(blendState.sampleAlphaToCoverage);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_DITHER_ENABLED) || mForceSetBlendState)
    {
        setDitherEnabled(blendState.dither);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_COLOR_MASK) || mForceSetBlendState)
    {
        setBlendColorMask(blendState, framebuffer);
    }

    if (mSampleMask != sampleMask || mForceSetBlendState)
    {
        setSampleMask(sampleMask);
    }

    mForceSetBlendState = false;

    return gl::Error(GL_NO_ERROR);
}

void StateManagerD3D9::setBlendEnableFuncsEquations(const gl::BlendState &blendState,
                                                    const gl::ColorF &blendColor)
{
    if (blendState.blend != mBlendState.blend ||
        blendState.sourceBlendRGB != mBlendState.sourceBlendRGB ||
        blendState.destBlendRGB != mBlendState.destBlendRGB ||
        blendState.sourceBlendAlpha != mBlendState.sourceBlendAlpha ||
        blendState.destBlendAlpha != mBlendState.destBlendAlpha ||
        blendState.blendEquationRGB != mBlendState.blendEquationRGB ||
        blendState.blendEquationAlpha != mBlendState.blendEquationAlpha)
    {
        if (blendState.blend)
        {
            mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

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

            mBlendState.sourceBlendRGB   = blendState.sourceBlendRGB;
            mBlendState.destBlendRGB     = blendState.destBlendRGB;
            mBlendState.sourceBlendAlpha = blendState.sourceBlendAlpha;
            mBlendState.destBlendAlpha   = blendState.destBlendAlpha;
        }
        else
        {
            mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }

        mBlendState.blend = blendState.blend;
    }
}

void StateManagerD3D9::setBlendColor(const gl::ColorF &blendColor, const gl::BlendState &blendState)
{
    if (blendState.blend &&
        (blendColor.red != mBlendColor.red || blendColor.green != mBlendColor.green ||
         blendColor.blue != mBlendColor.blue || blendColor.alpha != mBlendColor.alpha))
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

        mBlendColor.alpha = blendColor.alpha;
        mBlendColor.red   = blendColor.red;
        mBlendColor.green = blendColor.green;
        mBlendColor.blue  = blendColor.blue;
    }
}

void StateManagerD3D9::setSampleAlphaToCoverageEnabled(bool sampleAlphaToCoverage)
{
    if (sampleAlphaToCoverage)
    {
        FIXME("Sample alpha to coverage is unimplemented.");
    }
}

void StateManagerD3D9::setDitherEnabled(bool ditherEnabled)
{
    if (mBlendState.dither != ditherEnabled)
    {
        mDevice->SetRenderState(D3DRS_DITHERENABLE, ditherEnabled ? TRUE : FALSE);
        mBlendState.dither = ditherEnabled;
    }
}

void StateManagerD3D9::setBlendColorMask(const gl::BlendState &blendState,
                                         const gl::Framebuffer *framebuffer)
{
    if (mBlendState.colorMaskRed != blendState.colorMaskRed ||
        mBlendState.colorMaskGreen != blendState.colorMaskGreen ||
        mBlendState.colorMaskBlue != blendState.colorMaskBlue ||
        mBlendState.colorMaskAlpha != blendState.colorMaskAlpha)
    {
        const gl::FramebufferAttachment *attachment = framebuffer->getFirstColorbuffer();
        GLenum internalFormat                       = attachment ? attachment->getInternalFormat() : GL_NONE;

        // Set the color mask
        bool zeroColorMaskAllowed = getVendorId() != VENDOR_ID_AMD;
        // Apparently some ATI cards have a bug where a draw with a zero color
        // write mask can cause later draws to have incorrect results. Instead,
        // set a nonzero color write mask but modify the blend state so that no
        // drawing is done.
        // http://code.google.com/p/angleproject/issues/detail?id=169

        const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
        DWORD colorMask =
            gl_d3d9::ConvertColorMask(formatInfo.redBits > 0 && blendState.colorMaskRed,
                                      formatInfo.greenBits > 0 && blendState.colorMaskGreen,
                                      formatInfo.blueBits > 0 && blendState.colorMaskBlue,
                                      formatInfo.alphaBits > 0 && blendState.colorMaskAlpha);
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

        mBlendState.colorMaskRed   = blendState.colorMaskRed;
        mBlendState.colorMaskBlue  = blendState.colorMaskBlue;
        mBlendState.colorMaskGreen = blendState.colorMaskGreen;
        mBlendState.colorMaskAlpha = blendState.colorMaskAlpha;
    }
}

void StateManagerD3D9::setSampleMask(unsigned int sampleMask)
{
    // Set the multisample mask
    if (mSampleMask != sampleMask)
    {
        mDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
        mDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, static_cast<DWORD>(sampleMask));
        mSampleMask = sampleMask;
    }
}

gl::Error StateManagerD3D9::setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                                 int stencilRef,
                                                 int stencilBackRef,
                                                 bool frontFaceCCW,
                                                 const gl::State::DirtyBits &dirtyBits)
{
    ASSERT(depthStencilState.stencilWritemask == depthStencilState.stencilBackWritemask);
    ASSERT(stencilRef == stencilBackRef);
    ASSERT(depthStencilState.stencilMask == depthStencilState.stencilBackMask);

    if (dirtyBits.test(gl::State::DIRTY_BIT_DEPTH_MASK) || mForceSetDepthStencilState)
    {
        setDepthMask(depthStencilState.depthMask);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED) ||
        dirtyBits.test(gl::State::DIRTY_BIT_DEPTH_FUNC) || mForceSetDepthStencilState)
    {
        setDepthTestAndFunc(depthStencilState.depthTest, depthStencilState.depthFunc);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED) || mForceSetDepthStencilState)
    {
        setStencilTestEnabled(depthStencilState.stencilTest);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT) || mForceSetDepthStencilState)
    {
        setStencilFuncsFront(depthStencilState.stencilFunc, depthStencilState.stencilMask,
                             stencilRef, frontFaceCCW);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK) || mForceSetDepthStencilState)
    {
        setStencilFuncsBack(depthStencilState.stencilBackFunc, depthStencilState.stencilBackMask,
                            stencilBackRef, frontFaceCCW);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT) || mForceSetDepthStencilState)
    {
        setStencilWriteMaskFront(depthStencilState.stencilWritemask, frontFaceCCW);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK) || mForceSetDepthStencilState)
    {
        setStencilWriteMaskBack(depthStencilState.stencilBackWritemask, frontFaceCCW);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_OPS_FRONT) || mForceSetDepthStencilState)
    {
        setStencilOpsFront(depthStencilState.stencilFail, depthStencilState.stencilPassDepthFail,
                           depthStencilState.stencilPassDepthPass, frontFaceCCW);
    }

    if (dirtyBits.test(gl::State::DIRTY_BIT_STENCIL_OPS_BACK) || mForceSetDepthStencilState)
    {
        setStencilOpsBack(depthStencilState.stencilBackFail,
                          depthStencilState.stencilBackPassDepthFail,
                          depthStencilState.stencilBackPassDepthPass, frontFaceCCW);
    }

    mForceSetDepthStencilState = false;

    return gl::Error(GL_NO_ERROR);
}

void StateManagerD3D9::setDepthMask(bool depthMask)
{
    if (depthMask != mDepthStencilState.depthMask)
    {
        mDevice->SetRenderState(D3DRS_ZWRITEENABLE, depthMask ? TRUE : FALSE);
        mDepthStencilState.depthMask = depthMask;
    }
}

void StateManagerD3D9::setDepthTestAndFunc(bool depthTest, GLenum depthFunc)
{
    if (mDepthStencilState.depthTest != depthTest || mDepthStencilState.depthFunc != depthFunc)
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

        mDepthStencilState.depthTest = depthTest;
        mDepthStencilState.depthFunc = depthFunc;
    }
}

void StateManagerD3D9::setStencilTestEnabled(bool stencilTest)
{
    if (mDepthStencilState.stencilTest != stencilTest)
    {
        if (stencilTest && mCurStencilSize > 0)
        {
            mDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
            mDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);
        }
        else
        {
            mDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        }

        mDepthStencilState.stencilTest = stencilTest;
    }
}

void StateManagerD3D9::setStencilFuncsFront(GLenum stencilFunc,
                                            GLuint stencilMask,
                                            int stencilRef,
                                            bool frontFaceCCW)
{
    if (mDepthStencilState.stencilFunc != stencilFunc ||
        mDepthStencilState.stencilMask != stencilMask || mStencilRef != stencilRef)
    {
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                                gl_d3d9::ConvertComparison(stencilFunc));
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                                (stencilRef < (int)mMaxStencil) ? stencilRef : mMaxStencil);
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK,
                                stencilMask);

        mDepthStencilState.stencilFunc = stencilFunc;
        mDepthStencilState.stencilMask = stencilMask;
        mStencilRef                    = stencilRef;
    }
}

void StateManagerD3D9::setStencilFuncsBack(GLenum stencilBackFunc,
                                           GLuint stencilBackMask,
                                           int stencilBackRef,
                                           bool frontFaceCCW)
{
    if (mDepthStencilState.stencilBackFunc != stencilBackFunc ||
        mDepthStencilState.stencilMask != stencilBackMask || mStencilBackRef != stencilBackRef)
    {
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC,
                                gl_d3d9::ConvertComparison(stencilBackFunc));
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILREF : D3DRS_CCW_STENCILREF,
                                (stencilBackRef < (int)mMaxStencil) ? stencilBackRef : mMaxStencil);
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILMASK : D3DRS_CCW_STENCILMASK,
                                stencilBackMask);

        mDepthStencilState.stencilBackFunc = stencilBackFunc;
        mDepthStencilState.stencilBackMask = stencilBackMask;
        mStencilBackRef                    = stencilBackRef;
    }
}

void StateManagerD3D9::setStencilWriteMaskFront(GLuint stencilWritemask, bool frontFaceCCW)
{
    if (mDepthStencilState.stencilWritemask != stencilWritemask)
    {
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK,
                                stencilWritemask);

        mDepthStencilState.stencilWritemask = stencilWritemask;
    }
}

void StateManagerD3D9::setStencilWriteMaskBack(GLuint stencilBackWritemask, bool frontFaceCCW)
{
    if (mDepthStencilState.stencilBackWritemask != stencilBackWritemask)
    {
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILWRITEMASK : D3DRS_CCW_STENCILWRITEMASK,
                                stencilBackWritemask);

        mDepthStencilState.stencilBackWritemask = stencilBackWritemask;
    }
}

void StateManagerD3D9::setStencilOpsFront(GLenum stencilFail,
                                          GLenum stencilPassDepthFail,
                                          GLenum stencilPassDepthPass,
                                          bool frontFaceCCW)
{
    if (mDepthStencilState.stencilFail != stencilFail ||
        mDepthStencilState.stencilPassDepthFail != stencilPassDepthFail ||
        mDepthStencilState.stencilPassDepthPass != stencilPassDepthPass)
    {
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                                gl_d3d9::ConvertStencilOp(stencilFail));
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                                gl_d3d9::ConvertStencilOp(stencilPassDepthFail));
        mDevice->SetRenderState(frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                                gl_d3d9::ConvertStencilOp(stencilPassDepthPass));

        mDepthStencilState.stencilFail          = stencilFail;
        mDepthStencilState.stencilPassDepthFail = stencilPassDepthFail;
        mDepthStencilState.stencilPassDepthPass = stencilPassDepthPass;
    }
}

void StateManagerD3D9::setStencilOpsBack(GLenum stencilBackFail,
                                         GLenum stencilBackPassDepthFail,
                                         GLenum stencilBackPassDepthPass,
                                         bool frontFaceCCW)
{
    if (mDepthStencilState.stencilBackFail != stencilBackFail ||
        mDepthStencilState.stencilBackPassDepthFail != stencilBackPassDepthFail ||
        mDepthStencilState.stencilBackPassDepthPass != stencilBackPassDepthPass)
    {
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILFAIL : D3DRS_CCW_STENCILFAIL,
                                gl_d3d9::ConvertStencilOp(stencilBackFail));
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILZFAIL : D3DRS_CCW_STENCILZFAIL,
                                gl_d3d9::ConvertStencilOp(stencilBackPassDepthFail));
        mDevice->SetRenderState(!frontFaceCCW ? D3DRS_STENCILPASS : D3DRS_CCW_STENCILPASS,
                                gl_d3d9::ConvertStencilOp(stencilBackPassDepthPass));

        mDepthStencilState.stencilBackFail          = stencilBackFail;
        mDepthStencilState.stencilBackPassDepthFail = stencilBackPassDepthFail;
        mDepthStencilState.stencilBackPassDepthPass = stencilBackPassDepthPass;
    }
}
}