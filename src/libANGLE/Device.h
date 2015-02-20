//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device.h: Implements the egl::Device class, representing the abstract
// device. Implements EGLDevice.

#ifndef LIBANGLE_DEVICE_H_
#define LIBANGLE_DEVICE_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/Display.h"

namespace rx
{
    class DeviceImpl;
}

namespace egl
{
class Device final
{
  public:
    Device(Display* display, rx::DeviceImpl *impl);
    virtual ~Device();

    Error getDevice(EGLAttrib *value);
    Display* getDisplay() { return mDisplay; };

    rx::DeviceImpl *getImplementation() { return mImplementation; }

  private:
    DISALLOW_COPY_AND_ASSIGN(Device);

    Display* mDisplay;
    rx::DeviceImpl *mImplementation;
};

}

#endif   // LIBANGLE_DEVICE_H_
