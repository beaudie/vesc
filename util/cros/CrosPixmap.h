//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CrosPixmap.h: Definition of the implementation of OSPixmap for Chrome OS

#ifndef UTIL_CROS_PIXMAP_H_
#define UTIL_CROS_PIXMAP_H_

#include "OSPixmap.h"

class CrosPixmap : public OSPixmap
{
  public:
    CrosPixmap();
    ~CrosPixmap() override;

    bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) override;

    EGLNativePixmapType getNativePixmap() const override;

  private:
#ifdef XXX
    Pixmap mPixmap;
    Display *mDisplay;
#else
    unsigned mPixmap;
    void *mDisplay;
#endif
};

#endif // UTIL_CROS_PIXMAP_H_
