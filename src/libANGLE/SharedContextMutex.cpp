//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SharedContextMutex.cpp: Classes for protecting Shared Context access and EGLImage siblings.

#include "libANGLE/SharedContextMutex.h"

#if defined(ANGLE_ENABLE_ASSERTS)
#    include "common/system_utils.h"
#endif

namespace egl
{

// ContextMutex
ContextMutex::~ContextMutex()
{
    ASSERT(mRefCount == 0);
}

void ContextMutex::onDestroy(bool needUnlock)
{
    if (needUnlock)
    {
        unlock();
    }
}

void ContextMutex::release(bool needUnlock)
{
    ASSERT(mRefCount > 0);
    if (--mRefCount == 0)
    {
        onDestroy(needUnlock);
        delete this;
    }
    else if (needUnlock)
    {
        unlock();
    }
}

void ContextMutex::AddRefLock::lock(ContextMutex *mutex)
{
    ASSERT(mutex != nullptr);
    ASSERT(mMutex == nullptr);
    mMutex = mutex;
    // lock() before addRef() - using mMutex as synchronization
    mMutex->lock();
    // This lock alone must not cause mutex destruction
    ASSERT(mMutex->mRefCount > 0);
    mMutex->addRef();
}

void ContextMutex::AddRefLock::unlock()
{
    ASSERT(mMutex != nullptr);
    mMutex->release(true);
    mMutex = nullptr;
}

void ContextMutex::AddRefLock::unlockIfLocked()
{
    if (mMutex != nullptr)
    {
        unlock();
    }
}

// DummyContextMutex
bool DummyContextMutex::try_lock()
{
    UNREACHABLE();
    return false;
}

void DummyContextMutex::lock()
{
    mState.store(1, std::memory_order_relaxed);
}

void DummyContextMutex::unlock()
{
    mState.store(0, std::memory_order_release);
}

// SharedContextMutex
template <class Mutex>
bool SharedContextMutex<Mutex>::try_lock()
{
    SharedContextMutex *const master = getMaster();
    return (master->doTryLock() != nullptr);
}

template <class Mutex>
void SharedContextMutex<Mutex>::lock()
{
    SharedContextMutex *const master = getMaster();
    (void)master->doLock();
}

template <class Mutex>
void SharedContextMutex<Mutex>::unlock()
{
    SharedContextMutex *const master = getMaster();
    // "master" is currently locked so "master->getMaster()" will return stable result.
    ASSERT(master == master->getMaster());
    master->doUnlock();
}

template <class Mutex>
ANGLE_INLINE SharedContextMutex<Mutex> *SharedContextMutex<Mutex>::doTryLock()
{
    if (mMutex.try_lock())
    {
        SharedContextMutex *const master = getMaster();
        if (ANGLE_UNLIKELY(this != master))
        {
            // Unlock, so only the "real master" mutex remains locked
            mMutex.unlock();
            SharedContextMutex *const lockedMaster = master->doTryLock();
            ASSERT(lockedMaster == getMaster());
            return lockedMaster;
        }
#if defined(ANGLE_ENABLE_ASSERTS)
        mOwnerThreadId.store(angle::GetCurrentThreadId(), std::memory_order_relaxed);
#endif
        return this;
    }
    return nullptr;
}

template <class Mutex>
ANGLE_INLINE SharedContextMutex<Mutex> *SharedContextMutex<Mutex>::doLock()
{
    mMutex.lock();
    SharedContextMutex *const master = getMaster();
    if (ANGLE_UNLIKELY(this != master))
    {
        // Unlock, so only the "real master" mutex remains locked
        mMutex.unlock();
        SharedContextMutex *const lockedMaster = master->doLock();
        ASSERT(lockedMaster == getMaster());
        return lockedMaster;
    }
#if defined(ANGLE_ENABLE_ASSERTS)
    mOwnerThreadId.store(angle::GetCurrentThreadId(), std::memory_order_relaxed);
#endif
    return this;
}

template <class Mutex>
ANGLE_INLINE void SharedContextMutex<Mutex>::doUnlock()
{
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) == angle::GetCurrentThreadId());
    mOwnerThreadId.store(angle::InvalidThreadId(), std::memory_order_relaxed);
#endif
    mMutex.unlock();
}

template <class Mutex>
SharedContextMutex<Mutex>::SharedContextMutex()
    : mMaster(this),
      mRank(0)
#if defined(ANGLE_ENABLE_ASSERTS)
      ,
      mOwnerThreadId(angle::InvalidThreadId())
#endif
{}

template <class Mutex>
SharedContextMutex<Mutex>::~SharedContextMutex()
{
    ASSERT(this == getMaster());
    ASSERT(mOldMasters.empty());
    ASSERT(mSlaves.empty());
}

