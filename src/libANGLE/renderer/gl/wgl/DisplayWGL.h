//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_

#include "libANGLE/renderer/gl/DisplayGL.h"

#include <thread>
#include <unordered_map>

#include <GL/wglext.h>

namespace rx
{

class FunctionsWGL;
class RendererWGL;

class DisplayWGL : public DisplayGL
{
  public:
    DisplayWGL(const egl::DisplayState &state);
    ~DisplayWGL() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    // Surface creation
    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::SurfaceState &state,
                                      const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                               EGLenum buftype,
                                               EGLClientBuffer clientBuffer,
                                               const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPixmapSurface(const egl::SurfaceState &state,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override;

    ContextImpl *createContext(const gl::ContextState &state) override;

    egl::ConfigSet generateConfigs() override;

    bool testDeviceLost() override;
    egl::Error restoreLostDevice(const egl::Display *display) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;
    egl::Error validateClientBuffer(const egl::Config *configuration,
                                    EGLenum buftype,
                                    EGLClientBuffer clientBuffer,
                                    const egl::AttributeMap &attribs) const override;

    DeviceImpl *createDevice() override;

    std::string getVendorString() const override;

    egl::Error waitClient(const gl::Context *context) const override;
    egl::Error waitNative(const gl::Context *context, EGLint engine) const override;

    egl::Error makeCurrent(egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    egl::Error registerD3DDevice(IUnknown *device, HANDLE *outHandle);
    void releaseD3DDevice(HANDLE handle);

    gl::Version getMaxSupportedESVersion() const override;

    void destroyNativeContext(HGLRC context);

  private:
    egl::Error createRenderer(FunctionsWGL *functionsWGL,
                              HDC deviceContext,
                              HGLRC shareContext,
                              const egl::AttributeMap &displayAttributes,
                              bool makeNewContextCurrent,
                              std::shared_ptr<RendererWGL> *outRenderer);

    egl::Error initializeD3DDevice();

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    std::shared_ptr<RendererWGL> mRenderer;

    struct CurrentData
    {
        HGLRC glRC = nullptr;
        HDC drawDC = nullptr;
        HDC readDC = nullptr;
    };
    std::unordered_map<std::thread::id, CurrentData> mCurrentData;

    HMODULE mOpenGLModule;

    FunctionsWGL *mFunctionsWGL;

    bool mHasWGLCreateContextRobustness;
    bool mHasRobustness;

    ATOM mWindowClass;
    HWND mWindow;
    HDC mDeviceContext;
    int mPixelFormat;

    bool mUseDXGISwapChains;
    bool mHasDXInterop;
    HMODULE mDxgiModule;
    HMODULE mD3d11Module;
    HANDLE mD3D11DeviceHandle;
    ID3D11Device *mD3D11Device;

    struct D3DObjectHandle
    {
        HANDLE handle;
        size_t refCount;
    };
    std::map<IUnknown *, D3DObjectHandle> mRegisteredD3DDevices;

    egl::Display *mDisplay;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_DISPLAYWGL_H_
