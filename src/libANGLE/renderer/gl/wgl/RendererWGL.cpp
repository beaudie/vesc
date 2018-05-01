//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/wgl/RendererWGL.h"

namespace rx
{

RendererWGL::RendererWGL(const FunctionsGL *functionsGL,
                         const egl::AttributeMap &attribMap,
                         const FunctionsWGL *functionsWGL,
                         HGLRC context)
    : RendererGL(functionsGL, attribMap), mFunctionsWGL(functionsWGL), mContext(context)
{
}

RendererWGL::~RendererWGL()
{
    mFunctionsWGL->deleteContext(mContext);
    mContext = nullptr;
}

HGLRC RendererWGL::getContext() const
{
    return mContext;
}

}  // namespace rx