template <class Mutex>
void SharedContextMutex<Mutex>::Merge(SharedContextMutex *lockedMutex,
                                      SharedContextMutex *otherMutex)
{
    ASSERT(lockedMutex != nullptr);
    ASSERT(otherMutex != nullptr);

    SharedContextMutex *lockedMaster      = lockedMutex->getMaster();
    SharedContextMutex *otherLockedMaster = nullptr;

    for (;;)
    {
        SharedContextMutex *otherMaster = otherMutex->getMaster();
        if (otherMaster == lockedMaster)
        {
            // Do nothing if two mutexes are the same/merged
            return;
        }
        otherLockedMaster = otherMaster->doTryLock();
        if (otherLockedMaster != nullptr)
        {
            // This may only happen later in this invocation.
            ASSERT(otherLockedMaster != lockedMaster);
            break;
        }
        // Unlock and retry... May use "doUnlock()" because it is a "real master" mutex
        lockedMaster->doUnlock();
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 91 + 10));
        lockedMaster = lockedMaster->getMaster()->doLock();
    }

    ASSERT(lockedMaster->mRefCount > 0);
    ASSERT(otherLockedMaster->mRefCount > 0);

    // Make "otherLockedMaster" the master of the "merged" mutex
    if (lockedMaster->mRank > otherLockedMaster->mRank)
    {
        // So the "lockedMaster" is lower rank.
        std::swap(lockedMaster, otherLockedMaster);
    }
    else if (lockedMaster->mRank == otherLockedMaster->mRank)
    {
        ++otherLockedMaster->mRank;
    }

    // Update the structure
    for (SharedContextMutex *const slave : lockedMaster->mSlaves)
    {
        ASSERT(slave->getMaster() == lockedMaster);
        slave->setNewMaster(otherLockedMaster);
    }
    lockedMaster->mSlaves.clear();
    lockedMaster->setNewMaster(otherLockedMaster);

    // Leave only the "merged" mutex locked. "lockedMaster" already merged, need to use "doUnlock()"
    lockedMaster->doUnlock();
}

template <class Mutex>
void SharedContextMutex<Mutex>::setNewMaster(SharedContextMutex *newMaster)
{
    SharedContextMutex *const oldMaster = getMaster();

    ASSERT(newMaster != oldMaster);
    mMaster.store(newMaster, std::memory_order_relaxed);
    newMaster->addRef();

    newMaster->addSlave(this);

    if (oldMaster != this)
    {
        mOldMasters.emplace_back(oldMaster);
    }
}

template <class Mutex>
void SharedContextMutex<Mutex>::addSlave(SharedContextMutex *slave)
{
    ASSERT(this == getMaster());
    ASSERT(slave->getMaster() == this);
    ASSERT(slave->mSlaves.empty());
    ASSERT(mSlaves.count(slave) == 0);
    mSlaves.emplace(slave);
}

template <class Mutex>
void SharedContextMutex<Mutex>::removeSlave(SharedContextMutex *slave)
{
    ASSERT(this == getMaster());
    ASSERT(slave->getMaster() == this);
    ASSERT(slave->mSlaves.empty());
    ASSERT(mSlaves.count(slave) == 1);
    mSlaves.erase(slave);
}

template <class Mutex>
void SharedContextMutex<Mutex>::onDestroy(bool needUnlock)
{
    ASSERT(mRefCount == 0);
    ASSERT(mSlaves.empty());

    SharedContextMutex *const master = getMaster();
    if (this == master)
    {
        ASSERT(mOldMasters.empty());
        if (needUnlock)
        {
            doUnlock();
        }
    }
    else
    {
        for (SharedContextMutex *oldMaster : mOldMasters)
        {
            ASSERT(oldMaster->getMaster() == master);
            ASSERT(oldMaster->mSlaves.empty());
            oldMaster->release();
        }
        mOldMasters.clear();

        master->removeSlave(this);

        master->release(needUnlock);
        mMaster.store(this, std::memory_order_relaxed);
    }
}

template class SharedContextMutex<std::mutex>;
template class SharedContextMutex<angle::FastMutex1>;
template class SharedContextMutex<angle::FastMutex2>;
template class SharedContextMutex<angle::FastMutex3>;

// SharedContextMutexManager
template <class Mutex>
ContextMutex *SharedContextMutexManager<Mutex>::create()
{
    return new SharedContextMutex<Mutex>();
}

template <class Mutex>
void SharedContextMutexManager<Mutex>::merge(ContextMutex *lockedMutex, ContextMutex *otherMutex)
{
    SharedContextMutex<Mutex>::Merge(static_cast<SharedContextMutex<Mutex> *>(lockedMutex),
                                     static_cast<SharedContextMutex<Mutex> *>(otherMutex));
}

template <class Mutex>
ContextMutex *SharedContextMutexManager<Mutex>::getMasterMutex(ContextMutex *mutex)
{
    return static_cast<SharedContextMutex<Mutex> *>(mutex)->getMaster();
}

template class SharedContextMutexManager<std::mutex>;
template class SharedContextMutexManager<angle::FastMutex1>;
template class SharedContextMutexManager<angle::FastMutex2>;
template class SharedContextMutexManager<angle::FastMutex3>;

}  // namespace egl
