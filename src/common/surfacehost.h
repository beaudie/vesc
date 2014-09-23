//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// surfacehost.h: Defines SurfaceHost, a class for managing and performing
// operations on an EGLNativeWindowType. It is used for HWND (Desktop Windows)
// and IInspectable objects (Windows Store Applications).

#ifndef COMMON_SURFACEHOST_H_
#define COMMON_SURFACEHOST_H_

#include <EGL/eglplatform.h>
#include "common/debug.h"
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>

// DXGISwapChain and DXGIFactory are typedef'd to specific required
// types. HWND SurfaceHost requires IDXGISwapChain and IDXGIFactory
// and the Windows Store SurfaceHosts require IDXGISwapChain1 and
// IDXGIFactory2.
typedef IDXGISwapChain DXGISwapChain;
typedef IDXGIFactory DXGIFactory;

namespace rx
{
class SurfaceHost
{
public:
    SurfaceHost(EGLNativeWindowType window);
    ~SurfaceHost();

    // The HWND SurfaceHost implementation can benefit
    // by having inline versions of these methods to 
    // reduce the calling overhead.
    inline bool initialize() { return true; }
    inline bool getClientRect(LPRECT rect) { return !!GetClientRect(mWindow, rect); }
    inline bool isIconic() { return !!IsIconic(mWindow); }

    HRESULT createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, UINT width, UINT height, DXGISwapChain** swapChain);
    inline EGLNativeWindowType getNativeWindowType() const { return mWindow; }

private:
    EGLNativeWindowType mWindow;

};
}

bool isValidEGLNativeWindowType(EGLNativeWindowType window);

#endif // COMMON_SURFACEHOST_H_
