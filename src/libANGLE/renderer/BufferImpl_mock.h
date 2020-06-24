//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferImpl_mock.h: Defines a mock of the BufferImpl class.

#ifndef LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/renderer/BufferImpl.h"

namespace rx
{
class MockBufferImpl : public BufferImpl
{
  public:
    MockBufferImpl() : BufferImpl(mMockState) {}
    ~MockBufferImpl() { destructor(); }

    MOCK_METHOD(angle::Result,
                setData,
                (const gl::Context *, gl::BufferBinding, const void *, size_t, gl::BufferUsage),
                (override));
    MOCK_METHOD(angle::Result,
                setSubData,
                (const gl::Context *, gl::BufferBinding, const void *, size_t, size_t),
                (override));
    MOCK_METHOD(angle::Result,
                copySubData,
                (const gl::Context *contextImpl, BufferImpl *, GLintptr, GLintptr, GLsizeiptr),
                (override));
    MOCK_METHOD(angle::Result, map, (const gl::Context *contextImpl, GLenum, void **), (override));
    MOCK_METHOD(angle::Result,
                mapRange,
                (const gl::Context *contextImpl, size_t, size_t, GLbitfield, void **),
                (override));
    MOCK_METHOD(angle::Result,
                unmap,
                (const gl::Context *contextImpl, GLboolean *result),
                (override));

    MOCK_METHOD(angle::Result,
                getIndexRange,
                (const gl::Context *, gl::DrawElementsType, size_t, size_t, bool, gl::IndexRange *),
                (override));

    MOCK_METHOD(void, destructor, ());

  protected:
    gl::BufferState mMockState;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_BUFFERIMPLMOCK_H_
