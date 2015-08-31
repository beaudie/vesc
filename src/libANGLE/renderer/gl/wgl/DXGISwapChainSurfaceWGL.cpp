//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DXGISwapChainSurfaceWGL.cpp: WGL implementation of egl::Surface for windows using a DXGI swap-
// chain.

#include "libANGLE/renderer/gl/wgl/DXGISwapChainSurfaceWGL.h"

#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{

DXGISwapChainSurfaceWGL::DXGISwapChainSurfaceWGL(RendererGL *renderer,
                            EGLNativeWindowType window,
                            ID3D11Device* device,
                            HANDLE deviceHandle,
                            HGLRC wglContext,
                            HDC deviceContext,
                            const FunctionsGL *functionsGL,
                            const FunctionsWGL *functionsWGL)
    : SurfaceGL(renderer),
      mWindow(window),
      mStateManager(renderer->getStateManager()),
      mWorkarounds(renderer->getWorkarounds()),
      mFunctionsGL(functionsGL),
      mFunctionsWGL(functionsWGL),
      mDevice(device),
      mDeviceHandle(deviceHandle),
      mWGLDevice(deviceContext),
      mWGLContext(wglContext),
      mSwapChain(nullptr),
      mRenderbufferID(0),
      mRenderbufferBufferHandle(nullptr),
      mFramebufferID(0),
      mWidth(0),
      mHeight(0),
      mSwapInterval(1)
{
}

DXGISwapChainSurfaceWGL::~DXGISwapChainSurfaceWGL()
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

IDXGIFactory *GetDXGIFactory(ID3D11Device *device)
{
    IDXGIDevice *dxgiDevice = nullptr;
    HRESULT result = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(result))
    {
        return nullptr;
    }

    IDXGIAdapter *dxgiAdapter = nullptr;
    result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
    if (FAILED(result))
    {
        return nullptr;
    }

    IDXGIFactory *dxgiFactory = nullptr;
    result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
    if (FAILED(result))
    {
        return nullptr;
    }

    return dxgiFactory;
}

egl::Error DXGISwapChainSurfaceWGL::initialize()
{
    RECT rect;
    if (!GetClientRect(mWindow, &rect))
    {
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to query the window size.");
    }
    mWidth = rect.right - rect.left;
    mHeight = rect.bottom - rect.top;

    IDXGIFactory *dxgiFactory = GetDXGIFactory(mDevice);
    if (dxgiFactory == nullptr)
    {
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to query the DXGIFactory.");
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.Width = static_cast<UINT>(mWidth);
    swapChainDesc.BufferDesc.Height = static_cast<UINT>(mHeight);
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.Flags = 0;
    swapChainDesc.OutputWindow = mWindow;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT result = dxgiFactory->CreateSwapChain(mDevice, &swapChainDesc, &mSwapChain);
    if (FAILED(result))
    {
        return egl::Error(EGL_BAD_ALLOC, "Failed to create swap chain for window, result: 0x%X", result);
    }

    ID3D11Texture2D *colorBuffer = nullptr;
    result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&colorBuffer));
    if (FAILED(result))
    {
        return egl::Error(EGL_BAD_ALLOC, "Failed to query texture from swap chain, result: 0x%X", result);
    }

    mFunctionsGL->genRenderbuffers(1, &mRenderbufferID);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
    mRenderbufferBufferHandle = mFunctionsWGL->dxRegisterObjectNV(mDeviceHandle, colorBuffer, mRenderbufferID, GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);
    if (mRenderbufferBufferHandle == nullptr)
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to register D3D object, error: 0x%X.", HRESULT_CODE(error));
    }

    mFunctionsGL->genFramebuffers(1, &mFramebufferID);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderbufferID);

    if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, 1, &mRenderbufferBufferHandle))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to lock D3D object, error: 0x%X.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::makeCurrent()
{
    if (!mFunctionsWGL->makeCurrent(mWGLDevice, mWGLContext))
    {
        // TODO: What error type here?
        return egl::Error(EGL_CONTEXT_LOST, "Failed to make the WGL context current.");
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::swap()
{
    if (!mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, 1, &mRenderbufferBufferHandle))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to unlock D3D object, error: 0x%X.", HRESULT_CODE(error));
    }

    HRESULT result = mSwapChain->Present(mSwapInterval, 0);

    if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, 1, &mRenderbufferBufferHandle))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to lock D3D object, error: 0x%X.", HRESULT_CODE(error));
    }

    if (FAILED(result))
    {
        return egl::Error(EGL_BAD_ALLOC, "Failed to present swap chain, result: 0x%X", result);
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error DXGISwapChainSurfaceWGL::releaseTexImage(EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

void DXGISwapChainSurfaceWGL::setSwapInterval(EGLint interval)
{
    mSwapInterval = interval;
}

EGLint DXGISwapChainSurfaceWGL::getWidth() const
{
    return static_cast<EGLint>(mWidth);
}

EGLint DXGISwapChainSurfaceWGL::getHeight() const
{
    return static_cast<EGLint>(mHeight);
}

EGLint DXGISwapChainSurfaceWGL::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint DXGISwapChainSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

FramebufferImpl *DXGISwapChainSurfaceWGL::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(mFramebufferID, data, mFunctionsGL, mWorkarounds, mStateManager);
}


}
