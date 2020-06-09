//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageImpl_mock.h: Defines a mock of the ImageImpl class.

#ifndef LIBANGLE_RENDERER_IMAGEIMPLMOCK_H_
#define LIBANGLE_RENDERER_IMAGEIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{
class MockImageImpl : public ImageImpl
{
  public:
    MockImageImpl(const egl::ImageState &state) : ImageImpl(state) {}
    virtual ~MockImageImpl() { destructor(); }
    MOCK_METHOD(egl::Error, initialize, (const egl::Display *), (override));
    MOCK_METHOD(angle::Result, orphan, (const gl::Context *, egl::ImageSibling *), (override));
    MOCK_METHOD(void, destructor, (), (override));
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_IMAGEIMPLMOCK_H_
