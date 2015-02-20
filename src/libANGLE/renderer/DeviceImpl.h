//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceImpl.h: Implementation methods of egl::Device

#ifndef LIBANGLE_RENDERER_DEVICEIMPL_H_
#define LIBANGLE_RENDERER_DEVICEIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace rx
{
class DeviceImpl
{
  public:
    DeviceImpl();
    virtual ~DeviceImpl();

    virtual egl::Error queryAttribute(EGLint attribute, EGLAttrib *value) = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(DeviceImpl);
};

}

#endif // LIBANGLE_RENDERER_DEVICEIMPL_H_
