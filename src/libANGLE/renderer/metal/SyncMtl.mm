//
// Copyright (c) 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncMtl:
//    Defines the class interface for SyncMtl, implementing SyncImpl.
//

#include "libANGLE/renderer/metal/SyncMtl.h"

#include <time.h>
#include <chrono>

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

namespace rx
{
namespace mtl
{

// MTLEvent and MTLSharedEvent are only available on iOS 12.0+ or mac 10.14+
#if ANGLE_MTL_EVENT_AVAILABLE
Sync::Sync() {}
Sync::~Sync() {}

void Sync::onDestroy()
{
    mMetalEvent = nil;
}

// MTLEvent starts with a value of 0, use 1 to signal it.
static constexpr uint64_t kEventSignalValue = 1;

angle::Result Sync::initialize(ContextMtl *contextMtl, id<MTLEvent> event)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        if (event)
        {
            mMetalEvent.retainAssign(event);
        }
        else
        {
            mMetalEvent = contextMtl->getMetalDevice().newEvent();
        }
    }

    return angle::Result::Continue;
}

angle::Result Sync::set(ContextMtl *contextMtl,
                        GLenum condition,
                        GLbitfield flags,
                        id<MTLEvent> event,
                        std::shared_ptr<Sync> sharedThis)
{
    if (!mMetalEvent)
    {
        ANGLE_TRY(initialize(contextMtl, event));
    }
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE);
    ASSERT(flags == 0);

    if (condition == GL_SYNC_GPU_COMMANDS_COMPLETE)
    {
        contextMtl->queueEventSignal(mMetalEvent, kEventSignalValue, sharedThis);
    }

    return angle::Result::Continue;
}

angle::Result Sync::clientWait(ContextMtl *contextMtl,
                               bool flushCommands,
                               uint64_t timeout,
                               GLenum *outResult)
{
    if (contextMtl->isEventCompleted(this))
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

    // TODO(anglebug.com/8153): add a callback mechanism to
    // CommandBuffer to be able to wait until a command buffer serial
    // is reached.
    // Passing EGL_FOREVER_KHR overflows std::chrono::nanoseconds.
    const uint64_t kNanosecondsPerDay = 86400000000000;
    if (timeout > kNanosecondsPerDay)
    {
        timeout = kNanosecondsPerDay;
    }

    struct timespec rqtp, rmtp;

    const uint32_t kHalfMillisecondInNanos = 500000;

    while (timeout)
    {
        rqtp.tv_sec  = 0;
        rqtp.tv_nsec = kHalfMillisecondInNanos;
        rmtp.tv_sec  = 0;
        rmtp.tv_nsec = 0;
        if (!nanosleep(&rqtp, &rmtp))
        {
            // Successful sleep.
            timeout -= std::min(timeout, static_cast<uint64_t>(kHalfMillisecondInNanos));
        }
        else
        {
            timeout -= (kHalfMillisecondInNanos - rmtp.tv_nsec);
        }

        if (contextMtl->isEventCompleted(this))
        {
            *outResult = GL_CONDITION_SATISFIED;
            return angle::Result::Continue;
        }
    }

    *outResult = GL_TIMEOUT_EXPIRED;
    return angle::Result::Incomplete;
}
void Sync::serverWait(ContextMtl *contextMtl)
{
    contextMtl->serverWaitEvent(mMetalEvent, kEventSignalValue);
}
angle::Result Sync::getStatus(ContextMtl *contextMtl, bool *signaled)
{
    *signaled = contextMtl->isEventCompleted(this);
    return angle::Result::Continue;
}

void Sync::setEnqueuedCommandBufferSerial(uint64_t serial)
{
    mEncodedCommandBufferSerial = serial;
}

uint64_t Sync::getEnqueuedCommandBufferSerial()
{
    return mEncodedCommandBufferSerial;
}
#endif  // #if defined(__IPHONE_12_0) || defined(__MAC_10_14)
}  // namespace mtl

// FenceNVMtl implementation
FenceNVMtl::FenceNVMtl() : FenceNVImpl(), mSync(std::make_shared<mtl::Sync>()) {}

FenceNVMtl::~FenceNVMtl() {}

void FenceNVMtl::onDestroy(const gl::Context *context)
{
    mSync->onDestroy();
}

angle::Result FenceNVMtl::set(const gl::Context *context, GLenum condition)
{
    ASSERT(condition == GL_ALL_COMPLETED_NV);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    return mSync->set(contextMtl, GL_SYNC_GPU_COMMANDS_COMPLETE, 0, nil, mSync);
}

