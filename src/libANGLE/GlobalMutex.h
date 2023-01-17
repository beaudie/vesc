//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalMutex.h: Defines Global Mutex and utilities.

#ifndef LIBANGLE_GLOBAL_MUTEX_H_
#define LIBANGLE_GLOBAL_MUTEX_H_

#include "common/angleutils.h"

namespace egl
{
namespace priv
{
class GlobalMutex;
}  // namespace priv

class [[nodiscard]] ScopedGlobalMutexLock final : angle::NonCopyable
{
  public:
    ScopedGlobalMutexLock();
    ~ScopedGlobalMutexLock();

  private:
    priv::GlobalMutex &mMutex;
};

enum class GlobalMutexUnlockType
{
    IfOwned,
    Always,
};

namespace priv
{
class [[nodiscard]] ScopedGlobalMutexUnlock final : angle::NonCopyable
{
  public:
    explicit ScopedGlobalMutexUnlock(GlobalMutexUnlockType type);
    ~ScopedGlobalMutexUnlock();

  private:
    priv::GlobalMutex &mMutex;
    angle::ThreadId mLockThreadId;
    int mLockLevel;
};

// Dummy implementation to avoid conditional compilation.
class [[nodiscard]] DummyScopedGlobalMutexUnlock final : angle::NonCopyable
{
  public:
    explicit DummyScopedGlobalMutexUnlock(GlobalMutexUnlockType) {}
};
}  // namespace priv

#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_UNLOCK)
using ScopedGlobalMutexUnlock = priv::ScopedGlobalMutexUnlock;
#else
using ScopedGlobalMutexUnlock = priv::DummyScopedGlobalMutexUnlock;
#endif

#if defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
void AllocateGlobalMutex();
void DeallocateGlobalMutex();
#endif

}  // namespace egl

#endif  // LIBANGLE_GLOBAL_MUTEX_H_
