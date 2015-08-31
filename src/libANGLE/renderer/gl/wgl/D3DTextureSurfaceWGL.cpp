//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// D3DTextureSurfaceWGL.cpp: WGL implementation of egl::Surface for D3D texture interop.

#include "libANGLE/renderer/gl/wgl/D3DTextureSurfaceWGL.h"

namespace rx
{

D3DTextureSurfaceWGL::D3DTextureSurfaceWGL(RendererGL *renderer, EGLClientBuffer clientBuffer, DisplayWGL *display, const FunctionsWGL *functions)
    : SurfaceGL(renderer),
    mClientBuffer(clientBuffer),
    mDisplay(display),
    mFunctionsWGL(functions)
{

}

D3DTextureSurfaceWGL::~D3DTextureSurfaceWGL()
{

}

template <typename outType>
inline outType *DynamicCastComObject(IUnknown *object)
{
    outType *outObject = nullptr;
    HRESULT result = object->QueryInterface(__uuidof(outType), reinterpret_cast<void**>(&outObject));
    if (SUCCEEDED(result))
    {
        return outObject;
    }
    else
    {
        SafeRelease(outObject);
        return nullptr;
    }
}

egl::Error D3DTextureSurfaceWGL::initialize()
{
    IUnknown* buffer = static_cast<IUnknown*>(mClientBuffer);

    IDirect3DResource9 *resource9 = DynamicCastComObject<IDirect3DResource9>(buffer);
    ID3D11DeviceChild *resource11 = DynamicCastComObject<ID3D11DeviceChild>(buffer);

    if (resource9 != nullptr)
    {
        IDirect3DDevice9 *d3d9Device = nullptr;
        HRESULT result = resource9->GetDevice(&d3d9Device);
        if (FAILED(result))
        {
            return egl::Error(EGL_BAD_PARAMETER, "Could not query the D3D9 device from the client buffer.");
        }
    }
    else if (resource11 != nullptr)
    {
        ID3D11Device *d3d11Device = nullptr;
        resource11->GetDevice(&d3d11Device);
    }


    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::makeCurrent()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::swap()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::bindTexImage(EGLint buffer)
{

    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void D3DTextureSurfaceWGL::setSwapInterval(EGLint interval)
{
    UNIMPLEMENTED();
}

EGLint D3DTextureSurfaceWGL::getWidth() const
{
    UNIMPLEMENTED();
    return 0;
}

EGLint D3DTextureSurfaceWGL::getHeight() const
{
    UNIMPLEMENTED();
    return 0;
}

EGLint D3DTextureSurfaceWGL::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint D3DTextureSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
