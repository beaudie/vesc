//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CrosPixmap.cpp: Implementation of OSPixmap for Chrome OS

#include "cros/CrosPixmap.h"

CrosPixmap::CrosPixmap()
{
}

CrosPixmap::~CrosPixmap()
{
}

bool CrosPixmap::initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth)
{
    return false;
}

EGLNativePixmapType CrosPixmap::getNativePixmap() const
{
    return 0;
}

OSPixmap *CreateOSPixmap()
{
    return new CrosPixmap();
}
