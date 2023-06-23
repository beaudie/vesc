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
    // Results from "dEQP-EGL.functional.sharing.gles2.multithread.*" on S906B
    // 87.2% of all "lock()" calls are successful without this "wait()".
    // 79% of unsuccessful: will wait no more than 100 "yield()" calls.
    // 98% of unsuccessful: will wait no more than 1000 "yield()" calls.
    // 99.83% of unsuccessful: will not reach "1ms sleep".

    using micro = std::chrono::microseconds;
    while (!doSpinWait(2000, []() { std::this_thread::yield(); }) &&
           !doSpinWait(2, []() { std::this_thread::sleep_for(micro(1000)); }))
    {
    }
}

template <class DoPred>
bool FastMutex1::doSpinWait(int numSpins, DoPred &&doPred)
{
    for (int i = 0; i < numSpins; ++i)
    {
        doPred();
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