angle::Result FenceNVMtl::test(const gl::Context *context, GLboolean *outFinished)
{
    bool signaled = false;
    ANGLE_TRY(mSync->getStatus(mtl::GetImpl(context), &signaled));

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
        ANGLE_TRY(mSync->clientWait(contextMtl, true, timeout, &result));
    } while (result == GL_TIMEOUT_EXPIRED);

    if (result == GL_WAIT_FAILED)
    {
        UNREACHABLE();
        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

// SyncMtl implementation
SyncMtl::SyncMtl() = default;

SyncMtl::~SyncMtl() = default;

void SyncMtl::onDestroy(const gl::Context *context)
{
    mSync->onDestroy();
}

angle::Result SyncMtl::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    // set() is only called once.
    ASSERT(!mSync);
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE);
    ASSERT(flags == 0);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    mSync                  = std::make_shared<mtl::Sync>();
    return mSync->set(contextMtl, condition, flags, nil, mSync);
}

angle::Result SyncMtl::clientWait(const gl::Context *context,
                                  GLbitfield flags,
                                  GLuint64 timeout,
                                  GLenum *outResult)
{
    ASSERT((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) == 0);

    bool flushCommands     = ((flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    return mSync->clientWait(contextMtl, flushCommands, timeout, outResult);
}

angle::Result SyncMtl::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    ASSERT(flags == 0);
    ASSERT(timeout == GL_TIMEOUT_IGNORED);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    mSync->serverWait(contextMtl);
    return angle::Result::Continue;
}

angle::Result SyncMtl::getStatus(const gl::Context *context, GLint *outResult)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    bool completed         = contextMtl->isEventCompleted(mSync.get());
    *outResult             = completed ? GL_SIGNALED : GL_UNSIGNALED;
    return angle::Result::Continue;
}

// EGLSyncMtl implementation
static uint64_t makeSignalValue(EGLAttrib highPart, EGLAttrib lowPart)
{
    return (static_cast<uint64_t>(highPart & 0xFFFFFFFF) << 32) |
           (static_cast<uint64_t>(lowPart & 0xFFFFFFFF));
}

EGLSyncMtl::EGLSyncMtl(const egl::AttributeMap &attribs) : EGLSyncImpl()
{
    id<MTLSharedEvent> sharedEvent = (__bridge id<MTLSharedEvent>)reinterpret_cast<void *>(
        attribs.get(EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE, 0));
    if (sharedEvent)
    {
        mSharedEvent.retainAssign(sharedEvent);
    }

    if (attribs.contains(EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE) ||
        attribs.contains(EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE))
    {
        mSignalValue =
            makeSignalValue(attribs.get(EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE, 0),
                            attribs.get(EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE, 0));
    }

    // Translate conditions that aren't EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE to
    // GL_SYNC_GPU_COMMANDS_COMPLETE. This is to translate EGL_SYNC_PRIOR_COMMANDS_COMPLETE, from
    // the EGLSync layer, to the GL equivalent used in mtl::Sync.
    mCondition =
        attribs.getAsInt(EGL_SYNC_CONDITION, 0) == EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE
            ? EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE
            : GL_SYNC_GPU_COMMANDS_COMPLETE;
}

EGLSyncMtl::~EGLSyncMtl() {}

void EGLSyncMtl::onDestroy(const egl::Display *display)
{
    mSharedEvent = nil;
    mCv          = nullptr;
    mLock        = nullptr;
}

angle::Result EGLSyncMtl::initializeImpl(ContextMtl *contextMtl,
                                         id<MTLSharedEvent> sharedEvent,
                                         Optional<uint64_t> signalValue)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        if (sharedEvent)
        {
            mSharedEvent.retainAssign(sharedEvent);
        }
        else
        {
            mSharedEvent = contextMtl->getMetalDevice().newSharedEvent();
        }
    }

    auto signaledValue = mSharedEvent.get().signaledValue;
    mSignalValue       = signalValue.valid() ? signalValue.value() : signaledValue + 1;

    mCv.reset(new std::condition_variable());
    mLock.reset(new std::mutex());
    return angle::Result::Continue;
}

