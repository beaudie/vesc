//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferD3d.h: Defines the RenderbufferD3D class which implements RenderbufferImpl.

#ifndef LIBGLESV2_RENDERER_RENDERBUFFERD3D_H_
#define LIBGLESV2_RENDERER_RENDERBUFFERD3D_H_

#include "angle_gl.h"

#include "common/angleutils.h"
#include "libGLESv2/renderer/RenderbufferImpl.h"

namespace rx
{

class RenderbufferD3D : public RenderbufferImpl
{
public:
    RenderbufferD3D(RenderTarget *renderTarget, GLenum requestedFormat);
    virtual ~RenderbufferD3D();

    static RenderbufferD3D *makeRenderbufferD3D(RenderbufferImpl *texture);

    virtual rx::RenderTarget *getRenderTarget();
    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height);

private:
    DISALLOW_COPY_AND_ASSIGN(RenderbufferD3D);

    RenderTarget *mRenderTarget;
};
}

#endif // LIBGLESV2_RENDERER_RENDERBUFFERD3D_H_