//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OSPixmap.h: Definition of an abstract pixmap class

#ifndef SAMPLE_UTIL_PIXMAP_H
#define SAMPLE_UTIL_PIXMAP_H

#include "Event.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

class OSPixmap
{
  public:
    OSPixmap() {};
    virtual ~OSPixmap() {};

    virtual bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) = 0;

    virtual EGLNativePixmapType getNativePixmap() const = 0;
};

OSPixmap *CreateOSPixmap();

#endif // SAMPLE_UTIL_PIXMAP_H
