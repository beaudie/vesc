//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "libEGL/main.h"

#include "common/debug.h"

static DWORD currentTLS = TLS_OUT_OF_INDEXES;

namespace egl
{

Current *allocateCurrent()
{
    Current *current = (egl::Current*)LocalAlloc(LPTR, sizeof(egl::Current));

    if (!current)
    {
        ERR("Could not allocate thread local storage.");
        return NULL;
    }

    ASSERT(currentTLS != TLS_OUT_OF_INDEXES);
    TlsSetValue(currentTLS, current);

    current->error = EGL_SUCCESS;
    current->API = EGL_OPENGL_ES_API;
    current->display = EGL_NO_DISPLAY;
    current->drawSurface = EGL_NO_SURFACE;
    current->readSurface = EGL_NO_SURFACE;

    return current;
}

void deallocateCurrent()
{
    void *current = TlsGetValue(currentTLS);

    if (current)
    {
        LocalFree((HLOCAL)current);
    }
}

}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        {
#if !defined(ANGLE_DISABLE_TRACE)
            FILE *debug = fopen(TRACE_OUTPUT_FILE, "rt");

            if (debug)
            {
                fclose(debug);
                debug = fopen(TRACE_OUTPUT_FILE, "wt");   // Erase
                
                if (debug)
                {
                    fclose(debug);
                }
            }
#endif

            currentTLS = TlsAlloc();

            if (currentTLS == TLS_OUT_OF_INDEXES)
            {
                return FALSE;
            }
        }
        // Fall throught to initialize index
      case DLL_THREAD_ATTACH:
        {
            egl::allocateCurrent();
        }
        break;
      case DLL_THREAD_DETACH:
        {
            egl::deallocateCurrent();
        }
        break;
      case DLL_PROCESS_DETACH:
        {
            egl::deallocateCurrent();
            TlsFree(currentTLS);
        }
        break;
      default:
        break;
    }

    return TRUE;
}

namespace egl
{

Current *getCurrentData()
{
    Current *current = (Current*)TlsGetValue(currentTLS);
    
    // ANGLE issue 488: when the dll is loaded after thread initialization,
    // thread local storage (current) might not exist yet.
    return (current ? current : allocateCurrent());
}

void setCurrentError(EGLint error)
{
    Current *current = getCurrentData();

    current->error = error;
}

EGLint getCurrentError()
{
    Current *current = getCurrentData();

    return current->error;
}

void setCurrentAPI(EGLenum API)
{
    Current *current = getCurrentData();

    current->API = API;
}

EGLenum getCurrentAPI()
{
    Current *current = getCurrentData();

    return current->API;
}

void setCurrentDisplay(EGLDisplay dpy)
{
    Current *current = getCurrentData();

    current->display = dpy;
}

EGLDisplay getCurrentDisplay()
{
    Current *current = getCurrentData();

    return current->display;
}

void setCurrentDrawSurface(EGLSurface surface)
{
    Current *current = getCurrentData();

    current->drawSurface = surface;
}

EGLSurface getCurrentDrawSurface()
{
    Current *current = getCurrentData();

    return current->drawSurface;
}

void setCurrentReadSurface(EGLSurface surface)
{
    Current *current = getCurrentData();

    current->readSurface = surface;
}

EGLSurface getCurrentReadSurface()
{
    Current *current = getCurrentData();

    return current->readSurface;
}

void error(EGLint errorCode)
{
    egl::setCurrentError(errorCode);
}

}
