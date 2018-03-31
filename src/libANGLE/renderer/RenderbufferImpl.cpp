//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.cpp: Implements the abstract class gl::RenderbufferImpl

#include "libANGLE/renderer/RenderbufferImpl.h"

#include "libANGLE/Renderbuffer.h"

namespace rx
{
RenderbufferImpl::RenderbufferImpl(const gl::RenderbufferState &state)
    : FramebufferAttachmentObjectImpl(state), mState(state)
{
}

RenderbufferImpl::~RenderbufferImpl()
{
}

gl::Error RenderbufferImpl::onDestroy(const gl::Context *context)
{
    return gl::NoError();
}
}  // namespace rx
