//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/gl/egl/ContextEGL.h"

#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

ContextEGL::ContextEGL(const gl::State &state,
                       gl::ErrorSet *errorSet,
                       const std::shared_ptr<RendererEGL> &renderer,
                       RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus,
                       bool externalContext,
                       bool saveAndRestoreState)
    : ContextGL(state, errorSet, renderer, robustnessVideoMemoryPurgeStatus),
      mRendererEGL(renderer),
      mIsExternal(externalContext),
      mSaveAndRestoreState(saveAndRestoreState)
{
    if (mIsExternal)
    {
        mExtState        = std::make_unique<ExternalContextState>();
        const auto &caps = getNativeCaps();
        mExtState->textureBindings.resize(static_cast<size_t>(caps.maxCombinedTextureImageUnits));
    }
}

ContextEGL::~ContextEGL() {}

bool ContextEGL::isExternal() const
{
    return mIsExternal;
}

angle::Result ContextEGL::onMakeCurrent(const gl::Context *context)
{
    if (!mIsCurrent)
    {
        mIsCurrent = true;
        if (mIsExternal)
        {
            getStateManager()->syncFromNativeContext(getNativeExtensions(), mExtState.get());

            // Use current FBO as the default framebuffer when the external context is current.
            gl::Framebuffer *framebuffer = mState.getDefaultFramebuffer();
            GetImplAs<FramebufferGL>(framebuffer)
                ->updateFramebufferID(mExtState->framebufferBinding);
        }
    }
    return ContextGL::onMakeCurrent(context);
}

angle::Result ContextEGL::onUnMakeCurrent(const gl::Context *context)
{
    ASSERT(mIsCurrent);
    mIsCurrent = false;

    if (mSaveAndRestoreState)
    {
        ASSERT(mIsExternal);
        ASSERT(mExtState);
        getStateManager()->restoreNativeContext(getNativeExtensions(), mExtState.get());
    }

    return ContextGL::onUnMakeCurrent(context);
}

EGLContext ContextEGL::getContext() const
{
    return mRendererEGL->getContext();
}

}  // namespace rx
