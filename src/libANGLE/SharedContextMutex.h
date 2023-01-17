//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SharedContextMutex.h: Classes for protecting Shared Context access and EGLImage siblings.

#ifndef LIBANGLE_SHARED_CONTEXT_MUTEX_H_
#define LIBANGLE_SHARED_CONTEXT_MUTEX_H_

#include <atomic>

#include "common/debug.h"

namespace egl
{

class ContextMutex : angle::NonCopyable
{
  public:
    // Prevents destruction while locked
    class [[nodiscard]] AddRefLock final : angle::NonCopyable
    {
      public:
        ANGLE_INLINE AddRefLock() = default;
        ANGLE_INLINE AddRefLock(ContextMutex *mutex) { lock(mutex); }
        ANGLE_INLINE ~AddRefLock() { unlockIfLocked(); }

        void lock(ContextMutex *mutex);
        void unlock();
        void unlockIfLocked();

      private:
        ContextMutex *mMutex = nullptr;
    };

  public:
    // Requires external synchronization
    ANGLE_INLINE void addRef() { ++mRefCount; }
    void release(bool needUnlock = false);

    virtual bool try_lock() = 0;
    virtual void lock()     = 0;
    virtual void unlock()   = 0;

  protected:
    virtual ~ContextMutex();

    virtual void onDestroy(bool needUnlock);

  protected:
    size_t mRefCount = 0;
};

// Mutex may be locked only by a single thread. Other threads may only check the status.
class SingleContextMutex final : public ContextMutex
{
  public:
    ANGLE_INLINE int getState() const { return mState.load(std::memory_order_relaxed); }
    ANGLE_INLINE int acquireState() const { return mState.load(std::memory_order_acquire); }

    // ContextMutex
    bool try_lock() override;
    void lock() override;
    void unlock() override;

  private:
    std::atomic_int mState{0};
};

template <class Mutex>
class SharedContextMutex final : public ContextMutex
{
  public:
    SharedContextMutex();
    ~SharedContextMutex() override;

    // Merges mutexes so they work as one.
    // At the end, only final mutex will be locked.
    // Does nothing if two mutexes are the same or already merged.
    // Note: addRef()/release() for merged mutexes MUST be synchronized as for the single mutex.
    static void Merge(SharedContextMutex *lockedMutex, SharedContextMutex *otherMutex);

    // Returns current "root" mutex.
    // Warning! Result is only stable if mutex is locked, while may change any time if unlocked.
    // May be used to compare against already locked "root" mutex.
    ANGLE_INLINE SharedContextMutex *getRoot() { return mRoot.load(std::memory_order_relaxed); }

    // ContextMutex
    bool try_lock() override;
    void lock() override;
    void unlock() override;

  private:
    SharedContextMutex *doTryLock();
    SharedContextMutex *doLock();
    void doUnlock();

    void setNewRoot(SharedContextMutex *newRoot);
    void addLeaf(SharedContextMutex *leaf);
    void removeLeaf(SharedContextMutex *leaf);
    void onDestroy(bool needUnlock) override;

  private:
    Mutex mMutex;

    std::atomic<SharedContextMutex *> mRoot;
    std::vector<SharedContextMutex *> mOldRoots;  // For keeping references
    std::set<SharedContextMutex *> mLeaves;       // Non empty only if (this == mRoot)
    uint32_t mRank;  // Prevents "mOldRoots" from growing indefinitely...

#if defined(ANGLE_ENABLE_ASSERTS)
    std::atomic<angle::ThreadId> mOwnerThreadId;
#endif
};

class ContextMutexManager
{
  public:
    virtual ~ContextMutexManager() = default;

    virtual ContextMutex *create()                                          = 0;
    virtual void merge(ContextMutex *lockedMutex, ContextMutex *otherMutex) = 0;
    virtual ContextMutex *getRootMutex(ContextMutex *mutex)                 = 0;
};

template <class Mutex>
class SharedContextMutexManager final : public ContextMutexManager
{
  public:
    ContextMutex *create() override;
    void merge(ContextMutex *lockedMutex, ContextMutex *otherMutex) override;
    ContextMutex *getRootMutex(ContextMutex *mutex) override;
};

}  // namespace egl

#endif  // LIBANGLE_SHARED_CONTEXT_MUTEX_H_
