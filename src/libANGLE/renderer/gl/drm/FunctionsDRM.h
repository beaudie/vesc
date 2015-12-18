//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsDRM.h: Defines the FunctionsDRM class to load functions and data from DRM

#ifndef LIBANGLE_RENDERER_GL_DRM_FUNCTIONSDRM_H_
#define LIBANGLE_RENDERER_GL_DRM_FUNCTIONSDRM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/drm/platform_drm.h"

namespace rx
{

class FunctionsDRM
{
  public:
    FunctionsDRM();
    ~FunctionsDRM();

    bool initialize(EGLNativeDisplayType nativeDisplay, std::string *errorString);
    void terminate();

    bool hasExtension(const char *extension) const;
    int majorVersion;
    int minorVersion;

    EGLDisplay getDisplay() const;

    typedef __eglMustCastToProperFunctionPointerType (*GETPROC)(const char *procname);
    GETPROC getProc;

    EGLContext createContext(EGLConfig config, EGLContext share_context, EGLint const * attrib_list) const;
    EGLBoolean destroyContext(EGLContext context) const;
    EGLBoolean makeCurrent(EGLSurface surface, EGLContext context) const;
    const char *queryString(EGLint name) const;

    EGLImageKHR createImage(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) const;
    EGLBoolean destroyImage(EGLImageKHR image) const;

  private:
    // So as to isolate from angle we do not include angleutils.h and cannot
    // use angle::NonCopyable so we replicated it here instead.
    FunctionsDRM(const FunctionsDRM&) = delete;
    void operator=(const FunctionsDRM&) = delete;

    struct DRMFunctionTable;

    static void *sLibHandle;
    EGLDisplay mEGLDisplay;

    DRMFunctionTable *mFnPtrs;
    std::vector<std::string> mExtensions;
};

}

#endif // LIBANGLE_RENDERER_GL_DRM_FUNCTIONSDRM_H_
