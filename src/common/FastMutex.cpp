//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FastMutex.cpp:
//   Faster alternatives to the std::mutex.
//

#include "common/FastMutex.h"

namespace angle
{

// FastMutex1
void FastMutex1::wait()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // Results from "dEQP-EGL.functional.sharing.gles2.multithread.*" on S906B
    // 87.2% of all "lock()" calls are successful without this "wait()".
    // 79% of unsuccessful: will wait no more than 100 "yield()" calls.
    // 98% of unsuccessful: will wait no more than 1000 "yield()" calls.
    // 99.83% of unsuccessful: will not reach "1ms sleep".

    while (!doSpinWaitWithYield<1000>() && !doSpinWaitWithSleep<20, 1>() &&
           !doSpinWaitWithSleep<2, 1000>() && !doSpinWaitWithSleep<20, 1>())
    {
    }
#else
    while (!doSpinWaitWithYield<1>())
    {
    }
#endif
}

template <int kNumSpins>
bool FastMutex1::doSpinWaitWithYield()
{
    for (int i = 0; i < kNumSpins; ++i)
    {
        std::this_thread::yield();
        if (mState.load(std::memory_order_relaxed) == 0)
        {
            return true;
        }
    }
    return false;
}

template <int kNumSpins, int kSleepDurationMicro>
bool FastMutex1::doSpinWaitWithSleep()
{
    for (int i = 0; i < kNumSpins; ++i)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(kSleepDurationMicro));
        if (mState.load(std::memory_order_relaxed) == 0)
        {
            return true;
        }
    }
    return false;
}

// FastMutex2
void FastMutex2::wait()
{
    std::unique_lock<std::mutex> lock(mWaitMutex);
    // If mNumWaiters store is reordered after mState load, unlocking thread may skip notify() call
    // if unlocked before mNumWaiters store. But since mState load was already performed wait will
    // occur while mutex is already unlocked.
    mNumWaiters.fetch_add(1, std::memory_order_release);
    while (mState.load(std::memory_order_acquire) != 0)
    {
        // Wait with timeout, because atomics can't guarantee 100% notify...
        mWaitCondVar.wait_for(lock, std::chrono::microseconds(1000));
    }
    mNumWaiters.fetch_sub(1, std::memory_order_relaxed);
}

void FastMutex2::notify()
{
    mWaitMutex.lock();
    mWaitMutex.unlock();
    mWaitCondVar.notify_one();
}

// FastMutex3
void FastMutex3::waitAndLock()
{
    std::unique_lock<std::mutex> lock(mWaitMutex);
    while (mState.exchange(2, std::memory_order_acquire) != 0)
    {
        mWaitCondVar.wait(lock);
    }
}

void FastMutex3::notify()
{
    mWaitMutex.lock();
    mWaitMutex.unlock();
    mWaitCondVar.notify_one();
}

}  // namespace angle
