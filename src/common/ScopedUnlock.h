//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ScopedUnlock.h:
//    Helper class to temporary unlock a basic lockable object.
//

#ifndef COMMON_SCOPEDUNLOCK_H_
#define COMMON_SCOPEDUNLOCK_H_

#include "common/debug.h"

namespace angle
{
// Helper class to temporary unlock a basic lockable object. Supports deferred unlocking.
template <class BasicLockableT>
class [[nodiscard]] ScopedUnlock final : angle::NonCopyable
{
  public:
    ScopedUnlock() = default;
    explicit ScopedUnlock(BasicLockableT *lockable) { unlock(lockable); }
    ~ScopedUnlock() { lockIfUnlocked(); }

    void unlock(BasicLockableT *lockable)
    {
        ASSERT(lockable != nullptr);
        ASSERT(mLockable == nullptr);
        mLockable = lockable;
        mLockable->unlock();
    }

    void lock()
    {
        ASSERT(isUnlocked());
        mLockable->lock();
        mLockable = nullptr;
    }

    bool isUnlocked() const { return mLockable != nullptr; }

    void lockIfUnlocked()
    {
        if (isUnlocked())
        {
            lock();
        }
    }

  private:
    BasicLockableT *mLockable = nullptr;
};
}  // namespace angle

#endif  // COMMON_SCOPEDUNLOCK_H_
