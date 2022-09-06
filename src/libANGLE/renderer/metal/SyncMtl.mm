//
// Copyright (c) 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncMtl:
//    Defines the class interface for SyncMtl, implementing SyncImpl.
//

#include "libANGLE/renderer/metal/SyncMtl.h"

#include <chrono>

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/mtl_resources.h"

namespace rx
{
namespace mtl
{
// SharedEvent is only available on iOS 12.0+ or mac 10.14+
#if ANGLE_MTL_EVENT_AVAILABLE
Sync::Sync() {}
Sync::~Sync() {}

void Sync::onDestroy()
{
    mMetalSharedEvent     = nil;
    mCv                   = nullptr;
    mLock                 = nullptr;
    mCmdBufferTrackingRef = nullptr;
}

angle::Result Sync::initialize(ContextMtl *contextMtl)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        mMetalSharedEvent = contextMtl->getMetalDevice().newSharedEvent();
    }

    mSetCounter = mMetalSharedEvent.get().signaledValue;

    mCv.reset(new std::condition_variable());
    mLock.reset(new std::mutex());

    mCmdBufferTrackingRef.reset(new ResourceCommandBufferTracking());
    return angle::Result::Continue;
}

angle::Result Sync::set(ContextMtl *contextMtl, GLenum condition, GLbitfield flags)
{
    if (!mMetalSharedEvent)
    {
        ANGLE_TRY(initialize(contextMtl));
    }
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE);
    ASSERT(flags == 0);

    mSetCounter++;
    contextMtl->queueEventSignal(mMetalSharedEvent, mSetCounter, mCmdBufferTrackingRef);
    return angle::Result::Continue;
}
angle::Result Sync::clientWait(DisplayMtl *displayMtl,
                               ContextMtl *contextMtl,
                               bool flushCommands,
                               uint64_t timeout,
                               GLenum *outResult)
{
    std::unique_lock<std::mutex> lg(*mLock);
    if (mMetalSharedEvent.get().signaledValue >= mSetCounter)
    {
        *outResult = GL_ALREADY_SIGNALED;
        return angle::Result::Continue;
    }
    if (flushCommands)
    {
        contextMtl->flushCommandBuffer(mtl::WaitUntilScheduled);
    }

    if (timeout == 0)
    {
        *outResult = GL_TIMEOUT_EXPIRED;

        return angle::Result::Continue;
    }

    flushPendingContextsIfNeeded(contextMtl);

    // Create references to mutex and condition variable since they might be released in
    // onDestroy(), but the callback might still not be fired yet.
    std::shared_ptr<std::condition_variable> cvRef = mCv;
    std::shared_ptr<std::mutex> lockRef            = mLock;
    AutoObjCObj<MTLSharedEventListener> eventListener =
        displayMtl->getOrCreateSharedEventListener();
    [mMetalSharedEvent.get() notifyListener:eventListener
                                    atValue:mSetCounter
                                      block:^(id<MTLSharedEvent> sharedEvent, uint64_t value) {
                                        std::unique_lock<std::mutex> localLock(*lockRef);
                                        cvRef->notify_one();
                                      }];

    if (!mCv->wait_for(lg, std::chrono::nanoseconds(timeout),
                       [this] { return mMetalSharedEvent.get().signaledValue >= mSetCounter; }))
    {
        *outResult = GL_TIMEOUT_EXPIRED;
        return angle::Result::Incomplete;
    }

    ASSERT(mMetalSharedEvent.get().signaledValue >= mSetCounter);
    *outResult = GL_CONDITION_SATISFIED;

    return angle::Result::Continue;
}
void Sync::serverWait(ContextMtl *contextMtl)
{
    flushPendingContextsIfNeeded(contextMtl);
    contextMtl->serverWaitEvent(mMetalSharedEvent, mSetCounter);
}
angle::Result Sync::getStatus(bool *signaled)
{
    *signaled = mMetalSharedEvent.get().signaledValue >= mSetCounter;
    return angle::Result::Continue;
}

void Sync::flushPendingContextsIfNeeded(ContextMtl *contextMtl)
{
    if (!contextMtl)
    {
        // Don't do any flush if there is no current context.
        // This can happen if Sync is EGLSync.
        return;
    }
    // Flush any pending contexts (except current context) that are currently scheduled to signal
    // this event.
    angle::FastVector<ContextMtl *, 2> contextsToFlush;
    for (ContextMtl *pendingCtx : mCmdBufferTrackingRef->pendingContexts)
    {
        // For current context, only flush if
        // GL_SYNC_FLUSH_COMMANDS_BIT/EGL_SYNC_FLUSH_COMMANDS_BIT_KHR
        // flag is specified.
        if (pendingCtx == contextMtl)
        {
            continue;
        }
        // Only flush context in the same shared group as the current context.
        if (pendingCtx->getState().getShareGroup() == contextMtl->getState().getShareGroup())
        {
            contextsToFlush.push_back(pendingCtx);
        }
    }

    for (ContextMtl *pendingCtx : contextsToFlush)
    {
        pendingCtx->flushCommandBuffer(mtl::NoWait);
    }
}
#endif  // #if defined(__IPHONE_12_0) || defined(__MAC_10_14)
}  // namespace mtl

