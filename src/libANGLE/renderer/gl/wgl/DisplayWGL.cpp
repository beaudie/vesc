//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"

#include "libANGLE/Config.h"
#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

#include <EGL/eglext.h>

namespace rx
{

template <typename T>
static T GetWGLProcAddress(HMODULE glModule, const std::string &name)
{
    T proc = reinterpret_cast<T>(wglGetProcAddress(name.c_str()));
    if (proc)
    {
        return proc;
    }

    return reinterpret_cast<T>(GetProcAddress(glModule, name.c_str()));
}

DisplayWGL::DisplayWGL()
{
}

DisplayWGL::~DisplayWGL()
{
}

LRESULT CALLBACK IntermediateWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
      case WM_ERASEBKGND:
        // Prevent windows from erasing the background.
        return 1;
      case WM_PAINT:
        // Do not paint anything.
        PAINTSTRUCT paint;
        if (BeginPaint(window, &paint))
        {
            EndPaint(window, &paint);
        }
        return 0;
    }

    return DefWindowProc(window, message, wParam, lParam);
}

egl::Error DisplayWGL::initialize(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attribMap)
{
    mOpenGLModule = LoadLibraryA("opengl32.dll");
    if (!mOpenGLModule)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to load OpenGL library.");
    }

    // WGL can't grab extensions until it creates a context because it needs to load the driver's DLLs first.
    // Create a dummy context to load the driver and determine which GL versions are available.

    // Work around compile error from not defining "UNICODE" while Chromium does
    const LPSTR idcArrow = MAKEINTRESOURCEA(32512);

    WNDCLASSA intermediateClassDesc = { 0 };
    intermediateClassDesc.style = CS_OWNDC;
    intermediateClassDesc.lpfnWndProc = IntermediateWindowProc;
    intermediateClassDesc.cbClsExtra = 0;
    intermediateClassDesc.cbWndExtra = 0;
    intermediateClassDesc.hInstance = GetModuleHandle(NULL);
    intermediateClassDesc.hIcon = NULL;
    intermediateClassDesc.hCursor = LoadCursorA(NULL, idcArrow);
    intermediateClassDesc.hbrBackground = 0;
    intermediateClassDesc.lpszMenuName = NULL;
    intermediateClassDesc.lpszClassName = "ANGLE Intermediate Window";
    mWindowClass = RegisterClassA(&intermediateClassDesc);
    if (!mWindowClass)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to register intermediate OpenGL window class.");
    }

    HWND dummyWindow = CreateWindowExA(WS_EX_NOPARENTNOTIFY,
                                       reinterpret_cast<const char *>(mWindowClass),
                                       "",
                                       WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);
    if (!dummyWindow)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create dummy OpenGL window.");
    }

    HDC dummyDeviceContext = GetDC(dummyWindow);
    if (!dummyDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get the device context of the dummy OpenGL window.");
    }

    const PIXELFORMATDESCRIPTOR pixelFormatDescriptor = wgl::GetDefaultPixelFormatDescriptor();

    int dummyPixelFormat = ChoosePixelFormat(dummyDeviceContext, &pixelFormatDescriptor);
    if (dummyPixelFormat == 0)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not find a compatible pixel format for the dummy OpenGL window.");
    }

    if (!SetPixelFormat(dummyDeviceContext, dummyPixelFormat, &pixelFormatDescriptor))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to set the pixel format on the intermediate OpenGL window.");
    }

    HGLRC dummyWGLContext = wglCreateContext(dummyDeviceContext);
    if (!dummyDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create a WGL context for the dummy OpenGL window.");
    }

    if (!wglMakeCurrent(dummyDeviceContext, dummyWGLContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to make the dummy WGL context current.");
    }

    // Grab the GL version from this context and use it as the maximum version available.
    typedef const GLubyte* (GL_APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
    PFNGLGETSTRINGPROC getString = reinterpret_cast<PFNGLGETSTRINGPROC>(GetProcAddress(mOpenGLModule, "glGetString"));

    const char *dummyGLVersionString = reinterpret_cast<const char*>(getString(GL_VERSION));
    GLuint maxGLVersionMajor = dummyGLVersionString[0] - '0';
    GLuint maxGLVersionMinor = dummyGLVersionString[2] - '0';

    // Grab WGL extensions
    mGetExtensionStringEXT = GetWGLProcAddress<PFNWGLGETEXTENSIONSSTRINGEXTPROC>(mOpenGLModule, "wglGetExtensionsStringEXT");
    mGetExtensionStringARB = GetWGLProcAddress<PFNWGLGETEXTENSIONSSTRINGARBPROC>(mOpenGLModule, "wglGetExtensionsStringARB");

    if (isWGLExtensionSupported(dummyDeviceContext, "WGL_ARB_create_context"))
    {
        mCreateContextAttribsARB = GetWGLProcAddress<PFNWGLCREATECONTEXTATTRIBSARBPROC>(mOpenGLModule, "wglCreateContextAttribsARB");
    }

    if (isWGLExtensionSupported(dummyDeviceContext, "WGL_ARB_pixel_format"))
    {
        mGetPixelFormatAttribivARB = GetWGLProcAddress<PFNWGLGETPIXELFORMATATTRIBIVARBPROC>(mOpenGLModule, "wglGetPixelFormatAttribivARB");
    }

    // Destroy the dummy window and context
    wglMakeCurrent(dummyDeviceContext, NULL);
    wglDeleteContext(dummyWGLContext);
    ReleaseDC(dummyWindow, dummyDeviceContext);
    DestroyWindow(dummyWindow);

    // Create the real intermediate context and windows
    mWindow = CreateWindowExA(WS_EX_NOPARENTNOTIFY,
                              reinterpret_cast<const char *>(mWindowClass),
                              "",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    if (!mWindow)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create intermediate OpenGL window.");
    }

    mDeviceContext = GetDC(mWindow);
    if (!mDeviceContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to get the device context of the intermediate OpenGL window.");
    }

    mPixelFormat = ChoosePixelFormat(mDeviceContext, &pixelFormatDescriptor);
    if (mPixelFormat == 0)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not find a compatible pixel format for the intermediate OpenGL window.");
    }

    if (!SetPixelFormat(mDeviceContext, mPixelFormat, &pixelFormatDescriptor))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to set the pixel format on the intermediate OpenGL window.");
    }

    EGLint requestedMajorVersion = attribMap.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
    EGLint requestedMinorVersion = attribMap.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);

    if (mCreateContextAttribsARB)
    {
        int flags = 0;

        // TODO: also allow debug contexts through a user flag
#if !defined(NDEBUG)
        flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

        // TODO: handle robustness

        // TODO: handle core or compatability profile

        std::vector<int> contextCreationAttibutes;

        // Request a specific GL version if the user asked for one
        // TODO: validate the version numbers
        if (requestedMajorVersion != EGL_DONT_CARE)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
            contextCreationAttibutes.push_back(requestedMajorVersion);
        }
        if (requestedMinorVersion != EGL_DONT_CARE)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
            contextCreationAttibutes.push_back(requestedMinorVersion);
        }

        // Set the flag attributes
        if (flags != 0)
        {
            contextCreationAttibutes.push_back(WGL_CONTEXT_FLAGS_ARB);
            contextCreationAttibutes.push_back(flags);
        }

        // Signal the end of the attributes
        contextCreationAttibutes.push_back(0);
        contextCreationAttibutes.push_back(0);

        mWGLContext = mCreateContextAttribsARB(mDeviceContext, NULL, &contextCreationAttibutes[0]);
    }
    else
    {
        // Don't have control over GL versions
        mGLVersionMajor = maxGLVersionMajor;
        mGLVersionMinor = maxGLVersionMinor;

        mWGLContext = wglCreateContext(mDeviceContext);
    }

    if (!mWGLContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to create a WGL context for the intermediate OpenGL window.");
    }

    if (!wglMakeCurrent(mDeviceContext, mWGLContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to make the intermediate WGL context current.");
    }

    const char *versionString = reinterpret_cast<const char*>(getString(GL_VERSION));
    mGLVersionMajor = versionString[0] - '0';
    mGLVersionMinor = versionString[2] - '0';

    return egl::Error(EGL_SUCCESS);
}

