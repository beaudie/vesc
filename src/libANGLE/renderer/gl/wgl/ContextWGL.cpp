//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/wgl/ContextWGL.h"

namespace rx
{
ContextWGL::ContextWGL(const gl::ContextState &state, std::shared_ptr<RendererWGL> renderer)
    : ContextGL(state, renderer)
{}

ContextWGL::~ContextWGL() {}

HGLRC ContextWGL::getContext() const
{
    auto renderer = std::static_pointer_cast<RendererWGL>(mRenderer);
    return renderer->getContext();
}
}  // namespace rx
