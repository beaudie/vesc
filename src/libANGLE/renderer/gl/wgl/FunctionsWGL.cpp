//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsWGL.h: Implements the FuntionsWGL class.

#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{

typedef PROC(WINAPI *PFNWGLGETPROCADDRESSPROC)(LPCSTR);

template <typename T>
static void GetWGLProcAddress(HMODULE glModule, const char *name, T* outProcAddress)
{
    PFNWGLGETPROCADDRESSPROC getProcAddressWGL = reinterpret_cast<PFNWGLGETPROCADDRESSPROC>(GetProcAddress(glModule, "wglGetProcAddress"));
    ASSERT(getProcAddressWGL);

    T proc = reinterpret_cast<T>(getProcAddressWGL(name));
    if (proc)
    {
        *outProcAddress = proc;
    }
    else
    {
        *outProcAddress = reinterpret_cast<T>(GetProcAddress(glModule, name));
    }
}

FunctionsWGL::FunctionsWGL()
    : createContext(nullptr),
      deleteContext(nullptr),
      makeCurrent(nullptr),
      createContextAttribsARB(nullptr),
      getPixelFormatAttribivARB(nullptr),
      swapIntervalEXT(nullptr)
{
}

void FunctionsWGL::intialize(HMODULE glModule)
{
    GetWGLProcAddress(glModule, "wglCreateContext", &createContext);
    GetWGLProcAddress(glModule, "wglDeleteContext", &deleteContext);
    GetWGLProcAddress(glModule, "wglMakeCurrent", &makeCurrent);
    GetWGLProcAddress(glModule, "wglCreateContextAttribsARB", &createContextAttribsARB);
    GetWGLProcAddress(glModule, "wglGetPixelFormatAttribivARB", &getPixelFormatAttribivARB);
    GetWGLProcAddress(glModule, "wglSwapIntervalEXT", &swapIntervalEXT);
}

}
