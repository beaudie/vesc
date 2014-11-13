//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.h: D3D implementation of an EGL surface

#ifndef LIBEGL_SURFACED3D_H_
#define LIBEGL_SURFACED3D_H_

#include "common/NativeWindow.h"
#include "libEGL/SurfaceImpl.h"

namespace rx
{
class SwapChain;
class RendererD3D;
}

namespace egl
{
class Surface;

class SurfaceD3D : public SurfaceImpl
{
  public:
    static SurfaceD3D *createWindow(Display *display, const egl::Config *config,
                                    EGLNativeWindowType window, EGLint fixedSize,
                                    EGLint width, EGLint height, EGLint postSubBufferSupported);
    static SurfaceD3D *createOffscreen(Display *display, const egl::Config *config,
                                       EGLClientBuffer shareHandle, EGLint width, EGLint height,
                                       EGLenum textureFormat, EGLenum textureTarget);
    ~SurfaceD3D() override;

    static SurfaceD3D *makeSurfaceD3D(SurfaceImpl *impl);
    static SurfaceD3D *makeSurfaceD3D(Surface *surface);

    Error initialize() override;
    Error swap() override;
    Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    Error bindTexImage(EGLint buffer) override;
    Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    // D3D implementations (some virtual to hack across DLL boundaries)
    virtual rx::SwapChain *getSwapChain() const;

    void release();
    Error resetSwapChain();

    // Returns true if swapchain changed due to resize or interval update
    bool checkForOutOfDateSwapChain();

    EGLNativeWindowType getWindowHandle() const override;

  private:
    DISALLOW_COPY_AND_ASSIGN(SurfaceD3D);

    SurfaceD3D(Display *display, const Config *config, EGLint width, EGLint height,
               EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
               EGLenum textureType, EGLClientBuffer shareHandle, EGLNativeWindowType window);
    Error swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
    Error resetSwapChain(int backbufferWidth, int backbufferHeight);
    Error resizeSwapChain(int backbufferWidth, int backbufferHeight);

    void subclassWindow();
    void unsubclassWindow();

    rx::RendererD3D *mRenderer;

    rx::SwapChain *mSwapChain;
    bool mSwapIntervalDirty;
    bool mWindowSubclassed;        // Indicates whether we successfully subclassed mWindow for WM_RESIZE hooking

    rx::NativeWindow mNativeWindow;   // Handler for the Window that the surface is created for.
};


}

#endif // LIBEGL_SURFACED3D_H_
