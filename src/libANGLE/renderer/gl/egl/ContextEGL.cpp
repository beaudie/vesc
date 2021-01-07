//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/egl/ContextEGL.h"

#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

ContextEGL::ContextEGL(const gl::State &state,
                       gl::ErrorSet *errorSet,
                       const std::shared_ptr<RendererEGL> &renderer,
                       RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus,
                       bool externalContext)
    : ContextGL(state, errorSet, renderer, robustnessVideoMemoryPurgeStatus), mRendererEGL(renderer)
{
    if (externalContext)
    {
        mExtState        = std::make_unique<ExternalContextState>();
        const auto &caps = getNativeCaps();
        mExtState->vertexAttrib.resize(static_cast<size_t>(caps.maxVertexAttributes));
        mExtState->textureBindings.resize(static_cast<size_t>(caps.maxCombinedTextureImageUnits));
    }
}

ContextEGL::~ContextEGL() {}

bool ContextEGL::isExternalContext() const
{
    return !!mExtState;
}

void ContextEGL::syncWithExternalContext()
{
    ASSERT(mExtState);
    getStateManager()->syncFromNativeContext(getNativeExtensions(), mExtState.get());
}

void ContextEGL::restoreExternalContext()
{
    ASSERT(mExtState);
    getStateManager()->restoreNativeContext(getNativeExtensions(), mExtState.get());
}

EGLContext ContextEGL::getContext() const
{
    return mRendererEGL->getContext();
}

}  // namespace rx
