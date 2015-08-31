//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// D3DTextureSurfaceWGL.cpp: WGL implementation of egl::Surface for D3D texture interop.

#include "libANGLE/renderer/gl/wgl/D3DTextureSurfaceWGL.h"

#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{

D3DTextureSurfaceWGL::D3DTextureSurfaceWGL(RendererGL *renderer, EGLClientBuffer clientBuffer, DisplayWGL *display, const FunctionsWGL *functions)
    : SurfaceGL(renderer),
      mClientBuffer(clientBuffer),
      mDisplay(display),
      mFunctionsWGL(functions),
      mWidth(0),
      mHeight(0),
      mDeviceHandle(nullptr),
      mObject(nullptr),
      mBoundObjectHandle(nullptr)
{
}

D3DTextureSurfaceWGL::~D3DTextureSurfaceWGL()
{
    ASSERT(mBoundObjectHandle == nullptr);
    mObject->Release();
    mDisplay->releaseD3DDevice(mDeviceHandle);
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
    IUnknown* device = nullptr;

    IDirect3DSurface9 *surface9 = DynamicCastComObject<IDirect3DSurface9>(buffer);
    ID3D10Texture2D *texture10 = DynamicCastComObject<ID3D10Texture2D>(buffer);
    ID3D11Texture2D *texture11 = DynamicCastComObject<ID3D11Texture2D>(buffer);

    if (surface9 != nullptr)
    {
        IDirect3DDevice9 *d3d9Device = nullptr;
        HRESULT result = surface9->GetDevice(&d3d9Device);
        if (FAILED(result))
        {
            surface9->Release();
            return egl::Error(EGL_BAD_PARAMETER, "Could not query the D3D9 device from the client buffer.");
        }

        D3DSURFACE_DESC surfaceDesc;
        if (FAILED(surface9->GetDesc(&surfaceDesc)))
        {
            surface9->Release();
            device->Release();
            return egl::Error(EGL_BAD_PARAMETER, "Could not query description of the D3D9 surface.");
        }

        mWidth = surfaceDesc.Width;
        mHeight = surfaceDesc.Height;

        device = d3d9Device;
        mObject = surface9;
    }
    else if (texture10 != nullptr)
    {
        ID3D10Device *d3d10Device = nullptr;
        texture10->GetDevice(&d3d10Device);
        if (d3d10Device == nullptr)
        {
            texture10->Release();
            return egl::Error(EGL_BAD_PARAMETER, "Could not query the D3D10 device from the client buffer.");
        }

        D3D10_TEXTURE2D_DESC textureDesc;
        texture10->GetDesc(&textureDesc);
        mWidth = textureDesc.Width;
        mHeight = textureDesc.Height;

        device = d3d10Device;
        mObject = texture10;
    }
    else if (texture11 != nullptr)
    {
        ID3D11Device *d3d11Device = nullptr;
        texture11->GetDevice(&d3d11Device);
        if (d3d11Device == nullptr)
        {
            texture11->Release();
            return egl::Error(EGL_BAD_PARAMETER, "Could not query the D3D11 device from the client buffer.");
        }

        D3D11_TEXTURE2D_DESC textureDesc;
        texture11->GetDesc(&textureDesc);
        mWidth = textureDesc.Width;
        mHeight = textureDesc.Height;

        device = d3d11Device;
        mObject = texture11;
    }
    else
    {
        return egl::Error(EGL_BAD_PARAMETER, "Provided buffer is not a IDirect3DDevice9 or ID3D11Texture2D.");
    }

    ASSERT(device != nullptr);
    egl::Error error = mDisplay->registerD3DDevice(device, &mDeviceHandle);
    device->Release();
    if (error.isError())
    {
        return error;
    }

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

egl::Error D3DTextureSurfaceWGL::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    ASSERT(mBoundObjectHandle == nullptr);

    const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    GLuint textureID = textureGL->getTextureID();

    mBoundObjectHandle = mFunctionsWGL->dxRegisterObjectNV(mDeviceHandle, mObject, textureID, GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
    if (mBoundObjectHandle == nullptr)
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to register D3D object, error: 0x%08x.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error D3DTextureSurfaceWGL::releaseTexImage(EGLint buffer)
{
    ASSERT(mBoundObjectHandle != nullptr);
    if (!mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mBoundObjectHandle))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to unregister D3D object, error: 0x%08x.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

void D3DTextureSurfaceWGL::setSwapInterval(EGLint interval)
{
    UNIMPLEMENTED();
}

EGLint D3DTextureSurfaceWGL::getWidth() const
{
    return static_cast<EGLint>(mWidth);
}

EGLint D3DTextureSurfaceWGL::getHeight() const
{
    return static_cast<EGLint>(mHeight);
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
