//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FastMutex.h:
//   Faster alternatives to the std::mutex.
//

#ifndef COMMON_FASTMUTEX_H_
#define COMMON_FASTMUTEX_H_

#include <atomic>
#include <condition_variable>

#include "common/angleutils.h"

namespace angle
{

// Similar to "Spinlock" class, but uses combination of "yield" and "sleep" to save power.
class FastMutex1 final : angle::NonCopyable
{
  public:
    bool try_lock();
    void lock();
    void unlock();

  private:
    void wait();
    template <int kNumSpins>
    bool doSpinWaitWithYield();
    template <int kNumSpins, int kSleepDurationMicro>
    bool doSpinWaitWithSleep();

  private:
    // "int" works faster than "bool" on S906B
    std::atomic_int mState{0};
};

// Similar to "Spinlock" class, but uses waiting on "condition_variable".
// Implementation uses "relaxed" atomic operations to improve performance. Because of that,
// in theory it is possible to miss the "notify" call (never happened while testing on S906B).
// To solve possible "infinite wait issue", waiting is done in "1ms" intervals.
class FastMutex2 final : angle::NonCopyable
{
  public:
    bool try_lock();
    void lock();
    void unlock();

  private:
    void wait();
    void notify();

  private:
    std::atomic_int mState{0};
    std::atomic_int mNumWaiters{0};
    std::mutex mWaitMutex;
    std::condition_variable mWaitCondVar;
};

// Alternative (slower) to "FastMutex2". Uses waiting on "condition_variable" and "strong" atomics.
class FastMutex3 final : angle::NonCopyable
{
  public:
    bool try_lock();
    void lock();
    void unlock();

  private:
    void waitAndLock();
    void notify();

  private:
    std::atomic_int mState{0};
    std::mutex mWaitMutex;
    std::condition_variable mWaitCondVar;
};

// FastMutex1
ANGLE_INLINE bool FastMutex1::try_lock()
{
    // Relaxed check first to prevent unnecessary cache misses.
    return mState.load(std::memory_order_relaxed) == 0 &&
           mState.exchange(1, std::memory_order_acquire) == 0;
}

ANGLE_INLINE void FastMutex1::lock()
{
    while (ANGLE_UNLIKELY(mState.exchange(1, std::memory_order_acquire) != 0))
    {
        wait();
    }
}

ANGLE_INLINE void FastMutex1::unlock()
{
    mState.store(0, std::memory_order_release);
}

// FastMutex2
ANGLE_INLINE bool FastMutex2::try_lock()
{
    return mState.load(std::memory_order_relaxed) == 0 &&
           mState.exchange(1, std::memory_order_acquire) == 0;
}

ANGLE_INLINE void FastMutex2::lock()
{
    while (ANGLE_UNLIKELY(mState.exchange(1, std::memory_order_acquire) != 0))
    {
        wait();
    }
}

ANGLE_INLINE void FastMutex2::unlock()
{
    mState.store(0, std::memory_order_release);
    if (ANGLE_UNLIKELY(mNumWaiters.load(std::memory_order_acquire) > 0))
    {
        notify();
    }
}

// FastMutex3
ANGLE_INLINE bool FastMutex3::try_lock()
{
    if (mState.load(std::memory_order_relaxed) != 0)
    {
        return false;
    }
    const int prevState = mState.exchange(1, std::memory_order_acquire);
    if ((prevState != 0) && (prevState != 2 || mState.exchange(2, std::memory_order_acquire) != 0))
    {
        return false;
    }
    return true;
}

ANGLE_INLINE void FastMutex3::lock()
{
    if (ANGLE_UNLIKELY(mState.exchange(1, std::memory_order_acquire) != 0))
    {
        waitAndLock();
    }
}

ANGLE_INLINE void FastMutex3::unlock()
{
    if (ANGLE_UNLIKELY(mState.exchange(0, std::memory_order_release) == 2))
    {
        notify();
    }
}

}  // namespace angle

#endif  // COMMON_FASTMUTEX_H_
