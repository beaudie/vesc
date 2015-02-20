//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device.h: Implements the egl::Device class, representing the abstract
// device. Implements EGLDevice.

#ifndef LIBANGLE_DEVICE_H_
#define LIBANGLE_DEVICE_H_

#include "libANGLE/Error.h"
#include "common/angleutils.h"

namespace rx
{
class DeviceImpl;
}

namespace egl
{
class Device final
{
  public:
    Device(rx::DeviceImpl *impl);
    ~Device();

    Error queryAttribute(EGLint attribute, EGLAttrib *value);

  private:
    DISALLOW_COPY_AND_ASSIGN(Device);

    rx::DeviceImpl *mImplementation;
};

}

#endif   // LIBANGLE_DEVICE_H_
