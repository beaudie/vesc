//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncVk.cpp:
//    Implements the class methods for SyncVk.
//

#include "libANGLE/renderer/vulkan/SyncVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#if !defined(ANGLE_PLATFORM_WINDOWS)
#    include <unistd.h>
#else
#    include <io.h>
#endif

namespace rx
{
namespace vk
{

SyncHelperFence::SyncHelperFence()
{
    mUse.init();
}

SyncHelperFence::~SyncHelperFence()
{
    mUse.release();
}

void SyncHelperFence::releaseToRenderer(RendererVk *renderer)
{
    renderer->collectGarbageAndReinit(&mUse, &mEvent);
    mFence.reset(renderer->getDevice());
}

angle::Result SyncHelperFence::initialize(ContextVk *contextVk, int inFd)
{
    ANGLE_UNUSED_VARIABLE(inFd);
    ASSERT(!mEvent.valid());

    RendererVk *renderer = contextVk->getRenderer();
    VkDevice device      = renderer->getDevice();

    VkEventCreateInfo eventCreateInfo = {};
    eventCreateInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    eventCreateInfo.flags             = 0;

    DeviceScoped<Event> event(device);
    ANGLE_VK_TRY(contextVk, event.get().init(device, eventCreateInfo));
    // RendererVk::newSharedFence will cause FD to be generated with VkFence - when using Android
    // Native Fences.
    ANGLE_TRY(contextVk->getNextSubmitFence(&mFence));

    mEvent = event.release();

    vk::PrimaryCommandBuffer *primary;
    ANGLE_TRY(contextVk->flushAndGetPrimaryCommandBuffer(&primary));
    primary->setEvent(mEvent.getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    contextVk->getResourceUseList().add(mUse);

    return angle::Result::Continue;
}

angle::Result SyncHelperFence::clientWait(Context *context,
                                          ContextVk *contextVk,
                                          bool flushCommands,
                                          uint64_t timeout,
                                          VkResult *outResult)
{
    RendererVk *renderer = context->getRenderer();

    // If the event is already set, don't wait
    bool alreadySignaled = false;
    ANGLE_TRY(getStatus(context, &alreadySignaled));
    if (alreadySignaled)
    {
        *outResult = VK_EVENT_SET;
        return angle::Result::Continue;
    }

    // If timeout is zero, there's no need to wait, so return timeout already.
    if (timeout == 0)
    {
        *outResult = VK_TIMEOUT;
        return angle::Result::Continue;
    }

    if (flushCommands && contextVk)
    {
        ANGLE_TRY(contextVk->flushImpl(nullptr));
    }

    // Wait on the fence that's expected to be signaled on the first vkQueueSubmit after
    // `initialize` was called. The first fence is the fence created to signal this sync.
    ASSERT(mFence.get().valid());
    VkResult status = mFence.get().wait(renderer->getDevice(), timeout);

    // Check for errors, but don't consider timeout as such.
    if (status != VK_TIMEOUT)
    {
        ANGLE_VK_TRY(context, status);
    }

    *outResult = status;
    return angle::Result::Continue;
}

angle::Result SyncHelperFence::serverWait(ContextVk *contextVk)
{
    vk::PrimaryCommandBuffer *primary;
    ANGLE_TRY(contextVk->flushAndGetPrimaryCommandBuffer(&primary));
    primary->waitEvents(1, mEvent.ptr(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    contextVk->getResourceUseList().add(mUse);
    return angle::Result::Continue;
}

angle::Result SyncHelperFence::getStatus(Context *context, bool *signaled) const
{
    VkResult result = mEvent.getStatus(context->getDevice());
    if (result != VK_EVENT_SET && result != VK_EVENT_RESET)
    {
        ANGLE_VK_TRY(context, result);
    }
    *signaled = (result == VK_EVENT_SET);
    return angle::Result::Continue;
}

angle::Result SyncHelperFence::dupNativeFenceFD(Context *context, int *fdOut) const
{
    if (mFence.get().valid())
    {
        VkFenceGetFdInfoKHR fenceGetFdInfo = {};
        fenceGetFdInfo.sType               = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR;
        fenceGetFdInfo.pNext               = nullptr;
        fenceGetFdInfo.fence               = mFence.get().getHandle();
        fenceGetFdInfo.handleType          = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
        ANGLE_VK_TRY(context, mFence.get().getFd(context->getDevice(), fenceGetFdInfo, fdOut));
        return angle::Result::Continue;
    }
    return angle::Result::Stop;
}

SyncHelperSemaphore::SyncHelperSemaphore() : mSubmitted(false), mInFd(0), mContextVk(nullptr)
{
    mUse.init();
}

SyncHelperSemaphore::~SyncHelperSemaphore()
{
    mUse.release();
}

void SyncHelperSemaphore::releaseToRenderer(RendererVk *renderer)
{
    renderer->collectGarbageAndReinit(&mUse, &mEvent);
    if (mSubmitted)
    {
        mContextVk->addGarbage(&mSemaphore);
    }
    else
    {
        close(mInFd);  // To prevent FD leaks
        mSemaphore.destroy(renderer->getDevice());
    }
}

// inFd = -1 : generate a FD (invalid use case). inFd = 0 : do nothing. inFd > 0 : import FD.
angle::Result SyncHelperSemaphore::initialize(ContextVk *contextVk, int inFd)
{
    ASSERT(!mEvent.valid());
    mContextVk = contextVk;
    mInFd      = inFd;

    RendererVk *renderer = contextVk->getRenderer();
    VkDevice device      = renderer->getDevice();

    VkEventCreateInfo eventCreateInfo = {};
    eventCreateInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    eventCreateInfo.flags             = 0;

    DeviceScoped<Event> event(device);
    ANGLE_VK_TRY(contextVk, event.get().init(device, eventCreateInfo));
    mEvent = event.release();

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext                 = nullptr;
    semaphoreCreateInfo.flags                 = 0;

    // Initialize/create a VkSemaphore handle
    ANGLE_VK_TRY(contextVk, mSemaphore.init(device, semaphoreCreateInfo));

    // If valid FD provided by application - import it to semaphore.
    // This is needed to make this a waitSemaphore - blocking the vkQueueSubmit until FD signaled.
    if (inFd > 0)
    {
        VkImportSemaphoreFdInfoKHR importSemaphoreFdInfo = {};
        importSemaphoreFdInfo.sType      = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
        importSemaphoreFdInfo.pNext      = nullptr;
        importSemaphoreFdInfo.semaphore  = mSemaphore.getHandle();
        importSemaphoreFdInfo.flags      = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR;
        importSemaphoreFdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
        importSemaphoreFdInfo.fd         = inFd;
        ANGLE_VK_TRY(contextVk, mSemaphore.importFd(device, importSemaphoreFdInfo));
    }

    vk::PrimaryCommandBuffer *primary;
    ANGLE_TRY(contextVk->flushAndGetPrimaryCommandBuffer(&primary));
    primary->setEvent(mEvent.getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    contextVk->getResourceUseList().add(mUse);

    return angle::Result::Continue;
}

angle::Result SyncHelperSemaphore::clientWait(Context *context,
                                              ContextVk *contextVk,
                                              bool flushCommands,
                                              uint64_t timeout,
                                              VkResult *outResult)
{
    // Not used
    ANGLE_UNUSED_VARIABLE(context);
    ANGLE_UNUSED_VARIABLE(contextVk);
    ANGLE_UNUSED_VARIABLE(flushCommands);
    ANGLE_UNUSED_VARIABLE(timeout);
    ANGLE_UNUSED_VARIABLE(outResult);

    return angle::Result::Stop;
}

angle::Result SyncHelperSemaphore::serverWait(ContextVk *contextVk)
{
    if (mSemaphore.valid())
    {
        contextVk->insertWaitSemaphore(&mSemaphore);
        mSubmitted = true;
        return angle::Result::Continue;
    }
    return angle::Result::Stop;
}

angle::Result SyncHelperSemaphore::getStatus(Context *context, bool *signaled) const
{
    VkResult result = mEvent.getStatus(context->getDevice());
    if (result != VK_EVENT_SET && result != VK_EVENT_RESET)
    {
        ANGLE_VK_TRY(context, result);
    }
    *signaled = (result == VK_EVENT_SET);
    return angle::Result::Continue;
}

angle::Result SyncHelperSemaphore::dupNativeFenceFD(Context *context, int *fdOut) const
{
    if (mSemaphore.valid())
    {
        VkSemaphoreGetFdInfoKHR semaphoreGetFdInfo = {};
        semaphoreGetFdInfo.sType                   = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
        semaphoreGetFdInfo.pNext                   = nullptr;
        semaphoreGetFdInfo.semaphore               = mSemaphore.getHandle();
        semaphoreGetFdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
        ANGLE_VK_TRY(context, mSemaphore.getFd(context->getDevice(), semaphoreGetFdInfo, fdOut));
        return angle::Result::Continue;
    }
    return angle::Result::Stop;
}

}  // namespace vk

SyncVk::SyncVk() : SyncImpl() {}

SyncVk::~SyncVk() {}

void SyncVk::onDestroy(const gl::Context *context)
{
    mSyncHelper.releaseToRenderer(vk::GetImpl(context)->getRenderer());
}

angle::Result SyncVk::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE);
    ASSERT(flags == 0);

    return mSyncHelper.initialize(vk::GetImpl(context), 0);
}

angle::Result SyncVk::clientWait(const gl::Context *context,
                                 GLbitfield flags,
                                 GLuint64 timeout,
                                 GLenum *outResult)
{
    ContextVk *contextVk = vk::GetImpl(context);

    ASSERT((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) == 0);

    bool flush = (flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0;
    VkResult result;

    ANGLE_TRY(mSyncHelper.clientWait(contextVk, contextVk, flush, static_cast<uint64_t>(timeout),
                                     &result));

    switch (result)
    {
        case VK_EVENT_SET:
            *outResult = GL_ALREADY_SIGNALED;
            return angle::Result::Continue;

        case VK_SUCCESS:
            *outResult = GL_CONDITION_SATISFIED;
            return angle::Result::Continue;

        case VK_TIMEOUT:
            *outResult = GL_TIMEOUT_EXPIRED;
            return angle::Result::Incomplete;

        default:
            UNREACHABLE();
            *outResult = GL_WAIT_FAILED;
            return angle::Result::Stop;
    }
}

angle::Result SyncVk::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    ASSERT(flags == 0);
    ASSERT(timeout == GL_TIMEOUT_IGNORED);

    ContextVk *contextVk = vk::GetImpl(context);
    return mSyncHelper.serverWait(contextVk);
}

angle::Result SyncVk::getStatus(const gl::Context *context, GLint *outResult)
{
    bool signaled = false;
    ANGLE_TRY(mSyncHelper.getStatus(vk::GetImpl(context), &signaled));

    *outResult = signaled ? GL_SIGNALED : GL_UNSIGNALED;
    return angle::Result::Continue;
}

EGLSyncVk::EGLSyncVk(const egl::AttributeMap &attribs) : EGLSyncImpl(), mAttribs(attribs) {}

EGLSyncVk::~EGLSyncVk()
{
    delete mSyncHelper;
    mSyncHelper = nullptr;
}

void EGLSyncVk::onDestroy(const egl::Display *display)
{
    mSyncHelper->releaseToRenderer(vk::GetImpl(display)->getRenderer());
}

egl::Error EGLSyncVk::initialize(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLenum type)
{
    ASSERT(context != nullptr);

    switch (type)
    {
        case EGL_SYNC_FENCE_KHR:
            mSyncHelper = new vk::SyncHelperFence();
            ASSERT(mSyncHelper);
            return angle::ToEGL(mSyncHelper->initialize(vk::GetImpl(context), 0),
                                vk::GetImpl(display), EGL_BAD_ALLOC);
            break;
        case EGL_SYNC_NATIVE_FENCE_ANDROID:
        {
            EGLint externalFd = static_cast<EGLint>(mAttribs.getAsInt(
                EGL_SYNC_NATIVE_FENCE_FD_ANDROID, EGL_NO_NATIVE_FENCE_FD_ANDROID));
            if (externalFd == EGL_NO_NATIVE_FENCE_FD_ANDROID)
            {  // Generate FD - used with eglClientWait and dupNativeFence
                mSyncHelper = new vk::SyncHelperFence();
                ASSERT(mSyncHelper);
                return angle::ToEGL(
                    mSyncHelper->initialize(vk::GetImpl(context), EGL_NO_NATIVE_FENCE_FD_ANDROID),
                    vk::GetImpl(display), EGL_BAD_ALLOC);
            }
            else
            {  // Valid FD is used with eglWaitSync/server - import to VkSemaphore
                mSyncHelper = new vk::SyncHelperSemaphore();
                ASSERT(mSyncHelper);
                return angle::ToEGL(mSyncHelper->initialize(vk::GetImpl(context), externalFd),
                                    vk::GetImpl(display), EGL_BAD_ALLOC);
            }
        }
        break;
        default:
            UNREACHABLE();
            return egl::Error(EGL_BAD_ATTRIBUTE);
    }
}

egl::Error EGLSyncVk::clientWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags,
                                 EGLTime timeout,
                                 EGLint *outResult)
{
    ASSERT((flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) == 0);

    bool flush = (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) != 0;
    VkResult result;

    ContextVk *contextVk = context ? vk::GetImpl(context) : nullptr;
    if (mSyncHelper->clientWait(vk::GetImpl(display), contextVk, flush,
                                static_cast<uint64_t>(timeout), &result) == angle::Result::Stop)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    switch (result)
    {
        case VK_EVENT_SET:
            // fall through.  EGL doesn't differentiate between event being already set, or set
            // before timeout.
        case VK_SUCCESS:
            *outResult = EGL_CONDITION_SATISFIED_KHR;
            return egl::NoError();

        case VK_TIMEOUT:
            *outResult = EGL_TIMEOUT_EXPIRED_KHR;
            return egl::NoError();

        default:
            UNREACHABLE();
            *outResult = EGL_FALSE;
            return egl::Error(EGL_BAD_ALLOC);
    }
}

egl::Error EGLSyncVk::serverWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags)
{
    // Server wait requires a valid bound context.
    ASSERT(context);

    // No flags are currently implemented.
    ASSERT(flags == 0);

    DisplayVk *displayVk = vk::GetImpl(display);
    ContextVk *contextVk = vk::GetImpl(context);

    return angle::ToEGL(mSyncHelper->serverWait(contextVk), displayVk, EGL_BAD_ALLOC);
}

egl::Error EGLSyncVk::getStatus(const egl::Display *display, EGLint *outStatus)
{
    bool signaled = false;
    if (mSyncHelper->getStatus(vk::GetImpl(display), &signaled) == angle::Result::Stop)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    *outStatus = signaled ? EGL_SIGNALED_KHR : EGL_UNSIGNALED_KHR;
    return egl::NoError();
}

egl::Error EGLSyncVk::dupNativeFenceFD(const egl::Display *display, EGLint *fdOut) const
{
    return angle::ToEGL(mSyncHelper->dupNativeFenceFD(vk::GetImpl(display), fdOut),
                        vk::GetImpl(display), EGL_BAD_PARAMETER);
}

}  // namespace rx
