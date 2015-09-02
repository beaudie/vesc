//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WinrtWindow.cpp: Implementation of OSWindow for WinRT (Windows)

#include "windows/winrt/WinrtWindow.h"

#include <wrl.h>
#include <windows.applicationmodel.core.h>

#include "angle_windowsstore.h"
#include "common/debug.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

WinrtWindow::~WinrtWindow()
{
    destroy();
}

bool WinrtWindow::initialize(const std::string &name, size_t width, size_t height)
{
    ComPtr<ICoreWindowStatic> coreWindowStatic;
    ComPtr<IActivationFactory> propertySetFactory;
    ComPtr<IPropertyValueStatics> propertyValueStatics;

    ComPtr<ICoreWindow> coreWindow;
    ComPtr<IMap<HSTRING, IInspectable *>> nativeWindowAsMap;
    ComPtr<IInspectable> sizeValue;

    HRESULT result           = S_OK;
    boolean propertyReplaced = false;

    destroy();

    // Get all the relevant activation factories
    result = Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(), &coreWindowStatic);
    if (FAILED(result))
    {
        return false;
    }

    result = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Foundation_Collections_PropertySet).Get(),
        &propertySetFactory);
    if (FAILED(result))
    {
        return false;
    }

    result =
        GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(),
                             &propertyValueStatics);
    if (FAILED(result))
    {
        return false;
    }

    // Create a PropertySet to be used as the native window
    result = propertySetFactory->ActivateInstance(&mNativeWindow);
    if (FAILED(result))
    {
        return false;
    }

    // Reinterpret the PropertySet as a map, so we can Insert things into it later
    ComPtr<IInspectable> tempNativeWindow = mNativeWindow;
    result = tempNativeWindow.As(&nativeWindowAsMap);
    if (FAILED(result))
    {
        return false;
    }

    // Then get the CoreWindow for the current thread
    result = coreWindowStatic->GetForCurrentThread(&coreWindow);
    if (FAILED(result))
    {
        return false;
    }

    // Store away the CoreWindow's dispatcher, to be used later to process messages
    result = coreWindow->get_Dispatcher(&mCoreDispatcher);
    if (FAILED(result))
    {
        return false;
    }

    // Add the CoreWindow to the PropertySet
    result = nativeWindowAsMap->Insert(HStringReference(EGLNativeWindowTypeProperty).Get(),
                                       coreWindow.Get(), &propertyReplaced);
    if (FAILED(result))
    {
        return false;
    }

    // Create a Size to represent the Native Window's size
    ABI::Windows::Foundation::Size renderSize;
    renderSize.Width  = static_cast<float>(width);
    renderSize.Height = static_cast<float>(height);
    result = propertyValueStatics->CreateSize(renderSize, sizeValue.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    // Add the Size to the PropertySet
    result = nativeWindowAsMap->Insert(HStringReference(EGLRenderSurfaceSizeProperty).Get(),
                                       sizeValue.Get(), &propertyReplaced);
    if (FAILED(result))
    {
        return false;
    }

    return true;
};

void WinrtWindow::destroy()
{
    if (mNativeWindow != nullptr)
    {
        SafeRelease(mNativeWindow);
        mNativeWindow = nullptr;
    }

    if (mCoreDispatcher != nullptr)
    {
        SafeRelease(mCoreDispatcher);
        mCoreDispatcher = nullptr;
    }
}

EGLNativeWindowType WinrtWindow::getNativeWindow() const
{
    return mNativeWindow;
}

EGLNativeDisplayType WinrtWindow::getNativeDisplay() const
{
    UNIMPLEMENTED();
    return static_cast<EGLNativeDisplayType>(0);
}

void WinrtWindow::messageLoop()
{
    HRESULT result = mCoreDispatcher->ProcessEvents(CoreProcessEventsOption_ProcessAllIfPresent);
    UNUSED_ASSERTION_VARIABLE(result);
    ASSERT(SUCCEEDED(result));
}

void WinrtWindow::setMousePosition(int /* x */, int /* y */)
{
    UNIMPLEMENTED();
}

bool WinrtWindow::setPosition(int /* x */, int /* y */)
{
    UNIMPLEMENTED();
    return false;
}

bool WinrtWindow::resize(int /* width */, int /* height */)
{
    UNIMPLEMENTED();
    return false;
}

void WinrtWindow::setVisible(bool isVisible)
{
    if (isVisible)
    {
        // Already visible by default
        return;
    }
    else
    {
        // Not implemented in WinRT
        UNIMPLEMENTED();
    }
}

void WinrtWindow::signalTestEvent()
{
    UNIMPLEMENTED();
}

OSWindow *CreateOSWindow()
{
    return new WinrtWindow();
}