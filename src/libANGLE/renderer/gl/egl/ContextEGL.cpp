//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/egl/ContextEGL.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/egl/DisplayEGL.h"

namespace rx
{
ContextEGL::ContextEGL(const gl::State &state,
                       gl::ErrorSet *errorSet,
                       const std::shared_ptr<RendererEGL> &renderer)
    : ContextGL(state, errorSet, renderer), mRendererEGL(renderer)
{}

ContextEGL::ContextEGL(const gl::State &state, gl::ErrorSet *errorSet, EGLContext shareContext)
    : ContextGL(state, errorSet, nullptr), mShareContext(shareContext)
{}

ContextEGL::~ContextEGL() {}

angle::Result ContextEGL::initialize()
{
    ANGLE_TRY(ContextGL::initialize());

    if (!mRendererEGL)
    {
        // FIXME: pass DisplayEGL through ContextEGL's constructor as well.
        gl::ContextID ctx_id    = getState().getContextID();
        egl::Display *dpy       = reinterpret_cast<gl::Context *>(ctx_id)->getDisplay();
        DisplayEGL *display_egl = reinterpret_cast<DisplayEGL *>(dpy->getImplementation());

        egl::Error error = display_egl->createRenderer(mShareContext, &mRendererEGL);
        if (error.isError())
        {
            ERR() << "Failed to create a shared renderer: " << error.getMessage();
            return angle::Result::Incomplete;
        }

        mRenderer = mRendererEGL;
    }

    return angle::Result::Continue;
}

EGLContext ContextEGL::getContext() const
{
    return mRendererEGL->getContext();
}
}  // namespace rx
