//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayGLX.h: GLX implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_

#define __glext_h_ 1 // HORRIBLE HACK FIXME FIXME FIXME
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <string>
#include <vector>

#include "libANGLE/renderer/gl/DisplayGL.h"

namespace rx
{

class FunctionsGLX;

class DisplayGLX : public DisplayGL
{
  public:
    DisplayGLX();
    ~DisplayGLX() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    egl::Error createWindowSurface(const egl::Config *configuration, EGLNativeWindowType window, const egl::AttributeMap &attribs,
                                   SurfaceImpl **outSurface) override;
    egl::Error createPbufferSurface(const egl::Config *configuration, const egl::AttributeMap &attribs,
                                    SurfaceImpl **outSurface) override;
    egl::Error createPbufferFromClientBuffer(const egl::Config *configuration, EGLClientBuffer shareHandle,
                                             const egl::AttributeMap &attribs, SurfaceImpl **outSurface) override;
    egl::Error createPixmapSurface(const egl::Config *configuration, NativePixmapType nativePixmap,
                                   const egl::AttributeMap &attribs, SurfaceImpl **outSurface) override;

    egl::ConfigSet generateConfigs() const override;

    bool isDeviceLost() const override;
    bool testDeviceLost() override;
    egl::Error restoreLostDevice() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    egl::Error getDevice(DeviceImpl **device) override;

    std::string getVendorString() const override;

  private:
    const FunctionsGL *getFunctionsGL() const override;

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    bool hasGLXExtension(const char *extension) const;
    int getGLXFBConfigAttrib(GLXFBConfig config, int attrib) const;

    FunctionsGL *mFunctionsGL;

    //TODO(cwallez) yuck, change generateConfigs to be non-const or add a userdata member to egl::Config?
    mutable std::map<int, GLXFBConfig> configIdToGLXConfig;
    std::vector<std::string> mGLXExtensions;

    // The ID of the visual used to create the context
    int mContextVisualId;
    GLXContext mContext;

    egl::Display *mEGLDisplay;
    Display *mDisplay;
};

}

#endif // LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_H_
