//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGLDL.cpp: Implements the FunctionsEGLDL class.

#include "libANGLE/renderer/gl/egl/FunctionsEGLDL.h"

#include <dlfcn.h>

namespace rx
{
namespace
{
// Due to a bug in Mesa (or maybe libdl) it's not possible to close and re-open libEGL.so
// an arbitrary number of times.  End2end tests would die after a couple hundred tests.
// So we use a global object and leak it,
// since we want to close the library only when the program exits anyway.
// TODO(fjhenigman) File a bug and put a link here.
void *nativeEGLHandle;
}  // anonymous namespace

FunctionsEGLDL::FunctionsEGLDL() : mGetProcAddressPtr(nullptr)
{
}

FunctionsEGLDL::~FunctionsEGLDL()
{
}

egl::Error FunctionsEGLDL::initialize(EGLNativeDisplayType nativeDisplay, const char *libName)
{
    if (!nativeEGLHandle)
    {
        nativeEGLHandle = dlopen(libName, RTLD_NOW);
        if (!nativeEGLHandle)
        {
            return egl::EglNotInitialized() << "Could not dlopen native EGL: " << dlerror();
        }
    }

    mGetProcAddressPtr =
        reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(dlsym(nativeEGLHandle, "eglGetProcAddress"));
    if (!mGetProcAddressPtr)
    {
        return egl::EglNotInitialized() << "Could not find eglGetProcAddress";
    }

    return FunctionsEGL::initialize(nativeDisplay);
}

void *FunctionsEGLDL::getProcAddress(const char *name) const
{
    void *f = reinterpret_cast<void *>(mGetProcAddressPtr(name));
    if (f)
    {
        return f;
    }
    return dlsym(nativeEGLHandle, name);
}

}  // namespace rx
