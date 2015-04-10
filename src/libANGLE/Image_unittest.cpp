//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/Image.h"
#include "libANGLE/renderer/ImageImpl_mock.h"

namespace
{

class ImageTest : public testing::Test
{
  protected:
    virtual void SetUp()
    {
        mImpl = new rx::MockImageImpl;
        EXPECT_CALL(*mImpl, destructor());
        mImage = new egl::Image(mImpl);
    }

    virtual void TearDown()
    {
        SafeDelete(mImage);
    }

    rx::MockImageImpl *mImpl;
    egl::Image *mImage;
};

TEST_F(ImageTest, DestructionDeletesImpl)
{
    rx::MockImageImpl *impl = new rx::MockImageImpl;
    EXPECT_CALL(*impl, destructor()).Times(1).RetiresOnSaturation();

    egl::Image *image = new egl::Image(impl);
    SafeDelete(image);

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    testing::Mock::VerifyAndClear(impl);
}

}
