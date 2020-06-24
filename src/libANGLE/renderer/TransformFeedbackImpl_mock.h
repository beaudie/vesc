//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackImpl_mock.h: Defines a mock of the TransformFeedbackImpl class.

#ifndef LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_
#define LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class MockTransformFeedbackImpl : public TransformFeedbackImpl
{
  public:
    MockTransformFeedbackImpl(const gl::TransformFeedbackState &state)
        : TransformFeedbackImpl(state)
    {}
    ~MockTransformFeedbackImpl() { destructor(); }

    MOCK_METHOD(angle::Result, begin, (const gl::Context *, gl::PrimitiveMode), (override));
    MOCK_METHOD(angle::Result, end, (const gl::Context *), (override));
    MOCK_METHOD(angle::Result, pause, (const gl::Context *), (override));
    MOCK_METHOD(angle::Result, resume, (const gl::Context *), (override));

    MOCK_METHOD(angle::Result,
                bindIndexedBuffer,
                (const gl::Context *, size_t, const gl::OffsetBindingPointer<gl::Buffer> &),
                (override));

    MOCK_METHOD(void, destructor, ());
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_
