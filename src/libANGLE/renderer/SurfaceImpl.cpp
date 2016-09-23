//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceImpl.cpp: Implementation of Surface stub method class

#include "libANGLE/renderer/SurfaceImpl.h"

namespace rx
{

SurfaceImpl::SurfaceImpl(const egl::SurfaceState &state) : mState(state)
{
}

SurfaceImpl::~SurfaceImpl()
{
}

egl::Error SurfaceImpl::swapWithDamage(EGLint *rects, EGLint n_rects)
{
    UNREACHABLE();
    return egl::Error(EGL_BAD_SURFACE, "swapWithDamage implementation missing.");
}

egl::Error SurfaceImpl::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    UNREACHABLE();
    return egl::Error(EGL_BAD_SURFACE, "getSyncValues implementation missing.");
}

}  // namespace rx