// FenceNVMtl implementation
FenceNVMtl::FenceNVMtl() : FenceNVImpl() {}

FenceNVMtl::~FenceNVMtl() {}

void FenceNVMtl::onDestroy(const gl::Context *context)
{
    mSync.onDestroy();
}

angle::Result FenceNVMtl::set(const gl::Context *context, GLenum condition)
{
    ASSERT(condition == GL_ALL_COMPLETED_NV);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    return mSync.set(contextMtl, GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

angle::Result FenceNVMtl::test(const gl::Context *context, GLboolean *outFinished)
{
    bool signaled = false;
    ANGLE_TRY(mSync.getStatus(&signaled));

    *outFinished = signaled ? GL_TRUE : GL_FALSE;
    return angle::Result::Continue;
}

angle::Result FenceNVMtl::finish(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    uint64_t timeout       = 1000000000ul;
    GLenum result;
    do
    {
        ANGLE_TRY(mSync.clientWait(contextMtl->getDisplay(), contextMtl, true, timeout, &result));
    } while (result == GL_TIMEOUT_EXPIRED);

    if (result == GL_WAIT_FAILED)
    {
        UNREACHABLE();
        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

// SyncMtl implementation
SyncMtl::SyncMtl() : SyncImpl() {}

SyncMtl::~SyncMtl() {}

void SyncMtl::onDestroy(const gl::Context *context)
{
    mSync.onDestroy();
}

angle::Result SyncMtl::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    return mSync.set(contextMtl, condition, flags);
}

angle::Result SyncMtl::clientWait(const gl::Context *context,
                                  GLbitfield flags,
                                  GLuint64 timeout,
                                  GLenum *outResult)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    ASSERT((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) == 0);

    bool flush = (flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0;

    return mSync.clientWait(contextMtl->getDisplay(), contextMtl, flush, timeout, outResult);
}

angle::Result SyncMtl::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    ASSERT(flags == 0);
    ASSERT(timeout == GL_TIMEOUT_IGNORED);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    mSync.serverWait(contextMtl);
    return angle::Result::Continue;
}

angle::Result SyncMtl::getStatus(const gl::Context *context, GLint *outResult)
{
    bool signaled = false;
    ANGLE_TRY(mSync.getStatus(&signaled));

    *outResult = signaled ? GL_SIGNALED : GL_UNSIGNALED;
    return angle::Result::Continue;
}

// EGLSyncMtl implementation
EGLSyncMtl::EGLSyncMtl(const egl::AttributeMap &attribs) : EGLSyncImpl()
{
    ASSERT(attribs.isEmpty());
}

EGLSyncMtl::~EGLSyncMtl() {}

void EGLSyncMtl::onDestroy(const egl::Display *display)
{
    mSync.onDestroy();
}

egl::Error EGLSyncMtl::initialize(const egl::Display *display,
                                  const gl::Context *context,
                                  EGLenum type)
{
    ASSERT(type == EGL_SYNC_FENCE_KHR);
    ASSERT(context != nullptr);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    if (IsError(mSync.set(contextMtl, GL_SYNC_GPU_COMMANDS_COMPLETE, 0)))
    {
        return egl::Error(EGL_BAD_ALLOC, "eglCreateSyncKHR failed to create sync object");
    }

    return egl::NoError();
}

egl::Error EGLSyncMtl::clientWait(const egl::Display *display,
                                  const gl::Context *context,
                                  EGLint flags,
                                  EGLTime timeout,
                                  EGLint *outResult)
{
    ASSERT((flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) == 0);

    bool flush = (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) != 0;
    GLenum result;
    DisplayMtl *displayMtl = mtl::GetImpl(display);
    ContextMtl *contextMtl = context ? mtl::GetImpl(context) : nullptr;
    if (IsError(mSync.clientWait(displayMtl, contextMtl, flush, static_cast<uint64_t>(timeout),
                                 &result)))
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    switch (result)
    {
        case GL_ALREADY_SIGNALED:
            // fall through.  EGL doesn't differentiate between event being already set, or set
            // before timeout.
        case GL_CONDITION_SATISFIED:
            *outResult = EGL_CONDITION_SATISFIED_KHR;
            return egl::NoError();

        case GL_TIMEOUT_EXPIRED:
            *outResult = EGL_TIMEOUT_EXPIRED_KHR;
            return egl::NoError();

        default:
            UNREACHABLE();
            *outResult = EGL_FALSE;
            return egl::Error(EGL_BAD_ALLOC);
    }
}

egl::Error EGLSyncMtl::serverWait(const egl::Display *display,
                                  const gl::Context *context,
                                  EGLint flags)
{
    // Server wait requires a valid bound context.
    ASSERT(context);

    // No flags are currently implemented.
    ASSERT(flags == 0);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    mSync.serverWait(contextMtl);
    return egl::NoError();
}

egl::Error EGLSyncMtl::getStatus(const egl::Display *display, EGLint *outStatus)
{
    bool signaled = false;
    if (IsError(mSync.getStatus(&signaled)))
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    *outStatus = signaled ? EGL_SIGNALED_KHR : EGL_UNSIGNALED_KHR;
    return egl::NoError();
}

egl::Error EGLSyncMtl::dupNativeFenceFD(const egl::Display *display, EGLint *result) const
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}

}  // namespace rx
