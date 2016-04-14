//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OzonePixmap.h: Definition of the implementation of OSPixmap for Ozone

#ifndef UTIL_OZONE_PIXMAP_H_
#define UTIL_OZONE_PIXMAP_H_

#include "OSPixmap.h"

class OzonePixmap : public OSPixmap
{
  public:
    OzonePixmap();
    ~OzonePixmap() override;

    bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) override;

    EGLNativePixmapType getNativePixmap() const override;

  private:
};

#endif  // UTIL_OZONE_PIXMAP_H_
