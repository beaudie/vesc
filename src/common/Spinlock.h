//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Spinlock.h:
//   Spinlock is a lock that loops actively until it gets the resource.
//   Only use it when the lock will be granted in reasonably short time.

#ifndef COMMON_SPINLOCK_H_
#define COMMON_SPINLOCK_H_

#include "common/angleutils.h"

namespace angle
{

class Spinlock
{
  public:
    Spinlock() noexcept;

    bool try_lock() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

  private:
    std::atomic_int mLock;
};

ANGLE_INLINE Spinlock::Spinlock() noexcept : mLock(0) {}

ANGLE_INLINE bool Spinlock::try_lock() noexcept
{
    // Relaxed check first to prevent unnecessary cache misses.
    return mLock.load(std::memory_order_relaxed) == 0 &&
           mLock.exchange(1, std::memory_order_acquire) == 0;
}

ANGLE_INLINE void Spinlock::lock() noexcept
{
    while (mLock.exchange(1, std::memory_order_acquire) != 0)
    {
        // Relaxed wait to prevent unnecessary cache misses.
        while (mLock.load(std::memory_order_relaxed) != 0)
        {
            std::this_thread::yield();
        }
    }
}

ANGLE_INLINE void Spinlock::unlock() noexcept
{
    mLock.store(0, std::memory_order_release);
}

}  // namespace angle

#endif  // COMMON_SPINLOCK_H_
