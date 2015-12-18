//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGL.cpp: Implements the FunctionsEGL class.

#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

#include <dlfcn.h>
#include <algorithm>

#include "common/debug.h"
#include "common/string_utils.h"
#include "libANGLE/renderer/gl/egl/functionsegl_typedefs.h"

namespace
{

template <typename T>
bool SetPtr(T *dst, void *src)
{
    if (src)
    {
        *dst = reinterpret_cast<T>(src);
        return true;
    }
    return false;
}
}  // namespace

namespace rx
{

void *FunctionsEGL::sLibHandle = nullptr;

void *FunctionsEGL::getProcAddress(const char *name) const
{
    if (mGetProcAddressPtr)
    {
        void *f = reinterpret_cast<void *>(mGetProcAddressPtr(name));
        if (f)
        {
            return f;
        }
    }
    return dlsym(sLibHandle, name);
}

struct FunctionsEGL::EGLFunctionTable
{
    EGLFunctionTable()
        : getErrorPtr(nullptr),
          terminatePtr(nullptr),
          chooseConfigPtr(nullptr),
          getConfigAttribPtr(nullptr),
          createContextPtr(nullptr),
          destroyContextPtr(nullptr),
          makeCurrentPtr(nullptr),
          queryStringPtr(nullptr),
          createImageKHRPtr(nullptr),
          destroyImageKHRPtr(nullptr),
          createSyncKHRPtr(nullptr),
          destroySyncKHRPtr(nullptr),
          clientWaitSyncKHRPtr(nullptr),
          getSyncAttribKHRPtr(nullptr)
    {
    }

    PFNEGLGETERRORPROC getErrorPtr;
    PFNEGLTERMINATEPROC terminatePtr;
    PFNEGLCHOOSECONFIGPROC chooseConfigPtr;
    PFNEGLGETCONFIGATTRIBPROC getConfigAttribPtr;
    PFNEGLCREATECONTEXTPROC createContextPtr;
    PFNEGLDESTROYCONTEXTPROC destroyContextPtr;
    PFNEGLMAKECURRENTPROC makeCurrentPtr;
    PFNEGLQUERYSTRINGPROC queryStringPtr;

    // EGL_KHR_image
    PFNEGLCREATEIMAGEKHRPROC createImageKHRPtr;
    PFNEGLDESTROYIMAGEKHRPROC destroyImageKHRPtr;

    // EGL_KHR_fence_sync
    PFNEGLCREATESYNCKHRPROC createSyncKHRPtr;
    PFNEGLDESTROYSYNCKHRPROC destroySyncKHRPtr;
    PFNEGLCLIENTWAITSYNCKHRPROC clientWaitSyncKHRPtr;
    PFNEGLGETSYNCATTRIBKHRPROC getSyncAttribKHRPtr;
};

FunctionsEGL::FunctionsEGL()
    : majorVersion(0),
      minorVersion(0),
      mEGLDisplay(EGL_NO_DISPLAY),
      mFnPtrs(new EGLFunctionTable()),
      mGetProcAddressPtr(nullptr)
{
}

FunctionsEGL::~FunctionsEGL()
{
    SafeDelete(mFnPtrs);
}

egl::Error FunctionsEGL::initialize(EGLNativeDisplayType nativeDisplay)
{
    // Angle builds its executables with an RPATH so they pull in Angle's
    // libGL and libEGL.  Here we need to open the platform libEGL.
    // An absolute path would work, but then we couldn't use LD_LIBRARY_PATH
    // which is often useful during development.  Instead we take advantage
    // of the fact that the system lib is available under multiple names
    // (for example with a .1 suffix) while Angle only installs libEGL.so.
    if (!sLibHandle)
    {
        sLibHandle = dlopen("libEGL.so.1", RTLD_NOW);
        if (!sLibHandle)
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Could not dlopen libEGL: %s", dlerror());
        }
    }

#define ANGLE_GET_PROC_OR_ERROR(MEMBER, NAME)                                            \
    if (!SetPtr(MEMBER, getProcAddress(#NAME)))                                          \
    {                                                                                    \
        return egl::Error(EGL_NOT_INITIALIZED, "Could not load EGL entry point " #NAME); \
    }

    ANGLE_GET_PROC_OR_ERROR(&mGetProcAddressPtr, eglGetProcAddress);

    EGLBoolean (*initializePtr)(EGLDisplay display, EGLint *major, EGLint *minor);
    EGLDisplay (*getDisplayPtr)(NativeDisplayType native_display);
    EGLBoolean (*bindAPIPtr)(EGLenum api);

    ANGLE_GET_PROC_OR_ERROR(&initializePtr, eglInitialize);
    ANGLE_GET_PROC_OR_ERROR(&getDisplayPtr, eglGetDisplay);
    ANGLE_GET_PROC_OR_ERROR(&bindAPIPtr, eglBindAPI);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getErrorPtr, eglGetError);

    mEGLDisplay = getDisplayPtr(nativeDisplay);
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get system egl display");
    }
    if (!initializePtr(mEGLDisplay, &majorVersion, &minorVersion))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to initialize system egl");
    }
    if (!bindAPIPtr(EGL_OPENGL_ES_API))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to bind API in system egl");
    }

    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->terminatePtr, eglTerminate);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->chooseConfigPtr, eglChooseConfig);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getConfigAttribPtr, eglGetConfigAttrib);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createContextPtr, eglCreateContext);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroyContextPtr, eglDestroyContext);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->makeCurrentPtr, eglMakeCurrent);
    ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->queryStringPtr, eglQueryString);

    // Check we have a supported version of EGL
    if (majorVersion < 1 || (majorVersion == 1 && minorVersion < 4))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Unsupported EGL version (require at least 1.4).");
    }

    const char *extensions = queryString(EGL_EXTENSIONS);
    if (!extensions)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "System eglQueryString() failed");
    }
    angle::SplitStringAlongWhitespace(extensions, &mExtensions);

    if (hasExtension("EGL_KHR_image_base"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createImageKHRPtr, eglCreateImageKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroyImageKHRPtr, eglDestroyImageKHR);
    }
    if (hasExtension("EGL_KHR_fence_sync"))
    {
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->createSyncKHRPtr, eglCreateSyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->destroySyncKHRPtr, eglDestroySyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->clientWaitSyncKHRPtr, eglClientWaitSyncKHR);
        ANGLE_GET_PROC_OR_ERROR(&mFnPtrs->getSyncAttribKHRPtr, eglGetSyncAttribKHR);
    }

