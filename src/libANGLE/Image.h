//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.h: Defines the egl::Image class representing the EGLimage object.

#ifndef LIBANGLE_IMAGE_H_
#define LIBANGLE_IMAGE_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace rx
{
class ImageImpl;
}

namespace egl
{

class Image final : angle::NonCopyable
{
  public:
    explicit Image(rx::ImageImpl *impl);
    ~Image();

    rx::ImageImpl *getImplementation();
    const rx::ImageImpl *getImplementation() const;

  private:
    rx::ImageImpl *mImplementation;
};

}

#endif // LIBANGLE_IMAGE_H_
