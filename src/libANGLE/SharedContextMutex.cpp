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

// SingleContextMutex
bool SingleContextMutex::try_lock()
{
    UNREACHABLE();
    return false;
}

void SingleContextMutex::lock()
{
    mState.store(1, std::memory_order_relaxed);
}

void SingleContextMutex::unlock()
{
    mState.store(0, std::memory_order_release);
}

// SharedContextMutex
template <class Mutex>
bool SharedContextMutex<Mutex>::try_lock()
{
    SharedContextMutex *const root = getRoot();
    return (root->doTryLock() != nullptr);
}

template <class Mutex>
void SharedContextMutex<Mutex>::lock()
{
    SharedContextMutex *const root = getRoot();
    (void)root->doLock();
}

template <class Mutex>
void SharedContextMutex<Mutex>::unlock()
{
    SharedContextMutex *const root = getRoot();
    // "root" is currently locked so "root->getRoot()" will return stable result.
    ASSERT(root == root->getRoot());
    root->doUnlock();
}

template <class Mutex>
ANGLE_INLINE SharedContextMutex<Mutex> *SharedContextMutex<Mutex>::doTryLock()
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const angle::ThreadId threadId = angle::GetCurrentThreadId();
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) != threadId);
#endif
    if (mMutex.try_lock())
    {
        SharedContextMutex *const root = getRoot();
        if (ANGLE_UNLIKELY(this != root))
        {
            // Unlock, so only the "real root" mutex remains locked
            mMutex.unlock();
            SharedContextMutex *const lockedRoot = root->doTryLock();
            ASSERT(lockedRoot == getRoot());
            return lockedRoot;
        }
#if defined(ANGLE_ENABLE_ASSERTS)
        mOwnerThreadId.store(threadId, std::memory_order_relaxed);
#endif
        return this;
    }
    return nullptr;
}

template <class Mutex>
ANGLE_INLINE SharedContextMutex<Mutex> *SharedContextMutex<Mutex>::doLock()
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const angle::ThreadId threadId = angle::GetCurrentThreadId();
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) != threadId);
#endif
    mMutex.lock();
    SharedContextMutex *const root = getRoot();
    if (ANGLE_UNLIKELY(this != root))
    {
        // Unlock, so only the "real root" mutex remains locked
        mMutex.unlock();
        SharedContextMutex *const lockedRoot = root->doLock();
        ASSERT(lockedRoot == getRoot());
        return lockedRoot;
    }
#if defined(ANGLE_ENABLE_ASSERTS)
    mOwnerThreadId.store(threadId, std::memory_order_relaxed);
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
    : mRoot(this),
      mRank(0)
#if defined(ANGLE_ENABLE_ASSERTS)
      ,
      mOwnerThreadId(angle::InvalidThreadId())
#endif
{}

template <class Mutex>
SharedContextMutex<Mutex>::~SharedContextMutex()
{
    ASSERT(this == getRoot());
    ASSERT(mOldRoots.empty());
    ASSERT(mLeaves.empty());
}

template <class Mutex>
void SharedContextMutex<Mutex>::Merge(SharedContextMutex *lockedMutex,
                                      SharedContextMutex *otherMutex)
{
    ASSERT(lockedMutex != nullptr);
    ASSERT(otherMutex != nullptr);

    SharedContextMutex *lockedRoot      = lockedMutex->getRoot();
    SharedContextMutex *otherLockedRoot = nullptr;

    for (;;)
    {
        SharedContextMutex *otherRoot = otherMutex->getRoot();
        if (otherRoot == lockedRoot)
        {
            // Do nothing if two mutexes are the same/merged
            return;
        }
        otherLockedRoot = otherRoot->doTryLock();
        if (otherLockedRoot != nullptr)
        {
            // This may only happen later in this invocation.
            ASSERT(otherLockedRoot != lockedRoot);
            break;
        }
        // Unlock and retry... May use "doUnlock()" because it is a "real root" mutex
        lockedRoot->doUnlock();
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 91 + 10));
        lockedRoot = lockedRoot->getRoot()->doLock();
    }

    ASSERT(lockedRoot->mRefCount > 0);
    ASSERT(otherLockedRoot->mRefCount > 0);

    // Make "otherLockedRoot" the root of the "merged" mutex
    if (lockedRoot->mRank > otherLockedRoot->mRank)
    {
        // So the "lockedRoot" is lower rank.
        std::swap(lockedRoot, otherLockedRoot);
    }
    else if (lockedRoot->mRank == otherLockedRoot->mRank)
    {
        ++otherLockedRoot->mRank;
    }

    // Update the structure
    for (SharedContextMutex *const leaf : lockedRoot->mLeaves)
    {
        ASSERT(leaf->getRoot() == lockedRoot);
        leaf->setNewRoot(otherLockedRoot);
    }
    lockedRoot->mLeaves.clear();
    lockedRoot->setNewRoot(otherLockedRoot);

    // Leave only the "merged" mutex locked. "lockedRoot" already merged, need to use "doUnlock()"
    lockedRoot->doUnlock();
}

template <class Mutex>
void SharedContextMutex<Mutex>::setNewRoot(SharedContextMutex *newRoot)
{
    SharedContextMutex *const oldRoot = getRoot();

    ASSERT(newRoot != oldRoot);
    mRoot.store(newRoot, std::memory_order_relaxed);
    newRoot->addRef();

    newRoot->addLeaf(this);

    if (oldRoot != this)
    {
        mOldRoots.emplace_back(oldRoot);
    }
}

template <class Mutex>
void SharedContextMutex<Mutex>::addLeaf(SharedContextMutex *leaf)
{
    ASSERT(this == getRoot());
    ASSERT(leaf->getRoot() == this);
    ASSERT(leaf->mLeaves.empty());
    ASSERT(mLeaves.count(leaf) == 0);
    mLeaves.emplace(leaf);
}

template <class Mutex>
void SharedContextMutex<Mutex>::removeLeaf(SharedContextMutex *leaf)
{
    ASSERT(this == getRoot());
    ASSERT(leaf->getRoot() == this);
    ASSERT(leaf->mLeaves.empty());
    ASSERT(mLeaves.count(leaf) == 1);
    mLeaves.erase(leaf);
}

template <class Mutex>
void SharedContextMutex<Mutex>::onDestroy(bool needUnlock)
{
    ASSERT(mRefCount == 0);
    ASSERT(mLeaves.empty());

    SharedContextMutex *const root = getRoot();
    if (this == root)
    {
        ASSERT(mOldRoots.empty());
        if (needUnlock)
        {
            doUnlock();
        }
    }
    else
    {
        for (SharedContextMutex *oldRoot : mOldRoots)
        {
            ASSERT(oldRoot->getRoot() == root);
            ASSERT(oldRoot->mLeaves.empty());
            oldRoot->release();
        }
        mOldRoots.clear();

        root->removeLeaf(this);

        root->release(needUnlock);
        mRoot.store(this, std::memory_order_relaxed);
    }
}

template class SharedContextMutex<std::mutex>;

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
ContextMutex *SharedContextMutexManager<Mutex>::getRootMutex(ContextMutex *mutex)
{
    return static_cast<SharedContextMutex<Mutex> *>(mutex)->getRoot();
}

template class SharedContextMutexManager<std::mutex>;

}  // namespace egl
