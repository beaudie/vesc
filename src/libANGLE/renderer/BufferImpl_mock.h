//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/BufferImpl.h"

namespace rx
{

class MockBufferImpl : public BufferImpl
{
  public:
    ~MockBufferImpl() override { destructor(); }

    MOCK_METHOD3(setData, gl::Error(const void*, size_t, GLenum));
    MOCK_METHOD3(setSubData, gl::Error(const void*, size_t, size_t));
    MOCK_METHOD4(copySubData, gl::Error(BufferImpl *, GLintptr, GLintptr, GLsizeiptr));
    MOCK_METHOD4(map, gl::Error(size_t, size_t, GLbitfield, GLvoid **));
    MOCK_METHOD0(unmap, gl::Error());

    MOCK_METHOD1(getData, gl::Error(const uint8_t **));

    MOCK_METHOD0(destructor, void());
};

}
