//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DRMPixmap.h: Definition of the implementation of OSPixmap for DRM

#ifndef UTIL_DRM_PIXMAP_H_
#define UTIL_DRM_PIXMAP_H_

#include "OSPixmap.h"

class DRMPixmap : public OSPixmap
{
  public:
    DRMPixmap();
    ~DRMPixmap() override;

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

#endif // UTIL_DRM_PIXMAP_H_
