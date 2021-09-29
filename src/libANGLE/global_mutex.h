//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_mutex.h : Defines functions for the global mutex.

#ifndef LIBANGLE_GLOBALMUTEX_H_
#define LIBANGLE_GLOBALMUTEX_H_

#include "libANGLE/Context.h"

#include <mutex>

namespace angle
{
using GlobalMutex = std::recursive_mutex;
}  // namespace angle

namespace egl
{
angle::GlobalMutex &GetGlobalMutex();
}  // namespace egl

namespace gl
{
ANGLE_INLINE std::unique_lock<angle::GlobalMutex> GetContextLock(Context *context)
{
#if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
    auto lock = std::unique_lock<angle::GlobalMutex>(egl::GetGlobalMutex());

    DirtyContextIfNeeded(context);
    return lock;
#else
    return context->isShared() ? std::unique_lock<angle::GlobalMutex>(egl::GetGlobalMutex())
                               : std::unique_lock<angle::GlobalMutex>();
#endif
}
}  // namespace gl

#endif  // LIBANGLE_GLOBALMUTEX_H_
