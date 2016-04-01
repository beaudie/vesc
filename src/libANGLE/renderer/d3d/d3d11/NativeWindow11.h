//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow11.h: Defines NativeWindow11, a class for managing and
// performing operations on an EGLNativeWindowType.
// It is used for HWND (Desktop Windows) and IInspectable objects
//(Windows Store Applications).

#ifndef LIBANGLE_RENDERER_D3D_D3D11_NATIVEWINDOW11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_NATIVEWINDOW11_H_

#include "common/debug.h"
#include "common/platform.h"

#include "libANGLE/Config.h"
#include "libANGLE/renderer/d3d/NativeWindowD3D.h"

namespace rx
{

class NativeWindow11 : public NativeWindowD3D
{
  public:
    NativeWindow11(EGLNativeWindowType window) : NativeWindowD3D(window) {}

    virtual HRESULT createSwapChain(ID3D11Device *device,
                                    IDXGIFactory *factory,
                                    DXGI_FORMAT format,
                                    UINT width,
                                    UINT height,
                                    IDXGISwapChain **swapChain) = 0;
    virtual void commitChange() = 0;
};
}

#endif  // LIBANGLE_RENDERER_D3D_D3D11_NATIVEWINDOW11_H_
