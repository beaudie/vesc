//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.cpp : Implements functions for querying the thread-local GL and EGL state.

#include "libGLESv2/global_state.h"

#include "common/debug.h"
#include "common/platform.h"
#include "common/tls.h"

#include "libANGLE/Thread.h"

namespace gl
{

Context *GetGlobalContext()
{
    egl::Thread *thread = egl::GetCurrentThread();
    return thread->getContext();
}

Context *GetValidGlobalContext()
{
    egl::Thread *thread = egl::GetCurrentThread();
    return thread->getValidContext();
}

}  // namespace gl

namespace egl
{

namespace
{

Thread* currentThread = nullptr;

Thread *AllocateCurrentThread()
{
    if (currentThread == nullptr) {
        currentThread = new Thread();
    }
    return currentThread;
}

}  // anonymous namespace

Thread *GetCurrentThread()
{
    if (currentThread == nullptr) {
        currentThread = new Thread();
    }
    return currentThread;
}

}  // namespace egl

#ifdef ANGLE_PLATFORM_WINDOWS
namespace egl
{

namespace
{

bool DeallocateCurrentThread()
{
    SafeDelete(currentThread);
    return true;
}

bool InitializeProcess()
{
    return true;
}

bool TerminateProcess()
{
    SafeDelete(currentThread)
    return true;
}

}  // anonymous namespace

}  // namespace egl

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            return static_cast<BOOL>(egl::InitializeProcess());

        case DLL_THREAD_ATTACH:
            return static_cast<BOOL>(egl::AllocateCurrentThread() != nullptr);

        case DLL_THREAD_DETACH:
            return static_cast<BOOL>(egl::DeallocateCurrentThread());

        case DLL_PROCESS_DETACH:
            return static_cast<BOOL>(egl::TerminateProcess());
    }

    return TRUE;
}
#endif  // ANGLE_PLATFORM_WINDOWS
