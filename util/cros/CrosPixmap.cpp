//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CrosPixmap.cpp: Implementation of OSPixmap for Chrome OS

#include "cros/CrosPixmap.h"

CrosPixmap::CrosPixmap()
  : mPixmap(0),
    mDisplay(nullptr)
{
}

CrosPixmap::~CrosPixmap()
{
    if (mPixmap)
    {
        //XXX XFreePixmap(mDisplay, mPixmap);
    }
}

bool CrosPixmap::initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth)
{
    /*XXX
    mDisplay = display;

    int screen = DefaultScreen(mDisplay);
    Window root = RootWindow(mDisplay, screen);
    */

    mPixmap = 0;//XXX XCreatePixmap(mDisplay, root, width, height, depth);

    return mPixmap != 0;
}

EGLNativePixmapType CrosPixmap::getNativePixmap() const
{
    return mPixmap;
}

OSPixmap *CreateOSPixmap()
{
    return new CrosPixmap();
}