angle::Result EGLSyncMtl::setImpl(ContextMtl *contextMtl,
                                  GLenum condition,
                                  GLbitfield flags,
                                  id<MTLSharedEvent> sharedEvent,
                                  Optional<uint64_t> signalValue)
{
    if (!mSharedEvent)
    {
        ANGLE_TRY(initializeImpl(contextMtl, sharedEvent, signalValue));
    }
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE ||
           condition == EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE);
    ASSERT(flags == 0);

    if (condition == GL_SYNC_GPU_COMMANDS_COMPLETE)
    {
        contextMtl->queueEventSignal(mSharedEvent, mSignalValue.value(),
                                     std::shared_ptr<mtl::Sync>(nullptr));
    }

    return angle::Result::Continue;
}

angle::Result EGLSyncMtl::clientWaitImpl(ContextMtl *contextMtl,
                                         bool flushCommands,
                                         uint64_t timeout,
                                         GLenum *outResult)
{
    std::unique_lock<std::mutex> lg(*mLock);
    if (mSharedEvent.get().signaledValue >= mSignalValue.value())
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

    // Create references to mutex and condition variable since they might be released in
    // onDestroy(), but the callback might still not be fired yet.
    std::shared_ptr<std::condition_variable> cvRef = mCv;
    std::shared_ptr<std::mutex> lockRef            = mLock;
    mtl::AutoObjCObj<MTLSharedEventListener> eventListener =
        contextMtl->getDisplay()->getOrCreateSharedEventListener();
    [mSharedEvent.get() notifyListener:eventListener
                               atValue:mSignalValue.value()
                                 block:^(id<MTLSharedEvent> sharedEvent, uint64_t value) {
                                   std::unique_lock<std::mutex> localLock(*lockRef);
                                   cvRef->notify_one();
                                 }];

    // Passing EGL_FOREVER_KHR overflows std::chrono::nanoseconds.
    const uint64_t kNanosecondsPerDay = 86400000000000;
    if (timeout > kNanosecondsPerDay)
    {
        timeout = kNanosecondsPerDay;
    }

    if (!mCv->wait_for(lg, std::chrono::nanoseconds(timeout),
                       [this] { return mSharedEvent.get().signaledValue >= mSignalValue.value(); }))
    {
        *outResult = GL_TIMEOUT_EXPIRED;
        return angle::Result::Incomplete;
    }

    ASSERT(mSharedEvent.get().signaledValue >= mSignalValue.value());
    *outResult = GL_CONDITION_SATISFIED;

    return angle::Result::Continue;
}

egl::Error EGLSyncMtl::initialize(const egl::Display *display,
                                  const gl::Context *context,
                                  EGLenum type)
{
    ASSERT(context != nullptr);
    mType = type;

    ContextMtl *contextMtl = mtl::GetImpl(context);
    switch (type)
    {
        case EGL_SYNC_FENCE_KHR:
            ASSERT(mSharedEvent == nil);
            ASSERT(!mSignalValue.valid());
            break;
        case EGL_SYNC_METAL_SHARED_EVENT_ANGLE:
            break;
        default:
            UNREACHABLE();
            return egl::Error(EGL_BAD_ALLOC);
    }

    if (IsError(setImpl(contextMtl, mCondition, 0, mSharedEvent, mSignalValue)))
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
    ContextMtl *contextMtl = mtl::GetImpl(context);
    if (IsError(clientWaitImpl(contextMtl, flush, static_cast<uint64_t>(timeout), &result)))
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

    ASSERT(mSignalValue.valid());

    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->serverWaitEvent(mSharedEvent, mSignalValue.value());
    return egl::NoError();
}

egl::Error EGLSyncMtl::getStatus(const egl::Display *display, EGLint *outStatus)
{
    bool signaled = (mSharedEvent.get().signaledValue >= mSignalValue.value());
    *outStatus    = signaled ? EGL_SIGNALED_KHR : EGL_UNSIGNALED_KHR;
    return egl::NoError();
}

egl::Error EGLSyncMtl::copyMetalSharedEventANGLE(const egl::Display *display, void **result) const
{
    switch (mType)
    {
        case EGL_SYNC_METAL_SHARED_EVENT_ANGLE:
        {
            mtl::AutoObjCPtr<id<MTLSharedEvent>> copySharedEvent = mSharedEvent;
            *result = reinterpret_cast<void *>(copySharedEvent.leakObject());
            return egl::NoError();
        }

        default:
            return egl::EglBadDisplay();
    }
}

egl::Error EGLSyncMtl::dupNativeFenceFD(const egl::Display *display, EGLint *result) const
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}

}  // namespace rx
