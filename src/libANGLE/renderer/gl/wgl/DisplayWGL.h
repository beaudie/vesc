//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_

#include "common/platform.h"
#include "libANGLE/renderer/gl/DisplayGL.h"

namespace rx
{

class DisplayWGL : public DisplayGL
{
  public:
    DisplayWGL();
    virtual ~DisplayWGL();

    virtual egl::Error initialize(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attribMap);
    virtual void terminate();

    virtual SurfaceImpl *createWindowSurface(egl::Display *display, const egl::Config *config, EGLNativeWindowType window,
                                             EGLint fixedSize, EGLint width, EGLint height, EGLint postSubBufferSupported);
    virtual SurfaceImpl *createOffscreenSurface(egl::Display *display, const egl::Config *config, EGLClientBuffer shareHandle,
                                                EGLint width, EGLint height, EGLenum textureFormat, EGLenum textureTarget);
    virtual egl::Error createContext(const egl::Config *config, const gl::Context *shareContext, const egl::AttributeMap &attribs,
                                     gl::Context **outContext);

    virtual egl::ConfigSet generateConfigs() const;

    virtual bool isDeviceLost() const;
    virtual bool testDeviceLost();
    virtual egl::Error restoreLostDevice();

    virtual bool isValidNativeWindow(EGLNativeWindowType window) const;

    virtual std::string getVendorString() const;

  private:
    virtual void generateExtensions(egl::DisplayExtensions *outExtensions) const;
    virtual void generateCaps(egl::Caps *outCaps) const;

    bool isWGLExtensionSupported(HDC deviceContext, const std::string &name) const;

    HMODULE mOpenGLModule;
    GLuint mGLVersionMajor;
    GLuint mGLVersionMinor;

    PFNWGLGETEXTENSIONSSTRINGEXTPROC mGetExtensionStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC mGetExtensionStringARB;

    PFNWGLCREATECONTEXTATTRIBSARBPROC mCreateContextAttribsARB;

    PFNWGLGETPIXELFORMATATTRIBIVARBPROC mGetPixelFormatAttribivARB;

    ATOM mWindowClass;
    HWND mWindow;
    HDC mDeviceContext;
    int mPixelFormat;
    HGLRC mWGLContext;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_
