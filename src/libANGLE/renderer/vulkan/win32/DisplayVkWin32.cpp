//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWin32.cpp:
//    Implements the class methods for DisplayVkWin32.
//

#include "libANGLE/renderer/vulkan/win32/DisplayVkWin32.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

#include <vulkan/vulkan.h>
#include <windows.h>

#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/win32/WindowSurfaceVkWin32.h"

namespace rx
{

DisplayVkWin32::DisplayVkWin32(const egl::DisplayState &state) : DisplayVk(state) {}

DisplayVkWin32::~DisplayVkWin32() {}

void DisplayVkWin32::terminate()
{
    if (mWindowClass)
    {
        if (!UnregisterClassA(reinterpret_cast<const char *>(mWindowClass),
                              GetModuleHandle(nullptr)))
        {
            WARN() << "Failed to unregister ANGLE window class: " << gl::FmtHex(mWindowClass);
        }
        mWindowClass = NULL;
    }

    DisplayVk::terminate();
}

bool DisplayVkWin32::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (IsWindow(window) == TRUE);
}

SurfaceImpl *DisplayVkWin32::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                   EGLNativeWindowType window)
{
    return new WindowSurfaceVkWin32(state, window);
}

egl::Error DisplayVkWin32::initialize(egl::Display *display)
{
    ANGLE_TRY(DisplayVk::initialize(display));

    HINSTANCE hinstance = GetModuleHandle(nullptr);

    std::ostringstream stream;
    stream << "ANGLE DisplayWGL " << gl::FmtHex(display) << " Intermediate Window Class";

    const LPSTR idcArrow  = MAKEINTRESOURCEA(32512);
    std::string className = stream.str();
    WNDCLASSA wndClass;
    if (!GetClassInfoA(hinstance, className.c_str(), &wndClass))
    {
        WNDCLASSA intermediateClassDesc     = {};
        intermediateClassDesc.style         = CS_OWNDC;
        intermediateClassDesc.lpfnWndProc   = DefWindowProcA;
        intermediateClassDesc.cbClsExtra    = 0;
        intermediateClassDesc.cbWndExtra    = 0;
        intermediateClassDesc.hInstance     = GetModuleHandle(nullptr);
        intermediateClassDesc.hIcon         = nullptr;
        intermediateClassDesc.hCursor       = LoadCursorA(nullptr, idcArrow);
        intermediateClassDesc.hbrBackground = 0;
        intermediateClassDesc.lpszMenuName  = nullptr;
        intermediateClassDesc.lpszClassName = className.c_str();
        mWindowClass                        = RegisterClassA(&intermediateClassDesc);
        if (!mWindowClass)
        {
            return egl::EglNotInitialized()
                   << "Failed to register intermediate OpenGL window class \"" << className.c_str()
                   << "\":" << gl::FmtErr(HRESULT_CODE(GetLastError()));
        }
    }

    HWND dummyWindow =
        CreateWindowExA(0, reinterpret_cast<const char *>(mWindowClass), "ANGLE Dummy Window",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
    if (!dummyWindow)
    {
        return egl::EglNotInitialized() << "Failed to create dummy OpenGL window.";
    }

    VkSurfaceKHR surfaceVk;
    VkWin32SurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0,
                                        GetModuleHandle(nullptr), dummyWindow};

    VkInstance instance         = mRenderer->getInstance();
    VkPhysicalDevice physDevice = mRenderer->getPhysicalDevice();

    if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surfaceVk) == VK_SUCCESS)
    {
        uint32_t surfaceFormatCount;

        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount, nullptr);
        mSurfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount,
                                             mSurfaceFormats.data());
        vkDestroySurfaceKHR(instance, surfaceVk, nullptr);
    }
    else
    {
        return egl::EglNotInitialized() << "vkCreateWin32SurfaceKHR failed";
    }

    DestroyWindow(dummyWindow);
    return egl::NoError();
}

egl::ConfigSet DisplayVkWin32::generateConfigs()
{
    // If we've already computed the configs, don't need to do it again.
    if (mConfigs.size() > 0)
        return mConfigs;

    constexpr GLenum kColorFormats[] = {GL_RGB565, GL_BGRA8_EXT, GL_BGRX8_ANGLEX};
    mConfigs = egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);

    return mConfigs;
}

bool DisplayVkWin32::checkConfigSupport(egl::Config *config)
{
    const vk::Format &formatVk = this->getRenderer()->getFormat(config->renderTargetFormat);
    VkFormat nativeFormat      = formatVk.vkImageFormat;
    if (mSurfaceFormats.size() == 1u && mSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return true;
    }

    for (const VkSurfaceFormatKHR &surfaceFormat : mSurfaceFormats)
    {
        if (surfaceFormat.format == nativeFormat)
        {
            return true;
        }
    }

    return false;
}

const char *DisplayVkWin32::getWSIExtension() const
{
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
