//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceD3D.h: D3D implementation of egl::Device

#ifndef LIBANGLE_RENDERER_D3D_DEVICED3D_H_
#define LIBANGLE_RENDERER_D3D_DEVICED3D_H_

#include "libANGLE/Device.h"
#include "libANGLE/renderer/d3d/rendererd3d.h"

namespace rx
{
class DeviceD3D : public egl::Device
{
  public:
    DeviceD3D(egl::Display *display, RendererD3D* renderer);

    egl::Error getDevice(EGLAttrib *value) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(DeviceD3D);

    RendererD3D *mRenderer;
};

}

#endif // LIBANGLE_RENDERER_D3D_DEVICED3D_H_
