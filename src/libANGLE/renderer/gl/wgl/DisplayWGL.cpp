//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"

#include "libANGLE/Config.h"

namespace rx
{

DisplayWGL::DisplayWGL()
{
}

DisplayWGL::~DisplayWGL()
{
}

egl::Error DisplayWGL::initialize(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attribMap)
{
    return DisplayGL::initialize(display, nativeDisplay, attribMap);
}

void DisplayWGL::terminate()
{
    DisplayGL::terminate();
}

egl::Error DisplayWGL::createWindowSurface(const egl::Config *configuration, EGLNativeWindowType window,
                                           const egl::AttributeMap &attribs, SurfaceImpl **outSurface)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayWGL::createPbufferSurface(const egl::Config *configuration, const egl::AttributeMap &attribs,
                                            SurfaceImpl **outSurface)
{
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::Error DisplayWGL::createPbufferFromClientBuffer(const egl::Config *configuration, EGLClientBuffer shareHandle,
                                                     const egl::AttributeMap &attribs, SurfaceImpl **outSurface)
{
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::Error DisplayWGL::makeCurrent(egl::Surface *drawSurface, egl::Surface *readSurface, gl::Context *context)
{
    return egl::Error(EGL_SUCCESS);
}

egl::ConfigSet DisplayWGL::generateConfigs() const
{
    egl::ConfigSet configs;
    return configs;
}

bool DisplayWGL::isDeviceLost() const
{
    return false;
}

bool DisplayWGL::testDeviceLost()
{
    return false;
}

egl::Error DisplayWGL::restoreLostDevice()
{
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayWGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayWGL::getVendorString() const
{
    return "";
}

void DisplayWGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
}

void DisplayWGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

}
