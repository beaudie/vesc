#include "precompiled.h"
//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "libGLESv2/main.h"

#include "libGLESv2/Context.h"

static DWORD currentTLS = TLS_OUT_OF_INDEXES;

namespace gl
{

Current *allocateCurrent()
{
    Current *current = (Current*)LocalAlloc(LPTR, sizeof(Current));

    if (!current)
    {
        //ERR("Could not allocate thread local storage.");
        return NULL;
    }

    //ASSERT(currentTLS != TLS_OUT_OF_INDEXES);
    TlsSetValue(currentTLS, current);

    current->context = NULL;
    current->display = NULL;

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
            currentTLS = TlsAlloc();

            if (currentTLS == TLS_OUT_OF_INDEXES)
            {
                return FALSE;
            }
        }
        // Fall throught to initialize index
      case DLL_THREAD_ATTACH:
        {
            gl::allocateCurrent();
        }
        break;
      case DLL_THREAD_DETACH:
        {
            gl::deallocateCurrent();
        }
        break;
      case DLL_PROCESS_DETACH:
        {
            gl::deallocateCurrent();
            TlsFree(currentTLS);
        }
        break;
      default:
        break;
    }

    return TRUE;
}

namespace gl
{

Current *getCurrentData()
{
    Current *current = (Current*)TlsGetValue(currentTLS);
    
    // ANGLE issue 488: when the dll is loaded after thread initialization,
    // thread local storage (current) might not exist yet.
    return (current ? current : allocateCurrent());
}

void makeCurrent(Context *context, egl::Display *display, egl::Surface *surface)
{
    Current *current = getCurrentData();

    current->context = context;
    current->display = display;

    if (context && display && surface)
    {
        context->makeCurrent(surface);
    }
}

Context *getContext()
{
    Current *current = getCurrentData();

    return current->context;
}

Context *getNonLostContext()
{
    Context *context = getContext();
    
    if (context)
    {
        if (context->isContextLost())
        {
            gl::error(GL_OUT_OF_MEMORY);
            return NULL;
        }
        else
        {
            return context;
        }
    }
    return NULL;
}

egl::Display *getDisplay()
{
    Current *current = getCurrentData();

    return current->display;
}

// Records an error code
void error(GLenum errorCode)
{
    gl::Context *context = glGetCurrentContext();

    if (context)
    {
        switch (errorCode)
        {
          case GL_INVALID_ENUM:
            context->recordInvalidEnum();
            TRACE("\t! Error generated: invalid enum\n");
            break;
          case GL_INVALID_VALUE:
            context->recordInvalidValue();
            TRACE("\t! Error generated: invalid value\n");
            break;
          case GL_INVALID_OPERATION:
            context->recordInvalidOperation();
            TRACE("\t! Error generated: invalid operation\n");
            break;
          case GL_OUT_OF_MEMORY:
            context->recordOutOfMemory();
            TRACE("\t! Error generated: out of memory\n");
            break;
          case GL_INVALID_FRAMEBUFFER_OPERATION:
            context->recordInvalidFramebufferOperation();
            TRACE("\t! Error generated: invalid framebuffer operation\n");
            break;
          default: UNREACHABLE();
        }
    }
}

}

