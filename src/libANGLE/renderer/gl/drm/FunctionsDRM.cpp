//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsDRM.cpp: Implements the FunctionsDRM class.

#define ANGLE_SKIP_DRM_DEFINES 1
#include "libANGLE/renderer/gl/drm/FunctionsDRM.h"
#undef ANGLE_SKIP_DRM_DEFINES

/*XXX
// We can only include egl.h in files which do not include ANGLE's EGL
// headers, to avoid doubly-defined GLenum macros, typedefs, etc.
#include <GL/drm.h>
*/

#include <dlfcn.h>
#include <algorithm>

#include "common/string_utils.h"
//XXX #include "libANGLE/renderer/gl/drm/functionsdrm_typedefs.h"

namespace rx
{

void* FunctionsDRM::sLibHandle = nullptr;

template<typename T>
static bool GetProc(FunctionsDRM::GETPROC getProc, T *member, const char *name)
{
    *member = reinterpret_cast<T>(getProc(name));
    return *member != nullptr;
}

struct FunctionsDRM::DRMFunctionTable
{
    DRMFunctionTable()
        :
          createContextPtr(nullptr),
          makeCurrentPtr(nullptr),

          destroyContextPtr(nullptr),
          swapBuffersPtr(nullptr),
          queryExtensionPtr(nullptr),
          queryVersionPtr(nullptr),
          getCurrentContextPtr(nullptr),
          getCurrentDrawablePtr(nullptr),
          waitXPtr(nullptr),
          waitGLPtr(nullptr),
          queryExtensionsStringPtr(nullptr),
          getFBConfigsPtr(nullptr),
          chooseFBConfigPtr(nullptr),
          getFBConfigAttribPtr(nullptr),
          getVisualFromFBConfigPtr(nullptr),
          createWindowPtr(nullptr),
          destroyWindowPtr(nullptr),
          createPbufferPtr(nullptr),
          destroyPbufferPtr(nullptr),
          queryDrawablePtr(nullptr),
          createContextAttribsARBPtr(nullptr),
          swapIntervalEXTPtr(nullptr),
          swapIntervalMESAPtr(nullptr),
          swapIntervalSGIPtr(nullptr)
    {
    }

