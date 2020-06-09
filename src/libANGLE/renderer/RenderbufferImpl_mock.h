//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl_mock.h: Defines a mock of the RenderbufferImpl class.

#ifndef LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/Image.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/renderer/RenderbufferImpl.h"

namespace rx
{

class MockRenderbufferImpl : public RenderbufferImpl
{
  public:
    MockRenderbufferImpl() : RenderbufferImpl(mMockState) {}
    virtual ~MockRenderbufferImpl() { destructor(); }
    MOCK_METHOD(angle::Result,
                setStorage,
                (const gl::Context *, GLenum, size_t, size_t),
                (override));
    MOCK_METHOD(angle::Result,
                setStorageMultisample,
                (const gl::Context *, size_t, GLenum, size_t, size_t),
                (override));
    MOCK_METHOD(angle::Result,
                setStorageEGLImageTarget,
                (const gl::Context *, egl::Image *),
                (override));

    MOCK_METHOD(angle::Result,
                getAttachmentRenderTarget,
                (const gl::Context *,
                 GLenum,
                 const gl::ImageIndex &,
                 GLsizei,
                 FramebufferAttachmentRenderTarget **),
                (override));

    MOCK_METHOD(void, destructor, ());

  protected:
    gl::RenderbufferState mMockState;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_
