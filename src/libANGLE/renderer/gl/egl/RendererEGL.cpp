//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/egl/RendererEGL.h"

#include "libANGLE/renderer/gl/egl/DisplayEGL.h"

namespace rx
{

RendererEGL::RendererEGL(const FunctionsGL *functionsGL,
                         const egl::AttributeMap &attribMap,
                         DisplayEGL *display,
                         EGLContext context)
    : RendererGL(functionsGL, attribMap), mDisplay(display), mContext(context)
{
}

RendererEGL::~RendererEGL()
{
    mDisplay->destroyNativeContext(mContext);
    mContext = nullptr;
}

EGLContext RendererEGL::getContext() const
{
    return mContext;
}

}  // namespace rx
