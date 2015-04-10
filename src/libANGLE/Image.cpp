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

Image::Image(rx::ImageImpl *impl, EGLenum target, gl::Texture *buffer, const AttributeMap &attribs)
    : mImplementation(impl),
      mSiblings()
{
    ASSERT(mImplementation);

    mSiblings.push_back(BindingPointer<gl::Texture>(buffer));
}

Image::~Image()
{
    SafeDelete(mImplementation);

    // All siblings should hold a ref to the egl image and it should not be deleted until there are
    // no siblings left.
    ASSERT(mSiblings.empty());
}

void Image::reference(gl::Texture *sibling)
{
    // notify impl

    mSiblings.push_back(BindingPointer<gl::Texture>(sibling));
}

void Image::orphan(gl::Texture *sibling)
{
    for (auto iter = mSiblings.begin(); iter != mSiblings.end(); iter++)
    {
        if (iter->get() == sibling)
        {
            // notify impl

            mSiblings.erase(iter);
        }
    }
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
