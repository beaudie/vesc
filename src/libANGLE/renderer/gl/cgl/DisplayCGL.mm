//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayCGL.mm: CGL implementation of egl::Display

#include "libANGLE/renderer/gl/CGL/DisplayCGL.h"

#import <Cocoa/Cocoa.h>
#include <dlfcn.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "libANGLE/renderer/gl/CGL/PbufferSurfaceCGL.h"
#include "libANGLE/renderer/gl/CGL/WindowSurfaceCGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace
{

const char *kDefaultOpenGLDylibName =
    "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib";
const char *kFallbackOpenGLDylibName = "GL";

}

namespace rx
{

class FunctionsGLCGL : public FunctionsGL
{
  public:
    FunctionsGLCGL(void *dylibHandle) : mDylibHandle(dylibHandle) {}

    virtual ~FunctionsGLCGL() { dlclose(mDylibHandle); }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return dlsym(mDylibHandle, function.c_str());
    }

    void *mDylibHandle;
};

DisplayCGL::DisplayCGL() : DisplayGL(), mEGLDisplay(nullptr), mGLLibHandle(nullptr), mFunctions(nullptr), mContext(nullptr)
{
}

DisplayCGL::~DisplayCGL()
{
}

egl::Error DisplayCGL::initialize(egl::Display *display)
{
    mEGLDisplay = display;

    // TODO(cwallez) investigate which pixel formats we want
    CGLPixelFormatObj pixelFormat;
    {
        CGLPixelFormatAttribute attribs[] = {
            kCGLPFAOpenGLProfile, static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core),
            static_cast<CGLPixelFormatAttribute>(0)};
        GLint nVirtualScreens = 0;
        CGLChoosePixelFormat(attribs, &pixelFormat, &nVirtualScreens);
        if (pixelFormat != nullptr)
        {
            mContextPixelFormats.push_back(pixelFormat);
        }
    }
    {
        CGLPixelFormatAttribute attribs[] = {
            kCGLPFAOpenGLProfile, static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core),
            static_cast<CGLPixelFormatAttribute>(0)};
        GLint nVirtualScreens = 0;
        CGLChoosePixelFormat(attribs, &pixelFormat, &nVirtualScreens);
        if (pixelFormat != nullptr)
        {
            mContextPixelFormats.push_back(pixelFormat);
        }
    }

    if (mContextPixelFormats.empty())
    {
        egl::Error(EGL_NOT_INITIALIZED, "Could not create a context pixel format.");
    }

    // There is no equivalent getProcAddress in CGL so we open the dylib directly
    void *mGLLibHandle = dlopen(kDefaultOpenGLDylibName, RTLD_NOW);
    if (!mGLLibHandle)
    {
        mGLLibHandle = dlopen(kFallbackOpenGLDylibName, RTLD_NOW);
    }
    if (!mGLLibHandle)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not open the OpenGL Framework.");
    }

    for (auto pixelFormat : mContextPixelFormats)
    {
        CGLContextObj context = nullptr;
        CGLCreateContext(pixelFormat, nullptr, &context);

        if (context == nullptr)
        {
            continue;
        }

        CGLSetCurrentContext(context);

        FunctionsGL *functions = new FunctionsGLCGL(mGLLibHandle);
        functions->initialize();

        gl::Caps caps;
        gl::TextureCapsMap textureCaps;
        gl::Extensions extensions;
        gl::Version supportedVersion;
        rx::nativegl_gl::GenerateCaps(functions, &caps, &textureCaps, &extensions, &supportedVersion);

        if (supportedVersion >= gl::Version(2, 0))
        {
            mContext = context;
            mFunctions = functions;
            break;
        }

        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(context);
        context = nullptr;

        SafeDelete(functions);
    }
    if (mContext == nullptr || mFunctions == nullptr)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create the CGL context.");
    }

    return DisplayGL::initialize(display);
}

void DisplayCGL::terminate()
{
    DisplayGL::terminate();

    if (mContext != nullptr)
    {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(mContext);
        mContext = nullptr;
    }

    if (mGLLibHandle != nullptr)
    {
        dlclose(mGLLibHandle);
        mGLLibHandle = nullptr;
    }

    for (auto& pixelFormat : mContextPixelFormats)
    {
        CGLReleasePixelFormat(pixelFormat);
        pixelFormat = nullptr;
    }
    mContextPixelFormats.clear();

    SafeDelete(mFunctions);
}

SurfaceImpl *DisplayCGL::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    return new WindowSurfaceCGL(this->getRenderer(), window, mFunctions);
}

SurfaceImpl *DisplayCGL::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    EGLint width  = attribs.get(EGL_WIDTH, 0);
    EGLint height = attribs.get(EGL_HEIGHT, 0);
    return new PbufferSurfaceCGL(this->getRenderer(), width, height, mFunctions);
}

SurfaceImpl* DisplayCGL::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                       EGLClientBuffer shareHandle,
                                                       const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayCGL::createPixmapSurface(const egl::Config *configuration,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::Error DisplayCGL::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::ConfigSet DisplayCGL::generateConfigs() const
{
    // TODO(cwallez): generate more config permutations
    egl::ConfigSet configs;

    egl::Config config;

    // Native stuff
    config.nativeVisualID   = 0;
    config.nativeVisualType = 0;
    config.nativeRenderable = EGL_TRUE;

    // Buffer sizes
    config.redSize     = 8;
    config.greenSize   = 8;
    config.blueSize    = 8;
    config.alphaSize   = 8;
    config.depthSize   = 24;
    config.stencilSize = 8;

    config.colorBufferType = EGL_RGB_BUFFER;
    config.luminanceSize   = 0;
    config.alphaMaskSize   = 0;

    config.bufferSize = config.redSize + config.greenSize + config.blueSize + config.alphaSize;

    config.transparentType = EGL_NONE;

    // Pbuffer
    config.maxPBufferWidth  = 4096;
    config.maxPBufferHeight = 4096;
    config.maxPBufferPixels = 4096 * 4096;

    // Caveat
    config.configCaveat = EGL_NONE;

    // Misc
    config.sampleBuffers     = 0;
    config.samples           = 0;
    config.level             = 0;
    config.bindToTextureRGB  = EGL_FALSE;
    config.bindToTextureRGBA = EGL_FALSE;

    config.surfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;

    config.minSwapInterval = 1;
    config.maxSwapInterval = 1;

    config.renderTargetFormat = GL_RGBA8;
    config.depthStencilFormat = GL_DEPTH24_STENCIL8;

    config.conformant     = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;
    config.renderableType = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;

    config.matchNativePixmap = EGL_NONE;

    configs.add(config);
    return configs;
}

bool DisplayCGL::isDeviceLost() const
{
    // TODO(cwallez) investigate implementing this
    return false;
}

bool DisplayCGL::testDeviceLost()
{
    // TODO(cwallez) investigate implementing this
    return false;
}

egl::Error DisplayCGL::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayCGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    // TODO(cwallez) investigate implementing this
    return true;
}

std::string DisplayCGL::getVendorString() const
{
    // TODO(cwallez) find a useful vendor string
    return "";
}

const FunctionsGL *DisplayCGL::getFunctionsGL() const
{
    return mFunctions;
}

void DisplayCGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContext = true;
}

void DisplayCGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

}
