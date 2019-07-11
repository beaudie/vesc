//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Sampler.cpp : Implements the Sampler class, which represents a GLES 3
// sampler object. Sampler objects store some state needed to sample textures.

#include "libANGLE/Sampler.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/SamplerImpl.h"

namespace gl
{

Sampler::Sampler(rx::GLImplFactory *factory, GLuint id)
    : RefCountObject(id), mState(), mSampler(factory->createSampler(mState)), mLabel()
{
    // Initially assume the implementation is dirty.
    mDirtyBits.set(DIRTY_BIT_IMPLEMENTATION);
}

Sampler::~Sampler()
{
    SafeDelete(mSampler);
}

void Sampler::onDestroy(const Context *context)
{
    if (mSampler)
    {
        mSampler->onDestroy(context);
    }
}

void Sampler::setLabel(const Context *context, const std::string &label)
{
    mLabel = label;
}

const std::string &Sampler::getLabel() const
{
    return mLabel;
}

void Sampler::setMinFilter(const Context *context, GLenum minFilter)
{
    mState.setMinFilter(minFilter);
    signalDirtyState(DIRTY_BIT_MIN_FILTER);
}

GLenum Sampler::getMinFilter() const
{
    return mState.getMinFilter();
}

void Sampler::setMagFilter(const Context *context, GLenum magFilter)
{
    mState.setMagFilter(magFilter);
    signalDirtyState(DIRTY_BIT_MAG_FILTER);
}

GLenum Sampler::getMagFilter() const
{
    return mState.getMagFilter();
}

void Sampler::setWrapS(const Context *context, GLenum wrapS)
{
    mState.setWrapS(wrapS);
    signalDirtyState(DIRTY_BIT_WRAP_S);
}

GLenum Sampler::getWrapS() const
{
    return mState.getWrapS();
}

void Sampler::setWrapT(const Context *context, GLenum wrapT)
{
    mState.setWrapT(wrapT);
    signalDirtyState(DIRTY_BIT_WRAP_T);
}

GLenum Sampler::getWrapT() const
{
    return mState.getWrapT();
}

void Sampler::setWrapR(const Context *context, GLenum wrapR)
{
    mState.setWrapR(wrapR);
    signalDirtyState(DIRTY_BIT_WRAP_R);
}

GLenum Sampler::getWrapR() const
{
    return mState.getWrapR();
}

void Sampler::setMaxAnisotropy(const Context *context, float maxAnisotropy)
{
    mState.setMaxAnisotropy(maxAnisotropy);
    signalDirtyState(DIRTY_BIT_MAX_ANISOTROPY);
}

float Sampler::getMaxAnisotropy() const
{
    return mState.getMaxAnisotropy();
}

void Sampler::setMinLod(const Context *context, GLfloat minLod)
{
    mState.setMinLod(minLod);
    signalDirtyState(DIRTY_BIT_MIN_LOD);
}

GLfloat Sampler::getMinLod() const
{
    return mState.getMinLod();
}

void Sampler::setMaxLod(const Context *context, GLfloat maxLod)
{
    mState.setMaxLod(maxLod);
    signalDirtyState(DIRTY_BIT_MAX_LOD);
}

GLfloat Sampler::getMaxLod() const
{
    return mState.getMaxLod();
}

void Sampler::setCompareMode(const Context *context, GLenum compareMode)
{
    mState.setCompareMode(compareMode);
    signalDirtyState(DIRTY_BIT_COMPARE_MODE);
}

GLenum Sampler::getCompareMode() const
{
    return mState.getCompareMode();
}

void Sampler::setCompareFunc(const Context *context, GLenum compareFunc)
{
    mState.setCompareFunc(compareFunc);
    signalDirtyState(DIRTY_BIT_COMPARE_FUNC);
}

GLenum Sampler::getCompareFunc() const
{
    return mState.getCompareFunc();
}

void Sampler::setSRGBDecode(const Context *context, GLenum sRGBDecode)
{
    mState.setSRGBDecode(sRGBDecode);
    signalDirtyState(DIRTY_BIT_SRGB_DECODE);
}

GLenum Sampler::getSRGBDecode() const
{
    return mState.getSRGBDecode();
}

void Sampler::setBorderColor(const Context *context, const ColorGeneric &color)
{
    mState.setBorderColor(color);
    signalDirtyState(DIRTY_BIT_BORDER_COLOR);
}

const ColorGeneric &Sampler::getBorderColor() const
{
    return mState.getBorderColor();
}

const SamplerState &Sampler::getSamplerState() const
{
    return mState;
}

rx::SamplerImpl *Sampler::getImplementation() const
{
    return mSampler;
}

angle::Result Sampler::syncState(const Context *context)
{
    ASSERT(hasAnyDirtyBit());
    return mSampler->syncState(context, mDirtyBits);
    mDirtyBits.reset();
}

void Sampler::signalDirtyState(size_t dirtyBit)
{
    mDirtyBits.set(dirtyBit);
    onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
}

}  // namespace gl
