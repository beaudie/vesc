//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#include "libANGLE/Config.h"

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

egl::Error DisplayWGL::initialize(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attribMap)
{
    mDC = nativeDisplay;

    mOpenGLModule = LoadLibraryA("opengl32.dll");
    if (!mOpenGLModule)
    {
        return egl::Error(EGL_NOT_INITIALIZED);
    }

    mGetExtensionStringEXT = GetWGLProcAddress<PFNWGLGETEXTENSIONSSTRINGEXTPROC>(mOpenGLModule, "wglGetExtensionsStringEXT");
    mGetExtensionStringARB = GetWGLProcAddress<PFNWGLGETEXTENSIONSSTRINGARBPROC>(mOpenGLModule, "wglGetExtensionsStringARB");
    if (!mGetExtensionStringEXT || !mGetExtensionStringARB)
    {
        return egl::Error(EGL_NOT_INITIALIZED);
    }

    if (isWGLExtensionSupported("WGL_ARB_create_context"))
    {
        mCreateContextAttribsARB = GetWGLProcAddress<PFNWGLCREATECONTEXTATTRIBSARBPROC>(mOpenGLModule, "wglCreateContextAttribsARB");
    }

    if (isWGLExtensionSupported("WGL_ARB_pixel_format"))
    {
        mGetPixelFormatAttribivARB = GetWGLProcAddress<PFNWGLGETPIXELFORMATATTRIBIVARBPROC>(mOpenGLModule, "wglGetPixelFormatAttribivARB");
    }

    return egl::Error(EGL_SUCCESS);
}

void DisplayWGL::terminate()
{
    FreeLibrary(mOpenGLModule);
    mGetExtensionStringEXT = nullptr;
    mGetExtensionStringARB = nullptr;
    mCreateContextAttribsARB = nullptr;
}

SurfaceImpl *DisplayWGL::createWindowSurface(egl::Display *display, const egl::Config *config, EGLNativeWindowType window,
                                             EGLint fixedSize, EGLint width, EGLint height, EGLint postSubBufferSupported)
{
    return nullptr;
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

static int GetPixelFormatAttribute(PFNWGLGETPIXELFORMATATTRIBIVARBPROC func, HDC dc, int pixelFormat, int attribute)
{
    int result = 0;
    if (!func(dc, pixelFormat, 0, 1, &attribute, &result))
    {
        return 0;
    }
    return result;
}

egl::ConfigSet DisplayWGL::generateConfigs() const
{
    egl::ConfigSet configs;

    int minSwapInterval = 1;
    int maxSwapInterval = 1;
    if (isWGLExtensionSupported("WGL_EXT_swap_control"))
    {
        // No defined maximum swap interval in WGL_EXT_swap_control, use a reasonable number
        minSwapInterval = 0;
        maxSwapInterval = 4;
    }

    if (mGetPixelFormatAttribivARB)
    {
        size_t configCount = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, 1, WGL_NUMBER_PIXEL_FORMATS_ARB);
        for (size_t i = 0; i < configCount; i++)
        {
            // Pixel formats start with 1
            int pixelFormat = i + 1;

            if (!GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_SUPPORT_OPENGL_ARB) ||
                !GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_DRAW_TO_WINDOW_ARB) ||
                 GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_DRAW_TO_WINDOW_ARB) != WGL_TYPE_RGBA_ARB)
            {
                continue;
            }

            egl::Config config;
            config.renderTargetFormat = GL_NONE; // TODO
            config.depthStencilFormat = GL_NONE; // TODO
            config.bufferSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_COLOR_BITS_ARB);
            config.redSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_RED_BITS_ARB);
            config.greenSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_GREEN_BITS_ARB);
            config.blueSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_BLUE_BITS_ARB);
            config.luminanceSize = 0;
            config.alphaSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_ALPHA_BITS_ARB);
            config.alphaMaskSize = 0;
            config.bindToTextureRGB = EGL_TRUE;
            config.bindToTextureRGBA = EGL_TRUE;
            config.colorBufferType = EGL_RGB_BUFFER;
            config.configCaveat = (GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB) ? EGL_SLOW_CONFIG : EGL_NONE;
            config.configID = pixelFormat;
            config.conformant = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR; // TODO: determine the GL version and what ES versions it supports
            config.depthSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_DEPTH_BITS_ARB);
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
            config.stencilSize = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_STENCIL_BITS_ARB);
            config.surfaceType = (GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_DRAW_TO_WINDOW_ARB) ? EGL_WINDOW_BIT : 0) |
                                 (GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_DRAW_TO_BITMAP_ARB) ? EGL_PBUFFER_BIT : 0) |
                                 EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
            config.transparentType = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_TRANSPARENT_ARB) ? EGL_TRANSPARENT_RGB : EGL_NONE;
            config.transparentRedValue = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_TRANSPARENT_RED_VALUE_ARB);
            config.transparentGreenValue = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_TRANSPARENT_GREEN_VALUE_ARB);
            config.transparentBlueValue = GetPixelFormatAttribute(mGetPixelFormatAttribivARB, mDC, pixelFormat, WGL_TRANSPARENT_BLUE_VALUE_ARB);

            configs.add(config);
        }
    }
    else
    {
        //size_t configCount = DescribePixelFormat(mDC, 1, sizeof(PIXELFORMATDESCRIPTOR), nullptr);
        // TODO
    }

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

bool DisplayWGL::isWGLExtensionSupported(const std::string &name) const
{
    const char *extensions = nullptr;
    if (mGetExtensionStringEXT)
    {
        extensions = mGetExtensionStringEXT();
    }
    else
    {
        ASSERT(mGetExtensionStringARB);
        extensions = mGetExtensionStringARB(mDC);
    }

    return strstr(extensions, name.c_str()) != nullptr;
}

}
