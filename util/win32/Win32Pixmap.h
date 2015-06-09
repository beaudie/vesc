//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32Window.h: Definition of the implementation of OSPixmap for Windows

#ifndef UTIL_WIN32_PIXMAP_H
#define UTIL_WIN32_PIXMAP_H

#include <windows.h>

#include "OSPixmap.h"

class Win32Pixmap : public OSPixmap
{
  public:
    Win32Pixmap();
    ~Win32Pixmap() override;

    bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) override;

    EGLNativePixmapType getNativePixmap() const override;

  private:
    HBITMAP mBitmap;
};

#endif // UTIL_WIN32_PIXMAP_H
