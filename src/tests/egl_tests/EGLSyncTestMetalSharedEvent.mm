//
//  Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSyncTestMetalSharedEvent:
//   Tests pertaining to EGL_ANGLE_sync_mtl_shared_event extension.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"

#include <Metal/Metal.h>

using namespace angle;

class EGLSyncTestMetalSharedEvent : public ANGLETest<>
{
  protected:
    id<MTLSharedEvent> createMetalSharedEvent() const
    {
        id<MTLDevice> device = getMetalDevice();
        return [device newSharedEvent];
    }

    id<MTLDevice> getMetalDevice() const
    {
        EGLAttrib angleDevice = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

        EGLAttrib device = 0;
        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_METAL_DEVICE_ANGLE, &device));

        return (__bridge id<MTLDevice>)reinterpret_cast<void *>(device);
    }

    bool hasEGLDisplayExtension(const char *extname) const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), extname);
    }

    bool hasSyncMetalSharedEventExtension() const
    {
        return hasEGLDisplayExtension("EGL_ANGLE_metal_shared_event_sync");
    }
};

// Test usage of eglGetSyncAttrib
TEST_P(EGLSyncTestMetalSharedEvent, GetSyncAttrib)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    sharedEvent.label              = @"TestSharedEvent";
    EXPECT_EQ([sharedEvent retainCount], 1ul);

    EGLDisplay display      = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, (EGLAttrib)sharedEvent,
                               EGL_NONE};
    EXPECT_EQ([sharedEvent retainCount], 1ul);

    EGLSync sync = eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(sync, EGL_NO_SYNC);
    // sharedEvent, sync, mtlCommandBuffer
    EXPECT_EQ([sharedEvent retainCount], 3ul);

    // Fence sync attributes are:
    //
    // EGL_SYNC_TYPE: EGL_SYNC_METAL_SHARED_EVENT_ANGLE
    // EGL_SYNC_STATUS: EGL_UNSIGNALED or EGL_SIGNALED
    // EGL_SYNC_CONDITION: EGL_SYNC_PRIOR_COMMANDS_COMPLETE

    constexpr EGLAttrib kSentinelAttribValue = 123456789;
    EGLAttrib attribValue                    = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_TYPE, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_METAL_SHARED_EVENT_ANGLE);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_CONDITION, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_PRIOR_COMMANDS_COMPLETE);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, sync, EGL_SYNC_STATUS, &attribValue));

    glFlush();

    // Hack around EXPECT_* not having an "either this or that" variant:
    if (attribValue != EGL_SIGNALED)
    {
        EXPECT_EQ(attribValue, EGL_UNSIGNALED);
    }

    EXPECT_EQ([sharedEvent retainCount], 2ul);

    EXPECT_EGL_TRUE(eglDestroySync(display, sync));
    EXPECT_EQ([sharedEvent retainCount], 1ul);

    sharedEvent = nil;
}

// Verify CreateSync and ClientWait for EGL_ANGLE_metal_shared_event_sync
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_ClientWait)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    sharedEvent.label              = @"TestSharedEvent";

    EGLDisplay display      = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, (EGLAttrib)sharedEvent,
                               EGL_NONE};

    // We can ClientWait on this
    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);

    // Create work to do
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Wait for the draw to complete
    glClear(GL_COLOR_BUFFER_BIT);

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
    EGLAttrib value             = 0;
    ASSERT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, kTimeout));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent));
    sharedEvent = nil;
}

// Verify CreateSync and ClientWait for EGL_ANGLE_metal_shared_event_sync
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_ClientWait_WithSignalValue)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    sharedEvent.label              = @"TestSharedEvent";

    constexpr uint64_t kSignalValue = 0xDEADBEEFCAFE;
    EGLDisplay display              = getEGLWindow()->getDisplay();
    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, (EGLAttrib)sharedEvent,
                               EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_ANGLE, kSignalValue,
                               EGL_NONE};

    // We can ClientWait on this
    EGLSync syncWithSharedEvent =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent, EGL_NO_SYNC);

    // Create work to do
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Wait for the draw to complete
    glClear(GL_COLOR_BUFFER_BIT);

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
    EGLAttrib value             = 0;
    ASSERT_EQ(EGL_CONDITION_SATISFIED, eglClientWaitSync(display, syncWithSharedEvent,
                                                         EGL_SYNC_FLUSH_COMMANDS_BIT, kTimeout));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);
    ASSERT_EQ(sharedEvent.signaledValue, kSignalValue);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent));
    sharedEvent = nil;
}

// Verify WaitSync with EGL_ANGLE_metal_shared_event_sync
// Simulate passing shared events across processes by passing across Contexts.
TEST_P(EGLSyncTestMetalSharedEvent, AngleMetalSharedEventSync_WaitSync)
{
    ANGLE_SKIP_TEST_IF(!hasSyncMetalSharedEventExtension());

    id<MTLSharedEvent> sharedEvent = createMetalSharedEvent();
    sharedEvent.label              = @"TestSharedEvent";

    EGLAttrib value    = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();

    /*- First Context ------------------------*/

    EGLAttrib syncAttribs[] = {EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, (EGLAttrib)sharedEvent,
                               EGL_NONE};
    // We can ClientWait on this

    EGLSync syncWithSharedEvent1 =
        eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
    EXPECT_NE(syncWithSharedEvent1, EGL_NO_SYNC);

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    /*- Second Context ------------------------*/
    if (syncWithSharedEvent1 != EGL_NO_SYNC)
    {
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        EGLContext context2 = getEGLWindow()->createContext(EGL_NO_CONTEXT, nullptr);
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context2));

        // We can eglWaitSync on this - import FD from first sync.
        EGLSync syncWithSharedEvent2 =
            eglCreateSync(display, EGL_SYNC_METAL_SHARED_EVENT_ANGLE, syncAttribs);
        EXPECT_NE(syncWithSharedEvent2, EGL_NO_SYNC);

        // Second draw waits for first to complete. May already be signaled - ignore error.
        if (eglWaitSync(display, syncWithSharedEvent2, 0) == EGL_TRUE)
        {
            // Create work to do
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glFlush();
        }

        // Wait for second draw to complete
        EXPECT_EQ(EGL_CONDITION_SATISFIED,
                  eglClientWaitSync(display, syncWithSharedEvent2, EGL_SYNC_FLUSH_COMMANDS_BIT,
                                    1'000'000'000));
        EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent2, EGL_SYNC_STATUS, &value));
        EXPECT_EQ(value, EGL_SIGNALED);

        // Reset to default context and surface.
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, getEGLWindow()->getContext()));

        // Clean up created objects.
        EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent2));
        EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
    }

    // Wait for first draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSync(display, syncWithSharedEvent1, EGL_SYNC_FLUSH_COMMANDS_BIT,
                                1'000'000'000));
    EXPECT_EGL_TRUE(eglGetSyncAttrib(display, syncWithSharedEvent1, EGL_SYNC_STATUS, &value));
    EXPECT_EQ(value, EGL_SIGNALED);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySync(display, syncWithSharedEvent1));
    sharedEvent = nil;
}

ANGLE_INSTANTIATE_TEST(EGLSyncTestMetalSharedEvent, ES2_METAL(), ES3_METAL());
