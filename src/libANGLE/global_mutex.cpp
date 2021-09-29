//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_mutex.cpp : Implements functions for the global mutex.

#include "libANGLE/global_mutex.h"

#include "common/angleutils.h"

namespace egl
{
namespace
{
ANGLE_REQUIRE_CONSTANT_INIT std::atomic<angle::GlobalMutex *> g_Mutex(nullptr);
static_assert(std::is_trivially_destructible<decltype(g_Mutex)>::value,
              "global mutex is not trivially destructible");

void AllocateMutex()
{
    if (g_Mutex == nullptr)
    {
        std::unique_ptr<angle::GlobalMutex> newMutex(new angle::GlobalMutex());
        angle::GlobalMutex *expected = nullptr;
        if (g_Mutex.compare_exchange_strong(expected, newMutex.get()))
        {
            newMutex.release();
        }
    }
}
}  // anonymous namespace

angle::GlobalMutex &GetGlobalMutex()
{
    AllocateMutex();
    return *g_Mutex;
}
}  // namespace egl
