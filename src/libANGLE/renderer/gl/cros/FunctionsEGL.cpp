//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGL.cpp: Implements the FunctionsEGL class.

#include "libANGLE/renderer/gl/cros/FunctionsEGL.h"

#include <dlfcn.h>
#include <algorithm>

#include "common/string_utils.h"

namespace rx
{

void* FunctionsEGL::sLibHandle = nullptr;

template<typename T>
static bool GetProc(FunctionsEGL::GETPROC getProc, T *member, const char *name)
{
    *member = reinterpret_cast<T>(getProc(name));
    return *member != nullptr;
}
struct FunctionsEGL::EGLFunctionTable
{
    EGLFunctionTable()
        : createContextPtr(nullptr),
          destroyContextPtr(nullptr),
          makeCurrentPtr(nullptr),
          queryStringPtr(nullptr),
          createImagePtr(nullptr),
          destroyImagePtr(nullptr)
    {
    }

    EGLContext (*createContextPtr)(EGLDisplay display, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
    EGLBoolean (*destroyContextPtr)(EGLDisplay display, EGLContext context);
    EGLBoolean (*makeCurrentPtr)(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context);
    char const * (*queryStringPtr)(EGLDisplay display, EGLint name);

    PFNEGLCREATEIMAGEKHRPROC createImagePtr;
    PFNEGLDESTROYIMAGEKHRPROC destroyImagePtr;
};

FunctionsEGL::FunctionsEGL()
  : majorVersion(0),
    minorVersion(0),
    mEGLDisplay(EGL_NO_DISPLAY),
    mFnPtrs(new EGLFunctionTable())
{
}

FunctionsEGL::~FunctionsEGL()
{
    delete mFnPtrs;
    terminate();
}

bool FunctionsEGL::initialize(EGLNativeDisplayType nativeDisplay, std::string *errorString)
{
    terminate();

    //XXX shouldn't find angle's anyway since it's not under /usr/lib
    // Open the system EGL lib.  We rely on it having a different name
    // than angle's.  E.g. mesa's has ".1" at the end.
    if (!sLibHandle)
    {
        sLibHandle = dlopen("libEGL.so.1", RTLD_NOW);
        if (!sLibHandle)
        {
            *errorString = std::string("Could not dlopen libEGL: ") + dlerror();
            return false;
        }
    }

    getProc = reinterpret_cast<GETPROC>(dlsym(sLibHandle, "eglGetProcAddress"));
    if (!getProc)
    {
        *errorString = "Could not retrieve eglGetProcAddress";
        return false;
    }

#define GET_PROC_OR_ERROR(MEMBER, NAME) \
    if (!GetProc(getProc, MEMBER, #NAME)) \
    { \
        *errorString = "Could not load EGL entry point " #NAME; \
        return false; \
    }

    EGLBoolean (*initializePtr)(EGLDisplay display, EGLint *major, EGLint *minor);
    EGLDisplay (*getDisplayPtr)(NativeDisplayType native_display);
    EGLBoolean (*bindAPIPtr)(EGLenum api);

    GET_PROC_OR_ERROR(&initializePtr, eglInitialize);
    GET_PROC_OR_ERROR(&getDisplayPtr, eglGetDisplay);
    GET_PROC_OR_ERROR(&bindAPIPtr, eglBindAPI);

    mEGLDisplay = getDisplayPtr(nativeDisplay);
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        *errorString = "Failed to get system egl display";
        return false;
    }
    if (!initializePtr(mEGLDisplay, &majorVersion, &minorVersion))
    {
        *errorString = "Failed to initialize system egl";
        return false;
    }
    if (!bindAPIPtr(EGL_OPENGL_ES_API))
    {
        *errorString = "Failed to bind API in system egl";
        return false;
    }

    GET_PROC_OR_ERROR(&mFnPtrs->createContextPtr, eglCreateContext);
    GET_PROC_OR_ERROR(&mFnPtrs->destroyContextPtr, eglDestroyContext);
    GET_PROC_OR_ERROR(&mFnPtrs->makeCurrentPtr, eglMakeCurrent);
    GET_PROC_OR_ERROR(&mFnPtrs->queryStringPtr, eglQueryString);

    // Check we have a supported version of EGL
    if (majorVersion < 1 || (majorVersion == 1 && minorVersion < 4))
    {
        *errorString = "Unsupported EGL version (requires at least 1.4).";
        return false;
    }

    const char *extensions = queryString(EGL_EXTENSIONS);
    if (!extensions)
    {
        *errorString = "system eglQueryString returned NULL";
        return false;
    }
    angle::SplitStringAlongWhitespace(extensions, &mExtensions);

    // Extensions
    if (hasExtension("EGL_KHR_image_base"))
    {
        //XXX required for chromeos - what should we do when not found?
        GET_PROC_OR_ERROR(&mFnPtrs->createImagePtr, eglCreateImageKHR);
        GET_PROC_OR_ERROR(&mFnPtrs->destroyImagePtr, eglDestroyImageKHR);
    }
    if (hasExtension("EGL_EXT_image_dma_buf_import"))
    {
        // can pass dmabuf fd to EGLCreateImage
        //XXX required for chromeos - what should we do when not found?
    }
    if (hasExtension("EGL_KHR_create_context"))
    {
        // more options for eglCreateContext attrib list
    }
    if (hasExtension("EGL_KHR_get_all_proc_addresses"))
    {
        // can use eglGetProcAddress for any egl or gl function
    }
    if (hasExtension("EGL_MESA_configless_context"))
    {
        //XXX probably mesa-only, but useful in bringing up angle there as we won't have to deal with configs right away
        // can pass NULL config to eglCreateContext
    }
    if (hasExtension("EGL_KHR_surfaceless_context"))
    {
        // EGL_PLATFORM_SURFACELESS
        //XXX required for chromeos - what should we do when not found?
    }

    /* possibly useful extensions
    EGL_KHR_fence_sync
    EGL_KHR_gl_renderbuffer_image
    EGL_KHR_gl_texture_2D_image
    EGL_KHR_wait_sync
    */

#undef GET_PROC_OR_ERROR

    *errorString = "";
    return true;
}

void FunctionsEGL::terminate()
{
}

bool FunctionsEGL::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

EGLDisplay FunctionsEGL::getDisplay() const
{
    return mEGLDisplay;
}

EGLContext FunctionsEGL::createContext(EGLConfig config, EGLContext share_context, EGLint const * attrib_list) const
{
    return mFnPtrs->createContextPtr(mEGLDisplay, config, share_context, attrib_list);
}

EGLBoolean FunctionsEGL::destroyContext(EGLContext context) const
{
    return mFnPtrs->destroyContextPtr(mEGLDisplay, context);
}

EGLBoolean FunctionsEGL::makeCurrent(EGLSurface surface, EGLContext context) const
{
    return mFnPtrs->makeCurrentPtr(mEGLDisplay, surface, surface, context);
}

char const * FunctionsEGL::queryString(EGLint name) const
{
    return mFnPtrs->queryStringPtr(mEGLDisplay, name);
}

EGLImageKHR FunctionsEGL::createImage(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) const
{
    return mFnPtrs->createImagePtr(mEGLDisplay, context, target, buffer, attrib_list);
}

EGLBoolean FunctionsEGL::destroyImage(EGLImageKHR image) const
{
    return mFnPtrs->destroyImagePtr(mEGLDisplay, image);
}

}
