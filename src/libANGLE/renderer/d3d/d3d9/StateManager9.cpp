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

StateManager9::StateManager9() : mDevice(nullptr), mAdapterIdentifier(nullptr)
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

void StateManager9::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
}

gl::Error StateManager9::setBlendState(const gl::Framebuffer *framebuffer,
                                       const gl::BlendState &blendState,
                                       const gl::ColorF &blendColor,
                                       unsigned int sampleMask)
{
    bool blendStateChanged =
        mBlendStateIsDirty || memcmp(&blendState, &mCurBlendState, sizeof(gl::BlendState)) != 0;
    bool blendColorChanged =
        mBlendStateIsDirty || memcmp(&blendColor, &mCurBlendColor, sizeof(gl::ColorF)) != 0;
    bool sampleMaskChanged = mBlendStateIsDirty || sampleMask != mCurSampleMask;

    if (blendStateChanged || blendColorChanged)
    {
        setBlendEnabled(blendState.blend);
        setBlendColor(blendState, blendColor);
        setBlendFuncsEquations(blendState);
        setSampleAlphaToCoverage(blendState.sampleAlphaToCoverage);
        setColorMask(framebuffer, blendState.colorMaskRed, blendState.colorMaskBlue,
                     blendState.colorMaskGreen, blendState.colorMaskAlpha);
        setDither(blendState.dither);

        mCurBlendState = blendState;
        mCurBlendColor = blendColor;
    }

    if (sampleMaskChanged)
    {
        setSampleMask(sampleMask);
    }

    mBlendStateIsDirty = false;

    return gl::Error(GL_NO_ERROR);
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
    }
}

void StateManager9::setBlendEnabled(bool enabled)
{
    mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enabled ? TRUE : FALSE);
}

void StateManager9::setDither(bool dither)
{
    mDevice->SetRenderState(D3DRS_DITHERENABLE, dither ? TRUE : FALSE);
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
}

void StateManager9::setSampleMask(unsigned int sampleMask)
{
    // Set the multisample mask
    mDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    mDevice->SetRenderState(D3DRS_MULTISAMPLEMASK, static_cast<DWORD>(sampleMask));

    mCurSampleMask = sampleMask;
}
}