//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>

#include "libANGLE/Display.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLQueryStringIndexedTest : public ANGLETest
{
  public:
    void testSetUp() override
    {
        ASSERT_TRUE(IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                                 "EGL_ANGLE_query_string_indexed"));
    }

    void testTearDown() override {}
};

TEST_P(EGLQueryStringIndexedTest, InvalidDisplay)
{
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(EGL_NO_DISPLAY, EGL_WORKAROUND_NAME_ANGLE, 0));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);
}

TEST_P(EGLQueryStringIndexedTest, NegativeIndex)
{
    EXPECT_EQ(nullptr,
              eglQueryStringiANGLE(getEGLWindow()->getDisplay(), EGL_WORKAROUND_NAME_ANGLE, -1));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

TEST_P(EGLQueryStringIndexedTest, IndexOutOfBounds)
{
    EGLDisplay dpy        = getEGLWindow()->getDisplay();
    egl::Display *display = static_cast<egl::Display *>(dpy);
    EXPECT_EQ(nullptr,
              eglQueryStringiANGLE(dpy, EGL_WORKAROUND_NAME_ANGLE, display->getFeatures().size()));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

TEST_P(EGLQueryStringIndexedTest, InvalidName)
{
    EXPECT_EQ(nullptr, eglQueryStringiANGLE(getEGLWindow()->getDisplay(), 100, 0));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

TEST_P(EGLQueryStringIndexedTest, QueryAll)
{
    EGLDisplay dpy              = getEGLWindow()->getDisplay();
    egl::Display *display       = static_cast<egl::Display *>(dpy);
    angle::FeatureList features = display->getFeatures();
    for (size_t i = 0; i < features.size(); i++)
    {
        EXPECT_STREQ(features[i]->name, eglQueryStringiANGLE(dpy, EGL_WORKAROUND_NAME_ANGLE, i));
        EXPECT_STREQ(FeatureCategoryToString(features[i]->category),
                     eglQueryStringiANGLE(dpy, EGL_WORKAROUND_CATEGORY_ANGLE, i));
        EXPECT_STREQ(features[i]->description,
                     eglQueryStringiANGLE(dpy, EGL_WORKAROUND_DESCRIPTION_ANGLE, i));
        EXPECT_STREQ(features[i]->bug, eglQueryStringiANGLE(dpy, EGL_WORKAROUND_BUG_ANGLE, i));
        if (features[i]->enabled)
        {
            EXPECT_STREQ("true", eglQueryStringiANGLE(dpy, EGL_WORKAROUND_ENABLED_ANGLE, i));
        }
        else
        {
            EXPECT_STREQ("false", eglQueryStringiANGLE(dpy, EGL_WORKAROUND_ENABLED_ANGLE, i));
        }
        ASSERT_EGL_SUCCESS();
    }
}

ANGLE_INSTANTIATE_TEST(EGLQueryStringIndexedTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES2_VULKAN(),
                       ES3_D3D11(),
                       ES3_OPENGL());
