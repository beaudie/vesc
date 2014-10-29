//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererImpl.cpp: Defines the methods for a back-end renderer implementation.
//

#include "libGLESv2/renderer/RendererImpl.h"

namespace rx
{

RendererImpl::RendererImpl()
    : mCapsInitialized(false),
      mWorkaroundsInitialized(false)
{}

RendererImpl::~RendererImpl()
{}

const gl::Caps &RendererImpl::getRendererCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mCaps;
}

const gl::TextureCapsMap &RendererImpl::getRendererTextureCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mTextureCaps;
}

const gl::Extensions &RendererImpl::getRendererExtensions() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mExtensions;
}

const Workarounds &RendererImpl::getWorkarounds() const
{
    if (!mWorkaroundsInitialized)
    {
        mWorkarounds = generateWorkarounds();
        mWorkaroundsInitialized = true;
    }

    return mWorkarounds;
}

}
