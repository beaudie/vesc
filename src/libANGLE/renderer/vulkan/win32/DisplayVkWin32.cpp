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

SurfaceImpl *DisplayVkWin32::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                   EGLNativeWindowType window,
                                                   EGLint width,
                                                   EGLint height)
{
    return new WindowSurfaceVkWin32(state, window, width, height);
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

    egl::ConfigSet configs;
    if (!RegisterClassEx(&wndClass))
    {
        return configs;
    }

    mHWND = CreateWindowEx(
        0, CLASS_NAME, L"ANGLE Setup Window",
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 64, 64, NULL, NULL, hinstance, NULL);

    configs = egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);

    DestroyWindow(mHWND);

    return configs;
}

bool DisplayVkWin32::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692
    bool foundFormat = false;
    VkSurfaceKHR surfaceVk;
    VkWin32SurfaceCreateInfoKHR info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0,
                                        GetModuleHandle(nullptr), mHWND};

    VkInstance instance         = this->getRenderer()->getInstance();
    VkPhysicalDevice physDevice = this->getRenderer()->getPhysicalDevice();

    if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surfaceVk) == VK_SUCCESS)
    {
        uint32_t surfaceFormatCount;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;

        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount, nullptr);
        surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surfaceVk, &surfaceFormatCount,
                                             surfaceFormats.data());

        // Probably want to cache the formats based on the config color buffer
        // format so we can avoid creating more surfaces.
        const vk::Format &formatVk = this->getRenderer()->getFormat(config->renderTargetFormat);
        VkFormat nativeFormat      = formatVk.vkImageFormat;

        if (surfaceFormatCount == 1u && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            // This is fine.
        }
        else
        {
            for (const VkSurfaceFormatKHR &surfaceFormat : surfaceFormats)
            {
                if (surfaceFormat.format == nativeFormat)
                {
                    foundFormat = true;
                    break;
                }
            }
        }

        vkDestroySurfaceKHR(instance, surfaceVk, nullptr);
    }

    return foundFormat;
}

const char *DisplayVkWin32::getWSIExtension() const
{
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
