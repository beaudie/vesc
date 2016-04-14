//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OzonePixmap.cpp: Implementation of OSPixmap for Ozone

// TODO(fjhenigman) Implement or eliminate this class.

#include "ozone/OzonePixmap.h"

OzonePixmap::OzonePixmap()
{
}

OzonePixmap::~OzonePixmap()
{
}

bool OzonePixmap::initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth)
{
    return false;
}

EGLNativePixmapType OzonePixmap::getNativePixmap() const
{
    return 0;
}

OSPixmap *CreateOSPixmap()
{
    return new OzonePixmap();
}
