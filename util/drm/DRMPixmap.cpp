//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DRMPixmap.cpp: Implementation of OSPixmap for DRM

#include "drm/DRMPixmap.h"

DRMPixmap::DRMPixmap()
  : mPixmap(0),
    mDisplay(nullptr)
{
}

DRMPixmap::~DRMPixmap()
{
    if (mPixmap)
    {
        //XXX XFreePixmap(mDisplay, mPixmap);
    }
}

bool DRMPixmap::initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth)
{
    /*XXX
    mDisplay = display;

    int screen = DefaultScreen(mDisplay);
    Window root = RootWindow(mDisplay, screen);
    */

    mPixmap = 0;//XXX XCreatePixmap(mDisplay, root, width, height, depth);

    return mPixmap != 0;
}

EGLNativePixmapType DRMPixmap::getNativePixmap() const
{
    return mPixmap;
}

OSPixmap *CreateOSPixmap()
{
    return new DRMPixmap();
}
