//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGLWindow:
//   Implements initializing a WGL rendering context.
//

#include "util/windows/WGLWindow.h"

#include "common/string_utils.h"
#include "util/OSWindow.h"
#include "util/system_utils.h"
#include "util/windows/wgl_loader_autogen.h"

#include <iostream>

namespace
{
PIXELFORMATDESCRIPTOR GetDefaultPixelFormatDescriptor()
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {0};
    pixelFormatDescriptor.nSize                 = sizeof(pixelFormatDescriptor);
    pixelFormatDescriptor.nVersion              = 1;
    pixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType   = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits   = 24;
    pixelFormatDescriptor.cAlphaBits   = 8;
    pixelFormatDescriptor.cDepthBits   = 24;
    pixelFormatDescriptor.cStencilBits = 8;
    pixelFormatDescriptor.iLayerType   = PFD_MAIN_PLANE;

    return pixelFormatDescriptor;
}

PFNWGLGETPROCADDRESSPROC gCurrentWGLGetProcAddress = nullptr;
HMODULE gCurrentModule                             = nullptr;

angle::GenericProc GetProcAddressWithFallback(const char *name)
{
    angle::GenericProc proc = reinterpret_cast<angle::GenericProc>(gCurrentWGLGetProcAddress(name));
    if (proc)
    {
        return proc;
    }

    return reinterpret_cast<angle::GenericProc>(GetProcAddress(gCurrentModule, name));
}

bool HasExtension(const std::vector<std::string> &extensions, const char *ext)
{
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}
}  // namespace

WGLWindow::WGLWindow(int glesMajorVersion, int glesMinorVersion)
    : mDeviceContext(nullptr), mWGLContext(nullptr), mWindow(nullptr)
{}

WGLWindow::~WGLWindow() {}

// Internally initializes GL resources.
bool WGLWindow::initializeGL(OSWindow *osWindow, angle::Library *wglLibrary)
{
    wglLibrary->getAs("wglGetProcAddress", &gCurrentWGLGetProcAddress);

    if (!gCurrentWGLGetProcAddress)
        return false;

    gCurrentModule = reinterpret_cast<HMODULE>(wglLibrary->getNative());
    angle::LoadWGL(GetProcAddressWithFallback);

    mWindow                                           = osWindow->getNativeWindow();
    mDeviceContext                                    = GetDC(mWindow);
    const PIXELFORMATDESCRIPTOR pixelFormatDescriptor = GetDefaultPixelFormatDescriptor();

    int pixelFormat = ChoosePixelFormat(mDeviceContext, &pixelFormatDescriptor);
    if (pixelFormat == 0)
    {
        std::cerr << "Could not find a compatible pixel format for the dummy OpenGL window."
                  << std::endl;
        return false;
    }

    if (!SetPixelFormat(mDeviceContext, pixelFormat, &pixelFormatDescriptor))
    {
        std::cerr << "Failed to set the pixel format on the intermediate OpenGL window."
                  << std::endl;
        return false;
    }

    mWGLContext = _wglCreateContext(mDeviceContext);
    if (!mWGLContext)
    {
        std::cerr << "Failed to create a WGL context for the dummy OpenGL window." << std::endl;
        return false;
    }

    makeCurrent();

    // Reload entry points to capture extensions.
    angle::LoadWGL(GetProcAddressWithFallback);

    if (!_wglGetExtensionsStringARB)
    {
        return false;
    }

    const char *extensionsString = _wglGetExtensionsStringARB(mDeviceContext);

    std::vector<std::string> extensions;
    angle::SplitStringAlongWhitespace(extensionsString, &extensions);

    return HasExtension(extensions, "WGL_EXT_create_context_es2_profile");
}

void WGLWindow::destroyGL()
{
    if (mWGLContext)
    {
        _wglDeleteContext(mWGLContext);
        mWGLContext = nullptr;
    }

    if (mDeviceContext)
    {
        ReleaseDC(mWindow, mDeviceContext);
        mDeviceContext = nullptr;
    }
}

bool WGLWindow::isGLInitialized() const
{
    return false;
}

void WGLWindow::makeCurrent()
{
    if (_wglMakeCurrent(mDeviceContext, mWGLContext) == FALSE)
    {
        std::cerr << "Error during wglMakeCurrent." << std::endl;
    }
}