void DisplayWGL::terminate()
{
    wglMakeCurrent(mDeviceContext, NULL);
    wglDeleteContext(mWGLContext);
    mWGLContext = NULL;

    ReleaseDC(mWindow, mDeviceContext);
    mDeviceContext = NULL;

    DestroyWindow(mWindow);
    mWindow = NULL;

    UnregisterClassA(reinterpret_cast<const char*>(mWindowClass), NULL);
    mWindowClass = NULL;

    FreeLibrary(mOpenGLModule);
    mGetExtensionStringEXT = NULL;
    mGetExtensionStringARB = NULL;
    mCreateContextAttribsARB = NULL;
    mGetPixelFormatAttribivARB = NULL;
}

SurfaceImpl *DisplayWGL::createWindowSurface(egl::Display *display, const egl::Config *config, EGLNativeWindowType window,
                                             EGLint fixedSize, EGLint width, EGLint height, EGLint postSubBufferSupported)
{
    return new SurfaceWGL(display, config, width, height, fixedSize, postSubBufferSupported, EGL_NO_TEXTURE, EGL_NO_TEXTURE,
                          static_cast<EGLClientBuffer>(0), window, mWindowClass, mPixelFormat, mWGLContext);
}

SurfaceImpl *DisplayWGL::createOffscreenSurface(egl::Display *display, const egl::Config *config, EGLClientBuffer shareHandle,
                                                EGLint width, EGLint height, EGLenum textureFormat, EGLenum textureTarget)
{
    return nullptr;
}

