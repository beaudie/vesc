//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalMutex.cpp: Defines Global Mutex and utilities.

#include "libANGLE/GlobalMutex.h"

#include <atomic>

#include "common/debug.h"
#include "common/system_utils.h"

namespace egl
{
namespace priv
{
namespace
{
using GlobalMutexType = std::mutex;

class SimpleGlobalMutex : angle::NonCopyable
{
  public:
    angle::ThreadId getOwnerThreadId() const
    {
        UNREACHABLE();
        return angle::InvalidThreadId();
    }

    ANGLE_INLINE void lock() { mMutex.lock(); }
    ANGLE_INLINE void unlock() { mMutex.unlock(); }

    void tempUnlock(int *lockLevelOut) { UNREACHABLE(); }
    void restoreLock(angle::ThreadId id, int lockLevel) { UNREACHABLE(); }

  protected:
    GlobalMutexType mMutex;
};

class ThreadIdGlobalMutex : public SimpleGlobalMutex
{
  public:
    ANGLE_INLINE angle::ThreadId getOwnerThreadId() const
    {
        return mOwnerThreadId.load(std::memory_order_relaxed);
    }

    ANGLE_INLINE void lock(angle::ThreadId id = angle::GetCurrentThreadId())
    {
        mMutex.lock();
        setLocked(id);
    }

    ANGLE_INLINE void unlock()
    {
        ASSERT(getOwnerThreadId() == angle::GetCurrentThreadId());
        doUnlock();
    }

    ANGLE_INLINE void tempUnlock(int *lockLevelOut) { unlock(); }
    ANGLE_INLINE void restoreLock(angle::ThreadId id, int lockLevel) { lock(id); }

  protected:
    ANGLE_INLINE void setLocked(angle::ThreadId id)
    {
        ASSERT(id == angle::GetCurrentThreadId());
        ASSERT(getOwnerThreadId() == angle::InvalidThreadId());
        mOwnerThreadId.store(id, std::memory_order_relaxed);
    }

    ANGLE_INLINE void doUnlock()
    {
        mOwnerThreadId.store(angle::InvalidThreadId(), std::memory_order_relaxed);
        mMutex.unlock();
    }

    std::atomic<angle::ThreadId> mOwnerThreadId{angle::InvalidThreadId()};
};

class RecursiveGlobalMutex : public ThreadIdGlobalMutex
{
  public:
    ANGLE_INLINE void lock()
    {
        const angle::ThreadId id = angle::GetCurrentThreadId();
        if (ANGLE_UNLIKELY(!mMutex.try_lock()))
        {
            if (ANGLE_UNLIKELY(getOwnerThreadId() == id))
            {
                ASSERT(mLockLevel > 0);
                ++mLockLevel;
                return;
            }
            mMutex.lock();
        }
        setLocked(id, 1);
    }

    ANGLE_INLINE void unlock()
    {
        ASSERT(getOwnerThreadId() == angle::GetCurrentThreadId());
        ASSERT(mLockLevel > 0);
        if (ANGLE_LIKELY(--mLockLevel == 0))
        {
            doUnlock();
        }
    }

    ANGLE_INLINE void tempUnlock(int *lockLevelOut)
    {
        ASSERT(getOwnerThreadId() == angle::GetCurrentThreadId());
        ASSERT(mLockLevel > 0);
        *lockLevelOut = mLockLevel;
        mLockLevel    = 0;
        doUnlock();
    }

    ANGLE_INLINE void restoreLock(angle::ThreadId id, int lockLevel)
    {
        mMutex.lock();
        setLocked(id, lockLevel);
    }

  protected:
    ANGLE_INLINE void setLocked(angle::ThreadId id, int lockLevel)
    {
        ASSERT(lockLevel > 0);
        ASSERT(mLockLevel == 0);
        mLockLevel = lockLevel;
        ThreadIdGlobalMutex::setLocked(id);
    }

    ANGLE_INLINE void doUnlock()
    {
        ASSERT(mLockLevel == 0);
        ThreadIdGlobalMutex::doUnlock();
    }

    int mLockLevel = 0;
};
}  // anonymous namespace

#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
class GlobalMutex final : public RecursiveGlobalMutex
{};
#elif defined(ANGLE_ENABLE_GLOBAL_MUTEX_UNLOCK) || defined(ANGLE_ENABLE_ASSERTS)
class GlobalMutex final : public ThreadIdGlobalMutex
{};
#else
class GlobalMutex final : public SimpleGlobalMutex
{};
#endif
}  // namespace priv

namespace
{
ANGLE_REQUIRE_CONSTANT_INIT std::atomic<priv::GlobalMutex *> g_Mutex(nullptr);
static_assert(std::is_trivially_destructible<decltype(g_Mutex)>::value,
              "global mutex is not trivially destructible");

priv::GlobalMutex *AllocateGlobalMutexImpl()
{
    priv::GlobalMutex *currentMutex = nullptr;
    std::unique_ptr<priv::GlobalMutex> newMutex(new priv::GlobalMutex());
    if (g_Mutex.compare_exchange_strong(currentMutex, newMutex.get(), std::memory_order_release,
                                        std::memory_order_acquire))
    {
        return newMutex.release();
    }
    return currentMutex;
}

priv::GlobalMutex *GetGlobalMutex()
{
    priv::GlobalMutex *mutex = g_Mutex.load(std::memory_order_acquire);
    return mutex != nullptr ? mutex : AllocateGlobalMutexImpl();
}
}  // anonymous namespace

// ScopedGlobalMutexLock implementation.
ScopedGlobalMutexLock::ScopedGlobalMutexLock() : mMutex(*GetGlobalMutex())
{
    mMutex.lock();
}

ScopedGlobalMutexLock::~ScopedGlobalMutexLock()
{
    mMutex.unlock();
}

namespace priv
{
// ScopedGlobalMutexUnlock implementation.
ScopedGlobalMutexUnlock::ScopedGlobalMutexUnlock(GlobalMutexUnlockType type)
    : mMutex(*GetGlobalMutex()), mLockLevel(0)
{
    const angle::ThreadId ownerThreadId = mMutex.getOwnerThreadId();

    if ((type == GlobalMutexUnlockType::Always) || (ownerThreadId == angle::GetCurrentThreadId()))
    {
        mMutex.tempUnlock(&mLockLevel);
        mLockThreadId = ownerThreadId;
    }
    else
    {
        mLockThreadId = angle::InvalidThreadId();
    }
}

ScopedGlobalMutexUnlock::~ScopedGlobalMutexUnlock()
{
    if (mLockThreadId != angle::InvalidThreadId())
    {
        mMutex.restoreLock(mLockThreadId, mLockLevel);
    }
}
}  // namespace priv

// Global functions.
#if defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
void AllocateGlobalMutex()
{
    (void)AllocateGlobalMutexImpl();
}

void DeallocateGlobalMutex()
{
    priv::GlobalMutex *mutex = g_Mutex.exchange(nullptr);
    {
        // Wait for the mutex to become released by other threads before deleting.
        std::lock_guard<priv::GlobalMutex> lock(*mutex);
    }
    delete mutex;
}
#endif

}  // namespace egl
