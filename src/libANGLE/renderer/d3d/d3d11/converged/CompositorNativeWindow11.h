//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompositorNativeWindow11.h: Implementation of NativeWindow11 using Windows.UI.Composition APIs
// which work in both Win32 and WinRT contexts.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_

#include "libANGLE/renderer/d3d/d3d11/NativeWindow11.h"

#include <DispatcherQueue.h>
#include <Windows.ui.composition.interop.h>
#include <windows.foundation.metadata.h>
#include <windows.ui.composition.h>
#include <wrl.h>
#include <VersionHelpers.h>
#include "RoHelper.h"

namespace rx
{


class CompositorNativeWindow11 : public NativeWindow11
{
  public:
    CompositorNativeWindow11(EGLNativeWindowType window, bool hasAlpha);
    ~CompositorNativeWindow11() override = default;

    bool initialize() override;
    bool getClientRect(LPRECT rect) const override;
    bool isIconic() const override;

    HRESULT createSwapChain(ID3D11Device *device,
                            IDXGIFactory *factory,
                            DXGI_FORMAT format,
                            UINT width,
                            UINT height,
                            UINT samples,
                            IDXGISwapChain **swapChain) override;

    void commitChange() override;

    static bool IsValidNativeWindow(EGLNativeWindowType window);

    static bool IsSupportedWinRelease();

  private:
    static bool IsSpriteVisual(EGLNativeWindowType window);

    bool mHasAlpha;

    RoHelper mRoHelper;

    // Namespace prefix required here for some reason despite using namespace
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ISpriteVisual> mHostVisual;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionBrush> mCompositionBrush;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionSurface> mSurface;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionSurfaceBrush> mSurfaceBrush;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_