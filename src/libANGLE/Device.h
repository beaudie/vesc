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

namespace egl
{
class Device
{
  public:
    Device(Display* display);
    virtual ~Device();

    virtual Error getDevice(EGLAttrib *value) = 0;
    Display* getDisplay() { return mDisplay; };

  private:
    DISALLOW_COPY_AND_ASSIGN(Device);

    Display* mDisplay;
};

}

#endif   // LIBANGLE_DEVICE_H_
