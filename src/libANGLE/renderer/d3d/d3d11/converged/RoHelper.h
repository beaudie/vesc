//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RoHelper.h: Helper class to expose WinRT activation without statically linking windowapp.lib thus maintaining win7 compatibility

#ifndef LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_ROHELPER_H_
#define LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_ROHELPER_H_

namespace rx
{
class RoHelper
{
  public:
    RoHelper() : mWinRtAvailable(false)
    {
        if (!IsWindows10OrGreater())
        {
            return;
        }

        mComBaseModule = LoadLibraryA("ComBase.dll");

        if (mComBaseModule == nullptr)
        {
            return;
        }

        auto temp = GetProcAddress(mComBaseModule, "WindowsCreateStringReference");
        if (temp != nullptr)
        {
            mFpWindowsCreateStringReference =
                reinterpret_cast<_WindowsCreateStringReference *>(temp);

            temp = GetProcAddress(mComBaseModule, "RoGetActivationFactory");
            if (temp != nullptr)
            {
                mFpGetActivationFactory = reinterpret_cast<_GetActivationFactory *>(temp);

                temp = GetProcAddress(mComBaseModule, "WindowsCompareStringOrdinal");
                if (temp != nullptr)
                {
                    mFpWindowsCompareStringOrdinal =
                        reinterpret_cast<_WindowsCompareStringOrginal *>(temp);

                    temp = GetProcAddress(mComBaseModule, "WindowsDeleteString");
                    if (temp != nullptr)
                    {
                        mFpWindowsDeleteString = reinterpret_cast<_WindowsDeleteString *>(temp);

                        temp = GetProcAddress(mComBaseModule, "RoInitialize");
                        if (temp != nullptr)
                        {
                            mFpRoInitialize = reinterpret_cast<_RoInitialize *>(temp);

                            temp = GetProcAddress(mComBaseModule, "RoUninitialize");
                            if (temp != nullptr)
                            {
                                mFpRoUninitialize = reinterpret_cast<_RoUninitialize *>(temp);

                                mCoreMessagingModule = LoadLibraryA("coremessaging.dll");

                                if (mCoreMessagingModule == nullptr)
                                {
                                    return;
                                }

                                temp = GetProcAddress(mCoreMessagingModule,
                                                      "CreateDispatcherQueueController");

                                if (temp != nullptr)
                                {
                                    mFpCreateDispatcherQueueController =
                                        reinterpret_cast<_CreateDispatcherQueueController *>(temp);

                                    if (SUCCEEDED(RoInitialize(RO_INIT_MULTITHREADED)))
                                    {
                                        mWinRtAvailable = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ~RoHelper()
    {
        RoUninitialize();
        if (mCoreMessagingModule != nullptr)
        {
            FreeLibrary(mCoreMessagingModule);
            mCoreMessagingModule = nullptr;
        }

        if (mComBaseModule != nullptr)
        {
            FreeLibrary(mComBaseModule);
            mComBaseModule = nullptr;
        }
    }

    bool WinRtAvailable() const { return mWinRtAvailable; }

    bool SupportedWindowsRelease()
    {
        if (!mWinRtAvailable)
        {
            return false;
        }

        HSTRING className, contractName;
        HSTRING_HEADER classNameHeader, contractNameHeader;
        boolean isSupported = false;

        HRESULT hr = GetStringReference(RuntimeClass_Windows_Foundation_Metadata_ApiInformation,
                                        &className, &classNameHeader);

        if (FAILED(hr))
        {
            return isSupported;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Metadata::IApiInformationStatics> api;

        hr = GetActivationFactory(
            className, __uuidof(ABI::Windows::Foundation::Metadata::IApiInformationStatics), &api);

        if (FAILED(hr))
        {
            return isSupported;
        }

        hr = GetStringReference(L"Windows.Foundation.UniversalApiContract", &contractName,
                                &contractNameHeader);
        if (FAILED(hr))
        {
            return isSupported;
        }

        api->IsApiContractPresentByMajor(contractName, 6, &isSupported);

        return isSupported;
    }

    HRESULT GetStringReference(PCWSTR source, HSTRING *act, HSTRING_HEADER *header)
    {
        if (!mWinRtAvailable)
        {
            return E_FAIL;
        }

        const wchar_t *str = static_cast<const wchar_t *>(source);

        unsigned int length;
        HRESULT hr = SizeTToUInt32(::wcslen(str), &length);
        if (FAILED(hr))
        {
            return hr;
        }

        return mFpWindowsCreateStringReference(source, length, header, act);
    }

    HRESULT GetActivationFactory(const HSTRING act, const IID &interfaceId, void **fac)
    {
        if (!mWinRtAvailable)
        {
            return E_FAIL;
        }
        auto hr = mFpGetActivationFactory(act, interfaceId, fac);
        return hr;
    }

    HRESULT WindowsCompareStringOrdinal(HSTRING one, HSTRING two, int *result)
    {
        if (!mWinRtAvailable)
        {
            return E_FAIL;
        }
        return mFpWindowsCompareStringOrdinal(one, two, result);
    }

    HRESULT CreateDispatcherQueueController(
        DispatcherQueueOptions options,
        ABI::Windows::System::IDispatcherQueueController **dispatcherQueueController)
    {
        if (!mWinRtAvailable)
        {
            return E_FAIL;
        }
        return mFpCreateDispatcherQueueController(options, dispatcherQueueController);
    }

    HRESULT WindowsDeleteString(HSTRING one)
    {
        if (!mWinRtAvailable)
        {
            return E_FAIL;
        }
        return mFpWindowsDeleteString(one);
    }

    HRESULT RoInitialize(RO_INIT_TYPE type) { return mFpRoInitialize(type); }

    void RoUninitialize() { mFpRoUninitialize(); }

  private:
    using _WindowsCreateStringReference = HRESULT __stdcall(PCWSTR,
                                                            UINT32,
                                                            HSTRING_HEADER *,
                                                            HSTRING *);

    using _GetActivationFactory = HRESULT __stdcall(HSTRING, REFIID, void **);

    using _WindowsCompareStringOrginal = HRESULT __stdcall(HSTRING, HSTRING, int *);

    using _WindowsDeleteString = HRESULT __stdcall(HSTRING);

    using _CreateDispatcherQueueController =
        HRESULT __stdcall(DispatcherQueueOptions,
                          ABI::Windows::System::IDispatcherQueueController **);

    using _RoInitialize   = HRESULT __stdcall(RO_INIT_TYPE);
    using _RoUninitialize = void __stdcall();

    _WindowsCreateStringReference *mFpWindowsCreateStringReference;
    _GetActivationFactory *mFpGetActivationFactory;
    _WindowsCompareStringOrginal *mFpWindowsCompareStringOrdinal;
    _CreateDispatcherQueueController *mFpCreateDispatcherQueueController;
    _WindowsDeleteString *mFpWindowsDeleteString;
    _RoInitialize *mFpRoInitialize;
    _RoUninitialize *mFpRoUninitialize;

    bool mWinRtAvailable;

    HMODULE mComBaseModule;
    HMODULE mCoreMessagingModule;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_ROHELPER_H_