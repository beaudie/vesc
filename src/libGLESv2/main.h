//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.h: Management of thread-local data.

#ifndef LIBGLESV2_MAIN_H_
#define LIBGLESV2_MAIN_H_

#include "common/debug.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

namespace egl
{
class Display;
class Surface;
}

namespace gl
{
class Context;
    
struct Current
{
    Context *context;
    egl::Display *display;
};

// TODO(kbr): figure out how these are going to be managed on
// non-Windows platforms. These routines would need to be exported
// from ANGLE and called cooperatively when users create and destroy
// threads -- or the initialization of the TLS index, and allocation
// of thread-local data, will have to be done lazily. Will have to use
// destructor function with pthread_create_key on POSIX platforms to
// clean up thread-local data.

// Call this exactly once at process startup.
bool CreateThreadLocalIndex();

// Call this exactly once at process shutdown.
void DestroyThreadLocalIndex();

// Call this upon thread startup.
Current *AllocateCurrent();

// Call this upon thread shutdown.
void DeallocateCurrent();

void makeCurrent(Context *context, egl::Display *display, egl::Surface *surface);

Context *getContext();
Context *getNonLostContext();
egl::Display *getDisplay();

void error(GLenum errorCode);

template<class T>
const T &error(GLenum errorCode, const T &returnValue)
{
    error(errorCode);

    return returnValue;
}

}

namespace rx
{
class Renderer;
}

extern "C"
{
// Exported functions for use by EGL
gl::Context *glCreateContext(int clientVersion, const gl::Context *shareContext, rx::Renderer *renderer, bool notifyResets, bool robustAccess);
void glDestroyContext(gl::Context *context);
void glMakeCurrent(gl::Context *context, egl::Display *display, egl::Surface *surface);
gl::Context *glGetCurrentContext();
rx::Renderer *glCreateRenderer(egl::Display *display, EGLNativeDisplayType nativeDisplay, EGLint requestedDisplayType);
void glDestroyRenderer(rx::Renderer *renderer);

__eglMustCastToProperFunctionPointerType EGLAPIENTRY glGetProcAddress(const char *procname);
bool EGLAPIENTRY glBindTexImage(egl::Surface *surface);
}

#endif   // LIBGLESV2_MAIN_H_
