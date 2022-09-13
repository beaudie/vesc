//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/vulkan/android/AHBFunctions.h"

#include <dlfcn.h>

namespace rx
{

namespace
{

template <class T>
void AssignFn(void *handle, const char *name, T &fn)
{
    fn = reinterpret_cast<T>(dlsym(handle, name));
}

constexpr char kNativeWindowLibraryName[] = "libnativewindow.so";
constexpr char kAhbAcquireFunctionName[]  = "AHardwareBuffer_acquire";
constexpr char kAhbDescribeFunctionName[] = "AHardwareBuffer_describe";
constexpr char kAhbReleaseFunctionName[]  = "AHardwareBuffer_release";

const char *GetAndroidSystemPath()
{
#if defined(__LP64__)
    return "/system/lib64/";
#else
    return "/system/lib/";
#endif
}

}  // namespace

AHBFunctions::AHBFunctions()
{
    void *handle = dlopen(nullptr, RTLD_NOW);
    GetAhbProcAddresses(handle);

    // Some services load "libnativewindow.so" with RTLD_LOCAL flag resulting in AHB function
    // symbols being unresolvable through dlsym. Account for such cases and explicitly dlopen the
    // library.
    if (!valid())
    {
        mLibNativeWindowHandle = nullptr;
        std::string fullPath   = GetAndroidSystemPath() + kNativeWindowLibraryName;
        mLibNativeWindowHandle = dlopen(fullPath.c_str(), RTLD_NOW);
        GetAhbProcAddresses(mLibNativeWindowHandle);
    }
}

AHBFunctions::~AHBFunctions()
{
    if (mLibNativeWindowHandle)
    {
        dlclose(mLibNativeWindowHandle);
    }
}

void AHBFunctions::GetAhbProcAddresses(void *handle)
{
    AssignFn(handle, kAhbAcquireFunctionName, mAcquireFn);
    AssignFn(handle, kAhbDescribeFunctionName, mDescribeFn);
    AssignFn(handle, kAhbReleaseFunctionName, mReleaseFn);
}

}  // namespace rx
