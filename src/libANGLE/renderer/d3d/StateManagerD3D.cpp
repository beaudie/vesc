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

namespace
{

void SetGivenBitsDirty(gl::State::DirtyBits &dirtyBits,
                       const gl::State::DirtyBitType *givenBits,
                       size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dirtyBits.set(givenBits[i]);
    }
}

const gl::State::DirtyBitType kBlendStateDirtyBits[] = {
    gl::State::DIRTY_BIT_BLEND_EQUATIONS, gl::State::DIRTY_BIT_BLEND_FUNCS,
    gl::State::DIRTY_BIT_BLEND_ENABLED,   gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED,
    gl::State::DIRTY_BIT_DITHER_ENABLED,  gl::State::DIRTY_BIT_COLOR_MASK,
    gl::State::DIRTY_BIT_BLEND_COLOR};

}  // anonymous namespace

StateManagerD3D::StateManagerD3D()
    : mCurBlendColor(0, 0, 0, 0), mCurSampleMask(0), mExternalDirtyBits()
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
}

StateManagerD3D::~StateManagerD3D()
{
}

const gl::State::DirtyBits StateManagerD3D::mBlendDirtyBits = []()
{
    gl::State::DirtyBits blendDirtyBits;
    SetGivenBitsDirty(blendDirtyBits, kBlendStateDirtyBits, ArraySize(kBlendStateDirtyBits));
    return blendDirtyBits;
}();

bool StateManagerD3D::IsBlendStateDirty(const gl::State::DirtyBits &dirtyBits)
{
    return (dirtyBits & mBlendDirtyBits).any();
}

void StateManagerD3D::forceSetBlendState()
{
    mForceSetBlendState = true;
}

void StateManagerD3D::syncExternalDirtyBits(const gl::State::DirtyBits &dirtyBits)
{
    mExternalDirtyBits = dirtyBits;
}

gl::Error StateManagerD3D::syncState(const gl::Data &data, const gl::State::DirtyBits &dirtyBits)
{
    // Implement this when all states have been moved to state manager
    return gl::Error(GL_NO_ERROR);
}
}  // namespace rx
