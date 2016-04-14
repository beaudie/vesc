//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceOzone.h: Chrome OS implementation of egl::Surface for PBuffers

#ifndef LIBANGLE_RENDERER_GL_OZONE_PBUFFERSURFACEOZONE_H_
#define LIBANGLE_RENDERER_GL_OZONE_PBUFFERSURFACEOZONE_H_

#include "libANGLE/renderer/gl/egl/ozone/WindowSurfaceOzone.h"

namespace rx
{

class DisplayOzone;
class FunctionsEGL;

class PbufferSurfaceOzone : public WindowSurfaceOzone
{
  public:
    PbufferSurfaceOzone(RendererGL *renderer, DisplayOzone::Buffer *buffer);
    ~PbufferSurfaceOzone() override;

    egl::Error swap() override;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_OZONE_PBUFFERSURFACEZONE_H_
