//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

// TODO(kbr): figure out what to do for this file -- would be ideal to
// link against Chromium's base/ and pick up its TLS implementation
// from there.

#include "libGLESv2/main.h"
#include "libGLESv2/Context.h"

#include "common/tls.h"

static TLSIndex currentTLS = TLS_INVALID_INDEX;

namespace gl
{

Current *AllocateCurrent()
{
#ifdef ANGLE_PLATFORM_WINDOWS
    ASSERT(currentTLS != TLS_INVALID_INDEX);
    if (currentTLS == TLS_INVALID_INDEX)
    {
        return NULL;
    }

    Current *current = new Current();
    current->context = NULL;
    current->display = NULL;

    if (!SetTLSValue(currentTLS, current))
    {
        ERR("Could not set thread local storage.");
        return NULL;
    }

    return current;
#else
    // TODO(kbr): refactor above code.
    return NULL;
#endif
}

void DeallocateCurrent()
{
#ifdef ANGLE_PLATFORM_WINDOWS
    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));
    SafeDelete(current);
    SetTLSValue(currentTLS, NULL);
#else
    // TODO(kbr): refactor above code.
#endif
}

}

#ifdef ANGLE_PLATFORM_WINDOWS
extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        {
            currentTLS = CreateTLSIndex();
            if (currentTLS == TLS_INVALID_INDEX)
            {
                return FALSE;
            }
        }
        // Fall through to initialize index
      case DLL_THREAD_ATTACH:
        {
            gl::AllocateCurrent();
        }
        break;
      case DLL_THREAD_DETACH:
        {
            gl::DeallocateCurrent();
        }
        break;
      case DLL_PROCESS_DETACH:
        {
            gl::DeallocateCurrent();
            DestroyTLSIndex(currentTLS);
        }
        break;
      default:
        break;
    }

    return TRUE;
}
#else
    // TODO(kbr): refactor above code.
#endif

namespace gl
{

Current *GetCurrentData()
{
#ifdef ANGLE_PLATFORM_WINDOWS
    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));

    // ANGLE issue 488: when the dll is loaded after thread initialization,
    // thread local storage (current) might not exist yet.
    return (current ? current : AllocateCurrent());
#else
    // TODO(kbr): refactor above code.
    return NULL;
#endif
}

void makeCurrent(Context *context, egl::Display *display, egl::Surface *surface)
{
#ifdef ANGLE_PLATFORM_WINDOWS
    Current *current = GetCurrentData();

    current->context = context;
    current->display = display;

    if (context && display && surface)
    {
        context->makeCurrent(surface);
    }
#else
    // TODO(kbr): refactor above code.
#endif
}

Context *getContext()
{
#ifdef ANGLE_PLATFORM_WINDOWS
    Current *current = GetCurrentData();

    return current->context;
#else
    // TODO(kbr): refactor above code.
    return NULL;
#endif
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
#ifdef ANGLE_PLATFORM_WINDOWS
    Current *current = GetCurrentData();

    return current->display;
#else
    // TODO(kbr): refactor above code.
    return NULL;
#endif
}

// Records an error code
void error(GLenum errorCode)
{
    gl::Context *context = glGetCurrentContext();
    context->recordError(Error(errorCode));

    switch (errorCode)
    {
      case GL_INVALID_ENUM:
        TRACE("\t! Error generated: invalid enum\n");
        break;
      case GL_INVALID_VALUE:
        TRACE("\t! Error generated: invalid value\n");
        break;
      case GL_INVALID_OPERATION:
        TRACE("\t! Error generated: invalid operation\n");
        break;
      case GL_OUT_OF_MEMORY:
        TRACE("\t! Error generated: out of memory\n");
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        TRACE("\t! Error generated: invalid framebuffer operation\n");
        break;
      default: UNREACHABLE();
    }
}

}

