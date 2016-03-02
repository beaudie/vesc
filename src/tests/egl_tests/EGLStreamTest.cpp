//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLStreamTest:
//   Tests pertaining to egl::Stream.
//

#include <gtest/gtest.h>

#include <vector>

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class EGLStreamTest : public ANGLETest
{
  protected:
    EGLStreamTest()
        : mDisplay(EGL_NO_DISPLAY),
          mWindowSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mOSWindow(nullptr)
    {
    }

    void SetUp() override
    {
        mOSWindow = CreateOSWindow();
        mOSWindow->initialize("EGLStreamTest", 64, 64);
    }

    // Release any resources created in the test body
    void TearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mWindowSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mWindowSurface);
                mWindowSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        SafeDelete(mOSWindow);

        ASSERT_TRUE(mWindowSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void initializeDisplay(EGLenum platformType)
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
            reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
                eglGetProcAddress("eglGetPlatformDisplayEXT"));
        ASSERT_NE(eglGetPlatformDisplayEXT, nullptr);

        std::vector<EGLint> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platformType);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);

        if (platformType == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE ||
            platformType == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
            displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
        }
        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                            displayAttributes.data());
        ASSERT_NE(EGL_NO_DISPLAY, mDisplay);

        EGLint majorVersion, minorVersion;
        ASSERT_EQ(eglInitialize(mDisplay, &majorVersion, &minorVersion), EGL_TRUE);

        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_EQ(eglGetError(), EGL_SUCCESS);
    }

    void initializeSurface(EGLConfig config)
    {
        mConfig = config;

        std::vector<EGLint> surfaceAttributes;
        surfaceAttributes.push_back(EGL_NONE);
        surfaceAttributes.push_back(EGL_NONE);

        // Create window surface
        mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                                &surfaceAttributes[0]);
        ASSERT_EQ(eglGetError(), EGL_SUCCESS);

        EGLint contextAttibutes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

        mContext = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_EQ(eglGetError(), EGL_SUCCESS);
    }

    void initializeSurfaceWithDefaultConfig()
    {
        const EGLint configAttributes[] = {EGL_RED_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_GREEN_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_BLUE_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_ALPHA_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_DEPTH_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_STENCIL_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_SAMPLE_BUFFERS,
                                           0,
                                           EGL_NONE};

        EGLint configCount;
        EGLConfig config;
        ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &config, 1, &configCount) ||
                    (configCount != 1) == EGL_TRUE);

        initializeSurface(config);
    }

    EGLDisplay mDisplay;
    EGLSurface mWindowSurface;
    EGLContext mContext;
    EGLConfig mConfig;
    OSWindow *mOSWindow;
};

// Tests validation of the stream API
TEST_P(EGLStreamTest, StreamValidationTest)
{
    const char *extensionsString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (strstr(extensionsString, "EGL_KHR_stream") == nullptr)
    {
        std::cout << "Stream extension not supported" << std::endl;
        return;
    }

    initializeDisplay(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
    initializeSurfaceWithDefaultConfig();

    const EGLint streamAttributesBad[] = {
        EGL_STREAM_STATE_KHR,
        0,
        EGL_NONE,
        EGL_PRODUCER_FRAME_KHR,
        0,
        EGL_NONE,
        EGL_CONSUMER_FRAME_KHR,
        0,
        EGL_NONE,
        EGL_CONSUMER_LATENCY_USEC_KHR,
        -1,
        EGL_NONE,
        EGL_RED_SIZE,
        EGL_DONT_CARE,
        EGL_NONE,
    };

    // Validate create stream attributes
    EGLStreamKHR stream = eglCreateStreamKHR(mDisplay, &streamAttributesBad[0]);
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    stream = eglCreateStreamKHR(mDisplay, &streamAttributesBad[3]);
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    stream = eglCreateStreamKHR(mDisplay, &streamAttributesBad[6]);
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    stream = eglCreateStreamKHR(mDisplay, &streamAttributesBad[9]);
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    stream = eglCreateStreamKHR(mDisplay, &streamAttributesBad[12]);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    const EGLint streamAttributes[] = {
        EGL_CONSUMER_LATENCY_USEC_KHR, 0, EGL_NONE,
    };

    stream = eglCreateStreamKHR(EGL_NO_DISPLAY, streamAttributes);
    ASSERT_EGL_ERROR(EGL_BAD_DISPLAY);
    ASSERT_EQ(stream, EGL_NO_STREAM_KHR);

    // Create an actual stream
    stream = eglCreateStreamKHR(mDisplay, streamAttributes);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(stream, EGL_NO_STREAM_KHR);

    // Assert it is in the created state
    EGLint state;
    eglQueryStreamKHR(mDisplay, stream, EGL_STREAM_STATE_KHR, &state);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(state, EGL_STREAM_STATE_CREATED_KHR);

    // Test getting and setting the latency
    EGLint latency = 10;
    eglStreamAttribKHR(mDisplay, stream, EGL_CONSUMER_LATENCY_USEC_KHR, latency);
    ASSERT_EGL_SUCCESS();
    eglQueryStreamKHR(mDisplay, stream, EGL_CONSUMER_LATENCY_USEC_KHR, &latency);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(10, latency);
    eglStreamAttribKHR(mDisplay, stream, EGL_CONSUMER_LATENCY_USEC_KHR, -1);
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    ASSERT_EQ(10, latency);

    // Test the 64-bit queries
    EGLuint64KHR value;
    eglQueryStreamu64KHR(mDisplay, stream, EGL_CONSUMER_FRAME_KHR, &value);
    ASSERT_EGL_SUCCESS();
    eglQueryStreamu64KHR(mDisplay, stream, EGL_PRODUCER_FRAME_KHR, &value);
    ASSERT_EGL_SUCCESS();

    // Destroy the stream
    eglDestroyStreamKHR(mDisplay, stream);
    ASSERT_EGL_SUCCESS();
}

ANGLE_INSTANTIATE_TEST(EGLStreamTest, ES2_D3D11());
} // namespace