    EGLContext (*createContextPtr)(EGLDisplay display, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
    EGLBoolean (*makeCurrentPtr)(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context);

    //XXX DRM 1.0
    typedef int(*FOO)(...);
    //FOO createContextPtr;
    FOO destroyContextPtr;
    //FOO makeCurrentPtr;
    FOO swapBuffersPtr;
    FOO queryExtensionPtr;
    FOO queryVersionPtr;
    FOO getCurrentContextPtr;
    FOO getCurrentDrawablePtr;
    FOO waitXPtr;
    FOO waitGLPtr;

    // DRM 1.1
    FOO queryExtensionsStringPtr;

    //DRM 1.3
    FOO getFBConfigsPtr;
    FOO chooseFBConfigPtr;
    FOO getFBConfigAttribPtr;
    FOO getVisualFromFBConfigPtr;
    FOO createWindowPtr;
    FOO destroyWindowPtr;
    FOO createPbufferPtr;
    FOO destroyPbufferPtr;
    FOO queryDrawablePtr;

    // DRM_ARB_create_context
    FOO createContextAttribsARBPtr;

    // DRM_EXT_swap_control
    FOO swapIntervalEXTPtr;

    // DRM_MESA_swap_control
    FOO swapIntervalMESAPtr;

    // DRM_SGI_swap_control
    FOO swapIntervalSGIPtr;
};

FunctionsDRM::FunctionsDRM()
  : majorVersion(0),
    minorVersion(0),
    mXDisplay(nullptr),
    mXScreen(-1),
    mFnPtrs(new DRMFunctionTable())
{
}

FunctionsDRM::~FunctionsDRM()
{
    delete mFnPtrs;
    terminate();
}

bool FunctionsDRM::initialize(EGLNativeDisplayType nativeDisplay, std::string *errorString)
{
    terminate();
/*XXX
    mXDisplay = xDisplay;
    mXScreen = screen;
*/

#if !defined(ANGLE_LINK_EGL)
    // Some OpenGL implementations can't handle having this library
    // handle closed while there's any X window still open against
    // which a DRMWindow was ever created.
    if (!sLibHandle)
    {
        sLibHandle = dlopen("libEGL.so.1", RTLD_NOW);
        if (!sLibHandle)
        {
            *errorString = std::string("Could not dlopen libEGL.so.1: ") + dlerror();
            return false;
        }
    }

    getProc = reinterpret_cast<GETPROC>(dlsym(sLibHandle, "eglGetProcAddress"));
    if (!getProc)
    {
        *errorString = "Could not retrieve eglGetProcAddress";
        return false;
    }
#else
    getProc = reinterpret_cast<GETPROC>(eglGetProcAddress);
#endif

#define GET_PROC_OR_ERROR(MEMBER, NAME) \
    if (!GetProc(getProc, MEMBER, #NAME)) \
    { \
        *errorString = "Could not load DRM entry point " #NAME; \
        return false; \
    }
#if !defined(ANGLE_LINK_DRM)
#define GET_FNPTR_OR_ERROR(MEMBER, NAME) GET_PROC_OR_ERROR(MEMBER, NAME)
#else
#define GET_FNPTR_OR_ERROR(MEMBER, NAME) *MEMBER = NAME;
#endif

    EGLBoolean (*initializePtr)(EGLDisplay display, EGLint *major, EGLint *minor);
    EGLDisplay (*getDisplayPtr)(NativeDisplayType native_display);

    GET_FNPTR_OR_ERROR(&initializePtr, eglInitialize);
    GET_FNPTR_OR_ERROR(&getDisplayPtr, eglGetDisplay);

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

    GET_FNPTR_OR_ERROR(&mFnPtrs->createContextPtr, eglCreateContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->makeCurrentPtr, eglMakeCurrent);

#ifdef XXX
    // DRM 1.0
    GET_FNPTR_OR_ERROR(&mFnPtrs->createContextPtr, glXCreateContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyContextPtr, glXDestroyContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->makeCurrentPtr, glXMakeCurrent);
    GET_FNPTR_OR_ERROR(&mFnPtrs->swapBuffersPtr, glXSwapBuffers);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryExtensionPtr, glXQueryExtension);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryVersionPtr, glXQueryVersion);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getCurrentContextPtr, glXGetCurrentContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getCurrentDrawablePtr, glXGetCurrentDrawable);
    GET_FNPTR_OR_ERROR(&mFnPtrs->waitXPtr, glXWaitX);
    GET_FNPTR_OR_ERROR(&mFnPtrs->waitGLPtr, glXWaitGL);

    // DRM 1.1
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryExtensionsStringPtr, glXQueryExtensionsString);

    // Check we have a working DRM
    {
        int errorBase;
        int eventBase;
        if (!queryExtension(&errorBase, &eventBase))
        {
            *errorString = "DRM is not present.";
            return false;
        }
    }

    // Check we have a supported version of DRM
    if (!queryVersion(&majorVersion, &minorVersion))
    {
        *errorString = "Could not query the DRM version.";
        return false;
    }
    if (majorVersion != 1 || minorVersion < 3)
    {
        *errorString = "Unsupported DRM version (requires at least 1.3).";
        return false;
    }

    const char *extensions = queryExtensionsString();
    if (!extensions)
    {
        *errorString = "glXQueryExtensionsString returned NULL";
        return false;
    }
    angle::SplitStringAlongWhitespace(extensions, &mExtensions);

