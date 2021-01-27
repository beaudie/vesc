//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/vulkan/android/AHBFunctions.h"

#include <dlfcn.h>

#include "common/debug.h"

namespace rx
{

namespace
{

template <class T>
void AssignFn(void *handle, const char *name, T &fn)
{
    fn = reinterpret_cast<T>(dlsym(handle, name));
}

}  // namespace

struct AHBFunctions::FunctionTable
{
    FunctionTable() = default;

    typedef void (*PFAHardwareBuffer_allocate)(const AHardwareBuffer_Desc *desc,
                                               AHardwareBuffer **outBuffer);
    typedef void (*PFAHardwareBuffer_acquire)(AHardwareBuffer *buffer);
    typedef void (*PFAHardwareBuffer_describe)(const AHardwareBuffer *buffer,
                                               AHardwareBuffer_Desc *outDesc);
    typedef int (*PFAHardwareBuffer_lock)(AHardwareBuffer *buffer,
                                          uint64_t usage,
                                          int32_t fence,
                                          const ARect *rect,
                                          void **outVirtualAddress);
    typedef int (*PFAHardwareBuffer_recvHandleFromUnixSocket)(int socketFd,
                                                              AHardwareBuffer **outBuffer);
    typedef void (*PFAHardwareBuffer_release)(AHardwareBuffer *buffer);
    typedef int (*PFAHardwareBuffer_sendHandleToUnixSocket)(const AHardwareBuffer *buffer,
                                                            int socketFd);
    typedef int (*PFAHardwareBuffer_unlock)(AHardwareBuffer *buffer, int32_t *fence);

    PFAHardwareBuffer_allocate mAllocateFn                                 = nullptr;
    PFAHardwareBuffer_acquire mAcquireFn                                   = nullptr;
    PFAHardwareBuffer_describe mDescribeFn                                 = nullptr;
    PFAHardwareBuffer_lock mLockFn                                         = nullptr;
    PFAHardwareBuffer_recvHandleFromUnixSocket mRecvHandleFromUnixSocketFn = nullptr;
    PFAHardwareBuffer_release mReleaseFn                                   = nullptr;
    PFAHardwareBuffer_sendHandleToUnixSocket mSendHandleToUnixSocketFn     = nullptr;
    PFAHardwareBuffer_unlock mUnlockFn                                     = nullptr;
};

AHBFunctions::AHBFunctions()  = default;
AHBFunctions::~AHBFunctions() = default;

bool AHBFunctions::initialize()
{
    ASSERT(!mFunctionTable);

    void *handle   = dlopen(nullptr, RTLD_NOW);
    mFunctionTable = std::make_unique<FunctionTable>();
    AssignFn(handle, "AHardwareBuffer_allocate", mFunctionTable->mAllocateFn);
    AssignFn(handle, "AHardwareBuffer_acquire", mFunctionTable->mAcquireFn);
    AssignFn(handle, "AHardwareBuffer_describe", mFunctionTable->mDescribeFn);
    AssignFn(handle, "AHardwareBuffer_lock", mFunctionTable->mLockFn);
    AssignFn(handle, "AHardwareBuffer_recvHandleFromUnixSocket",
             mFunctionTable->mRecvHandleFromUnixSocketFn);
    AssignFn(handle, "AHardwareBuffer_release", mFunctionTable->mReleaseFn);
    AssignFn(handle, "AHardwareBuffer_sendHandleToUnixSocket",
             mFunctionTable->mSendHandleToUnixSocketFn);
    AssignFn(handle, "AHardwareBuffer_unlock", mFunctionTable->mUnlockFn);

    // clang-format off
    return mFunctionTable->mAllocateFn &&
           mFunctionTable->mAcquireFn &&
           mFunctionTable->mDescribeFn &&
           mFunctionTable->mLockFn &&
           mFunctionTable->mRecvHandleFromUnixSocketFn &&
           mFunctionTable->mReleaseFn &&
           mFunctionTable->mSendHandleToUnixSocketFn &&
           mFunctionTable->mUnlockFn;
    // clang-format on
}

// static
std::unique_ptr<AHBFunctions> AHBFunctions::Create()
{
    auto functions = std::make_unique<AHBFunctions>();
    if (!functions->initialize())
        functions.reset();
    return functions;
}

void AHBFunctions::allocate(const AHardwareBuffer_Desc *desc, AHardwareBuffer **outBuffer) const
{
    mFunctionTable->mAllocateFn(desc, outBuffer);
}

void AHBFunctions::acquire(AHardwareBuffer *buffer) const
{
    mFunctionTable->mAcquireFn(buffer);
}

void AHBFunctions::describe(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) const
{
    mFunctionTable->mDescribeFn(buffer, outDesc);
}

int AHBFunctions::lock(AHardwareBuffer *buffer,
                       uint64_t usage,
                       int32_t fence,
                       const ARect *rect,
                       void **outVirtualAddress) const
{
    return mFunctionTable->mLockFn(buffer, usage, fence, rect, outVirtualAddress);
}

int AHBFunctions::recvHandleFromUnixSocket(int socketFd, AHardwareBuffer **outBuffer) const
{
    return mFunctionTable->mRecvHandleFromUnixSocketFn(socketFd, outBuffer);
}

void AHBFunctions::release(AHardwareBuffer *buffer) const
{
    mFunctionTable->mReleaseFn(buffer);
}

int AHBFunctions::sendHandleToUnixSocket(const AHardwareBuffer *buffer, int socketFd) const
{
    return mFunctionTable->mSendHandleToUnixSocketFn(buffer, socketFd);
}

int AHBFunctions::unlock(AHardwareBuffer *buffer, int32_t *fence) const
{
    return mFunctionTable->mUnlockFn(buffer, fence);
}

}  // namespace rx
