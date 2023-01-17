//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.h : Defines functions for querying the thread-local GL and EGL state.

#ifndef LIBGLESV2_GLOBALSTATE_H_
#define LIBGLESV2_GLOBALSTATE_H_

#include "libANGLE/Context.h"
#include "libANGLE/Debug.h"
#include "libANGLE/GlobalMutex.h"
#include "libANGLE/SharedContextMutex.h"
#include "libANGLE/Thread.h"
#include "libANGLE/features.h"

#if defined(ANGLE_PLATFORM_APPLE) || (ANGLE_PLATFORM_ANDROID)
#    include "common/tls.h"
#endif

#include <mutex>

namespace egl
{
class Debug;
class Thread;

#if defined(ANGLE_PLATFORM_APPLE)
extern Thread *GetCurrentThreadTLS();
extern void SetCurrentThreadTLS(Thread *thread);
#else
extern thread_local Thread *gCurrentThread;
#endif

gl::Context *GetGlobalLastContext();
void SetGlobalLastContext(gl::Context *context);
Thread *GetCurrentThread();
Debug *GetDebug();

// Sync the current context from Thread to global state.
class [[nodiscard]] ScopedSyncCurrentContextFromThread
{
  public:
    ScopedSyncCurrentContextFromThread(egl::Thread *thread);
    ~ScopedSyncCurrentContextFromThread();

  private:
    egl::Thread *const mThread;
};

}  // namespace egl

#define ANGLE_SCOPED_GLOBAL_LOCK() egl::ScopedGlobalMutexLock globalMutexLock

namespace gl
{
ANGLE_INLINE Context *GetGlobalContext()
{
#if defined(ANGLE_PLATFORM_APPLE)
    egl::Thread *currentThread = egl::GetCurrentThreadTLS();
#else
    egl::Thread *currentThread = egl::gCurrentThread;
#endif
    ASSERT(currentThread);
    return currentThread->getContext();
}

ANGLE_INLINE Context *GetValidGlobalContext()
{
#if defined(ANGLE_USE_ANDROID_TLS_SLOT)
    // TODO: Replace this branch with a compile time flag (http://anglebug.com/4764)
    if (angle::gUseAndroidOpenGLTlsSlot)
    {
        return static_cast<gl::Context *>(ANGLE_ANDROID_GET_GL_TLS()[angle::kAndroidOpenGLTlsSlot]);
    }
#endif

#if defined(ANGLE_PLATFORM_APPLE)
    return GetCurrentValidContextTLS();
#else
    return gCurrentValidContext;
#endif
}

// Generate a context lost error on the context if it is non-null and lost.
void GenerateContextLostErrorOnContext(Context *context);
void GenerateContextLostErrorOnCurrentGlobalContext();

#if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
// TODO(b/177574181): This should be handled in a backend-specific way.
// if previous context different from current context, dirty all state
static ANGLE_INLINE void DirtyContextIfNeeded(Context *context)
{
    if (context && context != egl::GetGlobalLastContext())
    {
        context->dirtyAllState();
        SetGlobalLastContext(context);
    }
}

#endif

#if !defined(ANGLE_ENABLE_SHARE_CONTEXT_LOCK)
#    define SCOPED_SHARE_CONTEXT_LOCK(context)
#    define SCOPED_GLOBAL_AND_SHARE_CONTEXT_LOCK(context) ANGLE_SCOPED_GLOBAL_LOCK()
#else
#    if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
#        define SCOPED_SHARE_CONTEXT_LOCK(context)       \
            egl::ScopedGlobalMutexLock shareContextLock; \
            DirtyContextIfNeeded(context)
#        define SCOPED_GLOBAL_AND_SHARE_CONTEXT_LOCK(context) SCOPED_SHARE_CONTEXT_LOCK(context)
#    elif !defined(ANGLE_ENABLE_SHARED_CONTEXT_MUTEX)
#        define SCOPED_SHARE_CONTEXT_LOCK(context) \
            egl::ScopedOptionalGlobalMutexLock shareContextLock(context->isShared())
#        define SCOPED_GLOBAL_AND_SHARE_CONTEXT_LOCK(context) ANGLE_SCOPED_GLOBAL_LOCK()
#    else
#        define SCOPED_SHARE_CONTEXT_LOCK(context) \
            std::lock_guard<egl::ContextMutex> shareContextLock(*context->getContextMutex())
#        define SCOPED_GLOBAL_AND_SHARE_CONTEXT_LOCK(context) \
            ANGLE_SCOPED_GLOBAL_LOCK();                       \
            SCOPED_SHARE_CONTEXT_LOCK(context)
#    endif
#endif

#if !defined(ANGLE_ENABLE_SHARED_CONTEXT_MUTEX)
#    define ANGLE_EGL_CONTEXT_LOCK(EP, THREAD, ...) egl::ContextMutex *const contextMutex = nullptr
#    define ANGLE_EGL_CONTEXT_UNLOCK()
#else
#    define ANGLE_EGL_CONTEXT_LOCK(EP, THREAD, ...)                                          \
        egl::ContextMutex *const contextMutex = GetContextMutex_##EP(THREAD, ##__VA_ARGS__); \
        if (ANGLE_UNLIKELY(contextMutex))                                                    \
        contextMutex->lock()
#    define ANGLE_EGL_CONTEXT_UNLOCK()    \
        if (ANGLE_UNLIKELY(contextMutex)) \
        contextMutex->unlock()

ANGLE_INLINE egl::ContextMutex *TryGetContextMutex(const Context *context)
{
    return context != nullptr ? context->getContextMutex() : nullptr;
}

ANGLE_INLINE egl::ContextMutex *TryGetActiveSharedContextMutex(Context *context)
{
    if (context != nullptr)
    {
        context->ensureSharedMutexActive();
        return context->getContextMutex();
    }
    return nullptr;
}
#endif

}  // namespace gl

#if defined(ANGLE_ENABLE_SHARED_CONTEXT_MUTEX)
namespace egl
{
ANGLE_INLINE ContextMutex *TryGetContextMutex(const Thread *thread)
{
    return TryGetContextMutex(thread->getContext());
}
}  // namespace egl
#endif

#endif  // LIBGLESV2_GLOBALSTATE_H_
