//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotator11.cpp: D3D11 helpers for adding trace annotations.
//

#include "libANGLE/renderer/d3d/d3d11/DebugAnnotator11.h"

#include "common/debug.h"

namespace rx
{

DebugAnnotator11::DebugAnnotator11()
    : mInitialized(false),
      mD3d11Module(NULL),
      mUserDefinedAnnotation(NULL)
{
    // D3D11 devices can't be created during DllMain.
    // We defer device creation until the object is actually used.
}

DebugAnnotator11::~DebugAnnotator11()
{
    if (mInitialized)
    {
        SafeRelease(mUserDefinedAnnotation);
        FreeLibrary(mD3d11Module);
    }
}

void DebugAnnotator11::beginEvent(const std::wstring &eventName)
{
    initializeDevice();

    mUserDefinedAnnotation->BeginEvent(eventName.c_str());
}

void DebugAnnotator11::endEvent()
{
    initializeDevice();

    mUserDefinedAnnotation->EndEvent();
}

void DebugAnnotator11::setMarker(const std::wstring &markerName)
{
    initializeDevice();

    mUserDefinedAnnotation->SetMarker(markerName.c_str());
}

bool DebugAnnotator11::getStatus()
{
    // ID3DUserDefinedAnnotation::GetStatus doesn't work with the Graphics Diagnostics tools in Visual Studio 2013.

#if defined(_DEBUG) && defined(ANGLE_ENABLE_WINDOWS_STORE)
    // In the Windows Store, we can use IDXGraphicsAnalysis. The call to GetDebugInterface1 only succeeds if the app is under capture.
    // This should only be called in DEBUG mode.
    // If an app links against DXGIGetDebugInterface1 in release mode then it will fail Windows Store ingestion checks.
    IDXGraphicsAnalysis* graphicsAnalysis;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&graphicsAnalysis));
    bool underCapture = (graphicsAnalysis != NULL);
    SafeRelease(graphicsAnalysis);
    return underCapture;
#endif

    // Otherwise, we have to return true here.
    return true;
}

void DebugAnnotator11::initializeDevice()
{
    if (!mInitialized)
    {
#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
        mD3d11Module = LoadLibrary(TEXT("d3d11.dll"));
        ASSERT(mD3d11Module);

        PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(mD3d11Module, "D3D11CreateDevice");
        ASSERT(D3D11CreateDevice != NULL);
#endif // !ANGLE_ENABLE_WINDOWS_STORE

        ID3D11Device* device = NULL;
        ID3D11DeviceContext* context = NULL;

        HRESULT hr = E_FAIL;

        // Create a D3D_DRIVER_TYPE_NULL device, which is much cheaper than other types of device.
        hr =  D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &device, NULL, &context);
        ASSERT(SUCCEEDED(hr));

        hr = context->QueryInterface(__uuidof(mUserDefinedAnnotation), reinterpret_cast<void**>(&mUserDefinedAnnotation));
        ASSERT(SUCCEEDED(hr) && mUserDefinedAnnotation != NULL);

        SafeRelease(device);
        SafeRelease(context);

        mInitialized = true;
    }
}

}
