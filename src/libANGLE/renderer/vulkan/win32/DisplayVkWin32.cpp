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

bool DisplayVkWin32::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (IsWindow(window) == TRUE);
}

DisplayVkWin32::~DisplayVkWin32() {}

SurfaceImpl *DisplayVkWin32::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                   EGLNativeWindowType window)
{
    return new WindowSurfaceVkWin32(state, window);
}

LRESULT APIENTRY WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

egl::ConfigSet DisplayVkWin32::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_RGB565, GL_BGRA8_EXT, GL_BGRX8_ANGLEX};

    HINSTANCE hinstance = GetModuleHandle(nullptr);

    WNDCLASSEX wndClass;
    const wchar_t CLASS_NAME[] = L"ANGLE Class";

    if (!GetClassInfoEx(hinstance, CLASS_NAME, &wndClass))
    {
        wndClass.cbSize        = sizeof(WNDCLASSEX);
        wndClass.style         = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc   = WndProc;
        wndClass.cbClsExtra    = 0;
        wndClass.cbWndExtra    = 0;
        wndClass.hInstance     = hinstance;
        wndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName  = NULL;
        wndClass.lpszClassName = CLASS_NAME;
        wndClass.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);

        if (!RegisterClassEx(&wndClass))
        {
            return mConfigs;
        }
    }

    // If we've already computed the configs, don't need to do it again.
    if (mConfigs.size() > 0)
        return mConfigs;

    mHWND = CreateWindowEx(
        0, CLASS_NAME, L"ANGLE Setup Window",
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 64, 64, NULL, NULL, hinstance, NULL);

    VkSurfaceKHR surfaceVk;
    VkWin32SurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0,
                                        GetModuleHandle(nullptr), mHWND};

    VkInstance instance         = this->getRenderer()->getInstance();
    VkPhysicalDevice physDevice = this->getRenderer()->getPhysicalDevice();

    if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surfaceVk) == VK_SUCCESS)
    {
        uint32_t surfaceFormatCount;

        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount, nullptr);
        mSurfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount,
                                             mSurfaceFormats.data());
        vkDestroySurfaceKHR(instance, surfaceVk, nullptr);
    }

    mConfigs = egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);

    DestroyWindow(mHWND);

    return mConfigs;
}

bool DisplayVkWin32::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692
    bool foundFormat = false;

    const vk::Format &formatVk = this->getRenderer()->getFormat(config->renderTargetFormat);
    VkFormat nativeFormat      = formatVk.vkImageFormat;
    if (mSurfaceFormats.size() == 1u && mSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        // This is fine.
    }
    else
    {
        for (const VkSurfaceFormatKHR &surfaceFormat : mSurfaceFormats)
        {
            if (surfaceFormat.format == nativeFormat)
            {
                foundFormat = true;
                break;
            }
        }
    }

    return foundFormat;
}

const char *DisplayVkWin32::getWSIExtension() const
{
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