egl::Error DisplayWGL::createContext(const egl::Config *config, const gl::Context *shareContext, const egl::AttributeMap &attribs,
                                     gl::Context **outContext)
{
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::ConfigSet DisplayWGL::generateConfigs() const
{
    egl::ConfigSet configs;

    int minSwapInterval = 1;
    int maxSwapInterval = 1;
    if (isWGLExtensionSupported(mDeviceContext, "WGL_EXT_swap_control"))
    {
        // No defined maximum swap interval in WGL_EXT_swap_control, use a reasonable number
        minSwapInterval = 0;
        maxSwapInterval = 8;
    }

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    DescribePixelFormat(mDeviceContext, mPixelFormat, sizeof(pixelFormatDescriptor), &pixelFormatDescriptor);

    egl::Config config;
    config.renderTargetFormat = GL_NONE; // TODO
    config.depthStencilFormat = GL_NONE; // TODO
    config.bufferSize = pixelFormatDescriptor.cColorBits;
    config.redSize = pixelFormatDescriptor.cRedBits;
    config.greenSize = pixelFormatDescriptor.cGreenBits;
    config.blueSize = pixelFormatDescriptor.cBlueBits;
    config.luminanceSize = 0;
    config.alphaSize = pixelFormatDescriptor.cAlphaBits;
    config.alphaMaskSize = 0;
    config.bindToTextureRGB = EGL_TRUE;
    config.bindToTextureRGBA = EGL_TRUE;
    config.colorBufferType = EGL_RGB_BUFFER;
    config.configCaveat = EGL_NONE;
    config.configID = mPixelFormat;
    config.conformant = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR; // TODO: determine the GL version and what ES versions it supports
    config.depthSize = pixelFormatDescriptor.cDepthBits;
    config.level = 0;
    config.matchNativePixmap = EGL_NONE;
    config.maxPBufferWidth = 0; // TODO
    config.maxPBufferHeight = 0; // TODO
    config.maxPBufferPixels = 0; // TODO
    config.maxSwapInterval = maxSwapInterval;
    config.minSwapInterval = minSwapInterval;
    config.nativeRenderable = EGL_TRUE; // Direct rendering
    config.nativeVisualID = 0;
    config.nativeVisualType = EGL_NONE;
    config.renderableType = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR; // TODO
    config.sampleBuffers = 0; // FIXME: enumerate multi-sampling
    config.samples = 0;
    config.stencilSize = pixelFormatDescriptor.cStencilBits;
    config.surfaceType = ((pixelFormatDescriptor.dwFlags & PFD_DRAW_TO_WINDOW) ? EGL_WINDOW_BIT  : 0) |
                         ((pixelFormatDescriptor.dwFlags & PFD_DRAW_TO_BITMAP) ? EGL_PBUFFER_BIT : 0) |
                         EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
    config.transparentType = EGL_NONE;
    config.transparentRedValue = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue = 0;

    configs.add(config);

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
    return (IsWindow(window) == TRUE);
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
}

bool DisplayWGL::isWGLExtensionSupported(HDC deviceContext, const std::string &name) const
{
    const char *extensions = "";
    if (mGetExtensionStringEXT)
    {
        extensions = mGetExtensionStringEXT();
    }
    else if (mGetExtensionStringARB)
    {
        extensions = mGetExtensionStringARB(deviceContext);
    }

    return strstr(extensions, name.c_str()) != nullptr;
}

}
