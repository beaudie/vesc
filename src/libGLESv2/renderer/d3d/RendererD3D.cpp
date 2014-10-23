//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererD3D.cpp: Implements EGL dependencies for creating and destroying
// Renderer3D instances.

#include "libGLESv2/renderer/d3d/RendererD3D.h"

namespace rx
{

RendererD3D::RendererD3D(egl::Display *display)
    : Renderer(display)
{
}

RendererD3D::~RendererD3D()
{
}

}  // namespace rx
