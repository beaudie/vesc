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

#include "libANGLE/Error.h"

namespace rx
{

class FunctionsEGL
{
  public:
    FunctionsEGL();
    ~FunctionsEGL();

    egl::Error initialize(EGLNativeDisplayType nativeDisplay);
    egl::Error terminate();

    bool hasExtension(const char *extension) const;
    int majorVersion;
    int minorVersion;

    EGLDisplay getDisplay() const;

    void *getProcAddress(const char *name) const;

    EGLBoolean chooseConfig(EGLint const *attrib_list,
                            EGLConfig *configs,
                            EGLint config_size,
                            EGLint *num_config);
    EGLBoolean getConfigAttrib(EGLConfig config, EGLint attribute, EGLint *value);
    EGLContext createContext(EGLConfig config,
                             EGLContext share_context,
                             EGLint const *attrib_list) const;
    EGLBoolean destroyContext(EGLContext context) const;
    EGLBoolean makeCurrent(EGLSurface surface, EGLContext context) const;
    const char *queryString(EGLint name) const;

    EGLImageKHR createImageKHR(EGLContext context,
                               EGLenum target,
                               EGLClientBuffer buffer,
                               const EGLint *attrib_list) const;
    EGLBoolean destroyImageKHR(EGLImageKHR image) const;

    EGLSyncKHR createSyncKHR(EGLenum type, const EGLint *attrib_list);
    EGLBoolean destroySyncKHR(EGLSyncKHR sync);
    EGLint clientWaitSyncKHR(EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
    EGLBoolean getSyncAttribKHR(EGLSyncKHR sync, EGLint attribute, EGLint *value);

  private:
    // So as to isolate from angle we do not include angleutils.h and cannot
    // use angle::NonCopyable so we replicated it here instead.
    FunctionsEGL(const FunctionsEGL &) = delete;
    void operator=(const FunctionsEGL &) = delete;

    struct EGLDispatchTable;

    static void *sLibHandle;
    EGLDisplay mEGLDisplay;

    EGLDispatchTable *mFnPtrs;
    std::vector<std::string> mExtensions;

    __eglMustCastToProperFunctionPointerType (*mGetProcAddressPtr)(const char *procname);
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CROS_FUNCTIONSEGL_H_