#undef ANGLE_GET_PROC_OR_ERROR

    return egl::Error(EGL_SUCCESS);
}

bool FunctionsEGL::terminate()
{
    ASSERT(mFnPtrs->terminatePtr(mEGLDisplay));
    mEGLDisplay = nullptr;
    return true;
}

bool FunctionsEGL::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

EGLDisplay FunctionsEGL::getDisplay() const
{
    return mEGLDisplay;
}

EGLBoolean FunctionsEGL::chooseConfig(EGLint const *attribList,
                                      EGLConfig *configs,
                                      EGLint configSize,
                                      EGLint *numConfig)
{
    return mFnPtrs->chooseConfigPtr(mEGLDisplay, attribList, configs, configSize, numConfig);
}

EGLBoolean FunctionsEGL::getConfigAttrib(EGLConfig config, EGLint attribute, EGLint *value)
{
    return mFnPtrs->getConfigAttribPtr(mEGLDisplay, config, attribute, value);
}

EGLContext FunctionsEGL::createContext(EGLConfig config,
                                       EGLContext share_context,
                                       EGLint const *attrib_list) const
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

char const *FunctionsEGL::queryString(EGLint name) const
{
    return mFnPtrs->queryStringPtr(mEGLDisplay, name);
}

EGLImageKHR FunctionsEGL::createImageKHR(EGLContext context,
                                         EGLenum target,
                                         EGLClientBuffer buffer,
                                         const EGLint *attrib_list) const
{
    return mFnPtrs->createImageKHRPtr(mEGLDisplay, context, target, buffer, attrib_list);
}

EGLBoolean FunctionsEGL::destroyImageKHR(EGLImageKHR image) const
{
    return mFnPtrs->destroyImageKHRPtr(mEGLDisplay, image);
}

EGLSyncKHR FunctionsEGL::createSyncKHR(EGLenum type, const EGLint *attrib_list)
{
    return mFnPtrs->createSyncKHRPtr(mEGLDisplay, type, attrib_list);
}

EGLBoolean FunctionsEGL::destroySyncKHR(EGLSyncKHR sync)
{
    return mFnPtrs->destroySyncKHRPtr(mEGLDisplay, sync);
}

EGLint FunctionsEGL::clientWaitSyncKHR(EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
    return mFnPtrs->clientWaitSyncKHRPtr(mEGLDisplay, sync, flags, timeout);
}

EGLBoolean FunctionsEGL::getSyncAttribKHR(EGLSyncKHR sync, EGLint attribute, EGLint *value)
{
    return mFnPtrs->getSyncAttribKHRPtr(mEGLDisplay, sync, attribute, value);
}
}  // namespace rx
