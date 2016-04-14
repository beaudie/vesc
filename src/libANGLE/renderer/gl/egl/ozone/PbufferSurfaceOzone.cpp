//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceOzone.cpp: Ozone implementation of egl::Surface for PBuffers

#include "libANGLE/renderer/gl/egl/ozone/PbufferSurfaceOzone.h"

namespace rx
{

PbufferSurfaceOzone::PbufferSurfaceOzone(RendererGL *renderer, DisplayOzone::Buffer *buffer)
    : WindowSurfaceOzone(renderer, buffer)
{
}

PbufferSurfaceOzone::~PbufferSurfaceOzone()
{
}

egl::Error PbufferSurfaceOzone::swap()
{
    return egl::Error(EGL_SUCCESS);
}
}  // namespace rx
