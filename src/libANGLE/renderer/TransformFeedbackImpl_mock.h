//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class MockTransformFeedbackImpl : public TransformFeedbackImpl
{
  public:
    ~MockTransformFeedbackImpl() override { destructor(); }

    MOCK_METHOD1(begin, void(GLenum primitiveMode));
    MOCK_METHOD0(end, void());
    MOCK_METHOD0(pause, void());
    MOCK_METHOD0(resume, void());

    MOCK_METHOD1(bindGenericBuffer, void(const BindingPointer<gl::Buffer> &));
    MOCK_METHOD2(bindIndexedBuffer, void(size_t, const OffsetBindingPointer<gl::Buffer> &));

    MOCK_METHOD0(destructor, void());
};

}
