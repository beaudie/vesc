//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferImpl_mock.h:
//   Defines a mock of the FramebufferImpl class.
//

#ifndef LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

class MockFramebufferImpl : public rx::FramebufferImpl
{
  public:
    MockFramebufferImpl() : rx::FramebufferImpl(gl::FramebufferState(1)) {}
    virtual ~MockFramebufferImpl() { destructor(); }

    MOCK_METHOD(angle::Result, discard, (const gl::Context *, size_t, const GLenum *), (override));
    MOCK_METHOD(angle::Result,
                invalidate,
                (const gl::Context *, size_t, const GLenum *),
                (override));
    MOCK_METHOD(angle::Result,
                invalidateSub,
                (const gl::Context *, size_t, const GLenum *, const gl::Rectangle &),
                (override));

    MOCK_METHOD(angle::Result, clear, (const gl::Context *, GLbitfield), (override));
    MOCK_METHOD(angle::Result,
                clearBufferfv,
                (const gl::Context *, GLenum, GLint, const GLfloat *),
                (override));
    MOCK_METHOD(angle::Result,
                clearBufferuiv,
                (const gl::Context *, GLenum, GLint, const GLuint *),
                (override));
    MOCK_METHOD(angle::Result,
                clearBufferiv,
                (const gl::Context *, GLenum, GLint, const GLint *),
                (override));
    MOCK_METHOD(angle::Result,
                clearBufferfi,
                (const gl::Context *, GLenum, GLint, GLfloat, GLint),
                (override));

    MOCK_METHOD(angle::Result,
                readPixels,
                (const gl::Context *, const gl::Rectangle &, GLenum, GLenum, void *),
                (override));

    MOCK_METHOD(angle::Result,
                getSamplePosition,
                (const gl::Context *, size_t, GLfloat *),
                (const override));

    MOCK_METHOD(
        angle::Result,
        blit,
        (const gl::Context *, const gl::Rectangle &, const gl::Rectangle &, GLbitfield, GLenum),
        (override));

    MOCK_METHOD(bool, checkStatus, (const gl::Context *), (const override));

    MOCK_METHOD(angle::Result,
                syncState,
                (const gl::Context *, GLenum, const gl::Framebuffer::DirtyBits &),
                (override));

    MOCK_METHOD(void, destructor, ());
};

inline ::testing::NiceMock<MockFramebufferImpl> *MakeFramebufferMock()
{
    ::testing::NiceMock<MockFramebufferImpl> *framebufferImpl =
        new ::testing::NiceMock<MockFramebufferImpl>();
    // TODO(jmadill): add ON_CALLS for other returning methods
    ON_CALL(*framebufferImpl, checkStatus(testing::_)).WillByDefault(::testing::Return(true));

    // We must mock the destructor since NiceMock doesn't work for destructors.
    EXPECT_CALL(*framebufferImpl, destructor()).Times(1).RetiresOnSaturation();

    return framebufferImpl;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