    // DRM 1.3
    GET_FNPTR_OR_ERROR(&mFnPtrs->getFBConfigsPtr, glXGetFBConfigs);
    GET_FNPTR_OR_ERROR(&mFnPtrs->chooseFBConfigPtr, glXChooseFBConfig);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getFBConfigAttribPtr, glXGetFBConfigAttrib);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getVisualFromFBConfigPtr, glXGetVisualFromFBConfig);
    GET_FNPTR_OR_ERROR(&mFnPtrs->createWindowPtr, glXCreateWindow);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyWindowPtr, glXDestroyWindow);
    GET_FNPTR_OR_ERROR(&mFnPtrs->createPbufferPtr, glXCreatePbuffer);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyPbufferPtr, glXDestroyPbuffer);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryDrawablePtr, glXQueryDrawable);

    // Extensions
    if (hasExtension("DRM_ARB_create_context"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->createContextAttribsARBPtr, glXCreateContextAttribsARB);
    }
    if (hasExtension("DRM_EXT_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalEXTPtr, glXSwapIntervalEXT);
    }
    if (hasExtension("DRM_MESA_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalMESAPtr, glXSwapIntervalMESA);
    }
    if (hasExtension("DRM_SGI_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalSGIPtr, glXSwapIntervalSGI);
    }
#endif
#undef GET_FNPTR_OR_ERROR
#undef GET_PROC_OR_ERROR

    *errorString = "";
    return true;
}

void FunctionsDRM::terminate()
{
}

bool FunctionsDRM::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

Display *FunctionsDRM::getDisplay() const
{
    return mXDisplay;
}

int FunctionsDRM::getScreen() const
{
    return mXScreen;
}

// DRM functions

EGLContext FunctionsDRM::createContext(EGLConfig config, EGLContext share_context, EGLint const * attrib_list) const
{
    return mFnPtrs->createContextPtr(mEGLDisplay, config, share_context, attrib_list);
}

EGLBoolean FunctionsDRM::makeCurrent(EGLSurface surface, EGLContext context) const
{
    return mFnPtrs->makeCurrentPtr(mEGLDisplay, surface, surface, context);
}


// DRM 1.0
/*XXX
drm::Context FunctionsDRM::createContext(XVisualInfo *visual, drm::Context share, bool direct) const
{
    DRMContext shareCtx = reinterpret_cast<DRMContext>(share);
    DRMContext context = mFnPtrs->createContextPtr(mXDisplay, visual, shareCtx, direct);
    return reinterpret_cast<drm::Context>(context);
    return nullptr;
}
*/

void FunctionsDRM::destroyContext(drm::Context context) const
{/*XXX
    DRMContext ctx = reinterpret_cast<DRMContext>(context);
    mFnPtrs->destroyContextPtr(mXDisplay, ctx);
*/
}
/*XXX
Bool FunctionsDRM::makeCurrent(drm::Drawable drawable, drm::Context context) const
{
    DRMContext ctx = reinterpret_cast<DRMContext>(context);
    return mFnPtrs->makeCurrentPtr(mXDisplay, drawable, ctx);
    return 0;
}
*/
void FunctionsDRM::swapBuffers(drm::Drawable drawable) const
{
    //XXX mFnPtrs->swapBuffersPtr(mXDisplay, drawable);
}
/*XXX
Bool FunctionsDRM::queryExtension(int *errorBase, int *event) const
{
    return mFnPtrs->queryExtensionPtr(mXDisplay, errorBase, event);
}
*/
/*XXX
Bool FunctionsDRM::queryVersion(int *major, int *minor) const
{
    return mFnPtrs->queryVersionPtr(mXDisplay, major, minor);
}
*/
/*XXX
drm::Context FunctionsDRM::getCurrentContext() const
{
    DRMContext context = mFnPtrs->getCurrentContextPtr();
    return reinterpret_cast<drm::Context>(context);
}
*/
/*XXX
drm::Drawable FunctionsDRM::getCurrentDrawable() const
{
    DRMDrawable drawable = mFnPtrs->getCurrentDrawablePtr();
    return reinterpret_cast<drm::Drawable>(drawable);
}
*/
void FunctionsDRM::waitX() const
{
    //XXX mFnPtrs->waitXPtr();
}
void FunctionsDRM::waitGL() const
{
    //XXX mFnPtrs->waitGLPtr();
}
/*XXX

// DRM 1.1
const char *FunctionsDRM::queryExtensionsString() const
{
    return mFnPtrs->queryExtensionsStringPtr(mXDisplay, mXScreen);
}
*/
/*XXX

// DRM 1.4
drm::FBConfig *FunctionsDRM::getFBConfigs(int *nElements) const
{
    DRMFBConfig *configs = mFnPtrs->getFBConfigsPtr(mXDisplay, mXScreen, nElements);
    return reinterpret_cast<drm::FBConfig*>(configs);
}
*/
/*XXX
drm::FBConfig *FunctionsDRM::chooseFBConfig(const int *attribList, int *nElements) const
{
    DRMFBConfig *configs = mFnPtrs->chooseFBConfigPtr(mXDisplay, mXScreen, attribList, nElements);
    return reinterpret_cast<drm::FBConfig*>(configs);
}
*/
int FunctionsDRM::getFBConfigAttrib(drm::FBConfig config, int attribute, int *value) const
{
/*XXX
    DRMFBConfig cfg = reinterpret_cast<DRMFBConfig>(config);
    return mFnPtrs->getFBConfigAttribPtr(mXDisplay, cfg, attribute, value);
*/
    return 0;
}
XVisualInfo *FunctionsDRM::getVisualFromFBConfig(drm::FBConfig config) const
{
/*XXX
    DRMFBConfig cfg = reinterpret_cast<DRMFBConfig>(config);
    return mFnPtrs->getVisualFromFBConfigPtr(mXDisplay, cfg);
*/
    return nullptr;
}
drm::Window FunctionsDRM::createWindow(drm::FBConfig config, Window window, const int *attribList) const
{
/*XXX
    DRMFBConfig cfg = reinterpret_cast<DRMFBConfig>(config);
    return mFnPtrs->createWindowPtr(mXDisplay, cfg, window, attribList);
*/
    return 0;
}
void FunctionsDRM::destroyWindow(drm::Window window) const
{
    //XXX mFnPtrs->destroyWindowPtr(mXDisplay, window);
}
/*XXX
drm::Pbuffer FunctionsDRM::createPbuffer(drm::FBConfig config, const int *attribList) const
{
    DRMFBConfig cfg = reinterpret_cast<DRMFBConfig>(config);
    return mFnPtrs->createPbufferPtr(mXDisplay, cfg, attribList);
}
*/

void FunctionsDRM::destroyPbuffer(drm::Pbuffer pbuffer) const
{
    //XXX mFnPtrs->destroyPbufferPtr(mXDisplay, pbuffer);
}

/*XXX
void FunctionsDRM::queryDrawable(drm::Drawable drawable, int attribute, unsigned int *value) const
{
    mFnPtrs->queryDrawablePtr(mXDisplay, drawable, attribute, value);
}
*/

// DRM_ARB_create_context
drm::Context FunctionsDRM::createContextAttribsARB(drm::FBConfig config, drm::Context shareContext, Bool direct, const int *attribList) const
{
/*XXX
    DRMContext shareCtx = reinterpret_cast<DRMContext>(shareContext);
    DRMFBConfig cfg = reinterpret_cast<DRMFBConfig>(config);
    DRMContext ctx = mFnPtrs->createContextAttribsARBPtr(mXDisplay, cfg, shareCtx, direct, attribList);
    return reinterpret_cast<drm::Context>(ctx);
*/
    return nullptr;
}
/*XXX

void FunctionsDRM::swapIntervalEXT(drm::Drawable drawable, int intervals) const
{
    mFnPtrs->swapIntervalEXTPtr(mXDisplay, drawable, intervals);
}
*/
/*XXX

int FunctionsDRM::swapIntervalMESA(int intervals) const
{
    return mFnPtrs->swapIntervalMESAPtr(intervals);
}
*/
/*XXX

int FunctionsDRM::swapIntervalSGI(int intervals) const
{
    return mFnPtrs->swapIntervalSGIPtr(intervals);
}
*/

}
