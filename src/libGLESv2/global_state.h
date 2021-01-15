//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.h : Defines functions for querying the thread-local GL and EGL state.

#ifndef LIBGLESV2_GLOBALSTATE_H_
#define LIBGLESV2_GLOBALSTATE_H_

#include "common/android_tls.h"
#include "libANGLE/Context.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Thread.h"
#include "libANGLE/features.h"

#include <mutex>

namespace angle
{
using GlobalMutex = std::recursive_mutex;
}  // namespace angle

namespace egl
{
class Debug;
class Thread;

extern thread_local Thread *gCurrentThread;

angle::GlobalMutex &GetGlobalMutex();
Thread *GetCurrentThread();
Debug *GetDebug();
void SetContextCurrent(Thread *thread, gl::Context *context);
}  // namespace egl

#define ANGLE_SCOPED_GLOBAL_LOCK() \
    std::lock_guard<angle::GlobalMutex> globalMutexLock(egl::GetGlobalMutex())

namespace gl
{
ANGLE_INLINE Context *GetGlobalContext()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // TODO: Replace this branch with a compile time flag (http://anglebug.com/4764)
    if (angle::gUseAndroidOpenGLTlsSlot)
    {
        return angle::android::GetTlsContextAndroid();
    }
#endif

    ASSERT(egl::gCurrentThread);
    return egl::gCurrentThread->getContext();
}

ANGLE_INLINE Context *GetValidGlobalContext()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // TODO: Replace this branch with a compile time flag (http://anglebug.com/4764)
    if (angle::gUseAndroidOpenGLTlsSlot)
    {
        Context *context = angle::android::GetTlsContextAndroid();
        if (context && !context->isContextLost())
        {
            return context;
        }
    }
#endif

    return gCurrentValidContext;
}

// Generate a context lost error on the context if it is non-null and lost.
void GenerateContextLostErrorOnContext(Context *context);
void GenerateContextLostErrorOnCurrentGlobalContext();

ANGLE_INLINE std::unique_lock<angle::GlobalMutex> GetShareGroupLock(const Context *context)
{
    return context->isShared() ? std::unique_lock<angle::GlobalMutex>(egl::GetGlobalMutex())
                               : std::unique_lock<angle::GlobalMutex>();
}

}  // namespace gl

#endif  // LIBGLESV2_GLOBALSTATE_H_
