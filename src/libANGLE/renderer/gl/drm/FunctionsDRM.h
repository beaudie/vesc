//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
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

    // Load data from EGL, can be called multiple times
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


    // DRM 1.0
    //drm::Context createContext(XVisualInfo *visual, drm::Context share, bool direct) const;
    //void destroyContext(drm::Context context) const;
    //Bool makeCurrent(drm::Drawable drawable, drm::Context context) const;
    void swapBuffers(drm::Drawable drawable) const;
    Bool queryExtension(int *errorBase, int *event) const;
    Bool queryVersion(int *major, int *minor) const;
    drm::Context getCurrentContext() const;
    drm::Drawable getCurrentDrawable() const;
    void waitX() const;
    void waitGL() const;

    // DRM 1.1
    const char *queryExtensionsString() const;

    // DRM 1.3
    drm::FBConfig *getFBConfigs(int *nElements) const;
    drm::FBConfig *chooseFBConfig(const int *attribList, int *nElements) const;
    int getFBConfigAttrib(drm::FBConfig config, int attribute, int *value) const;
    XVisualInfo *getVisualFromFBConfig(drm::FBConfig config) const;
    drm::Window createWindow(drm::FBConfig config, Window window, const int *attribList) const;
    void destroyWindow(drm::Window window) const;
    drm::Pbuffer createPbuffer(drm::FBConfig config, const int *attribList) const;
    void destroyPbuffer(drm::Pbuffer pbuffer) const;
    void queryDrawable(drm::Drawable drawable, int attribute, unsigned int *value) const;

    // DRM_ARB_create_context
    drm::Context createContextAttribsARB(drm::FBConfig config, drm::Context shareContext, Bool direct, const int *attribList) const;

    // DRM_EXT_swap_control
    void swapIntervalEXT(drm::Drawable drawable, int interval) const;

    // DRM_MESA_swap_control
    int swapIntervalMESA(int interval) const;

    // DRM_SGI_swap_control
    int swapIntervalSGI(int interval) const;

  private:
    // So as to isolate DRM from angle we do not include angleutils.h and cannot
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
