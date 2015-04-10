//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.cpp: Implements the egl::Image class representing the EGLimage object.

#include "libANGLE/Image.h"

#include "common/debug.h"
#include "libANGLE/renderer/ImageImpl.h"

namespace egl
{

Image::Image(rx::ImageImpl *impl)
    : mImplementation(impl)
{
    ASSERT(mImplementation);
}

Image::~Image()
{
    SafeDelete(mImplementation);
}

rx::ImageImpl *Image::getImplementation()
{
    return mImplementation;
}

const rx::ImageImpl *Image::getImplementation() const
{
    return mImplementation;
}

}
