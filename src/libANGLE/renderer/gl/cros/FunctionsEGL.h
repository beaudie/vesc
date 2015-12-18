//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGL.h: Defines the FunctionsEGL class to load functions and data from EGL

#ifndef LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_
#define LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

namespace rx
{

class FunctionsEGL
{
  public:
    FunctionsEGL();
    ~FunctionsEGL();

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
    FunctionsEGL(const FunctionsEGL&) = delete;
    void operator=(const FunctionsEGL&) = delete;

    struct EGLFunctionTable;

    static void *sLibHandle;
    EGLDisplay mEGLDisplay;

    EGLFunctionTable *mFnPtrs;
    std::vector<std::string> mExtensions;
};

}

#endif // LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_
