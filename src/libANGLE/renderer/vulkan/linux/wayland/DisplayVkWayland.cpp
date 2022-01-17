//
// Copyright 2021-2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWayland.cpp:
//    Implements the class methods for DisplayVkWayland.
//

#include "libANGLE/renderer/vulkan/linux/wayland/DisplayVkWayland.h"

#include <wayland-client.h>

#include "common/angleutils.h"
#include "common/linux/dma_buf_utils.h"
#include "common/system_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/linux/wayland/WindowSurfaceVkWayland.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

/// Returns the list of DRM modifiers that a VkFormat supports
std::vector<VkDrmFormatModifierPropertiesEXT> GetDrmModifiers(const DisplayVk *displayVk,
                                                              VkFormat vkFormat)
{
    RendererVk *renderer = displayVk->getRenderer();

    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierProperties(
        formatModifierPropertiesList.drmFormatModifierCount);
    formatModifierPropertiesList.pDrmFormatModifierProperties = formatModifierProperties.data();

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    return formatModifierProperties;
}

/// Returns true if that VkFormat has at least on format modifier in its properties
bool SupportsDrmModifiers(const DisplayVk *displayVk, VkFormat vkFormat)
{
    RendererVk *renderer = displayVk->getRenderer();

    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    // If there is at least one DRM format modifier, it is supported?
    return formatModifierPropertiesList.drmFormatModifierCount > 0;
}

/// Returns a list of VkFormats supporting at least one DRM format modifier
std::vector<VkFormat> GetVkFormatsWithDrmModifiers(const DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();

    std::vector<VkFormat> vkFormats;
    for (size_t formatIndex = 1; formatIndex < angle::kNumANGLEFormats; ++formatIndex)
    {
        const vk::Format &format = renderer->getFormat(angle::FormatID(formatIndex));
        VkFormat vkFormat        = format.getActualImageVkFormat(rx::vk::ImageAccess::Renderable);

        if (vkFormat != VK_FORMAT_UNDEFINED && SupportsDrmModifiers(displayVk, vkFormat))
        {
            vkFormats.push_back(vkFormat);
        }
    }

    return vkFormats;
}

/// Returns a list of supported DRM formats
std::vector<EGLint> GetDrmFormats(const DisplayVk *displayVk)
{
    std::unordered_set<EGLint> drmFormatsSet;
    for (VkFormat vkFormat : GetVkFormatsWithDrmModifiers(displayVk))
    {
        std::vector<EGLint> drmFormats = angle::VkFormatToDrmFourCCFormat(vkFormat);
        for (EGLint drmFormat : drmFormats)
        {
            drmFormatsSet.insert(drmFormat);
        }
    }

    std::vector<EGLint> drmFormats;
    std::copy(std::begin(drmFormatsSet), std::end(drmFormatsSet), std::back_inserter(drmFormats));
    return drmFormats;
}

DisplayVkWayland::DisplayVkWayland(const egl::DisplayState &state)
    : DisplayVkLinux(state), mWaylandDisplay(nullptr)
{}

egl::Error DisplayVkWayland::initialize(egl::Display *display)
{
    mWaylandDisplay = reinterpret_cast<wl_display *>(display->getNativeDisplayId());
    if (!mWaylandDisplay)
    {
        ERR() << "Failed to retrieve wayland display";
        return egl::EglNotInitialized();
    }

    return DisplayVk::initialize(display);
}

void DisplayVkWayland::terminate()
{
    mWaylandDisplay = nullptr;
    DisplayVk::terminate();
}

bool DisplayVkWayland::isValidNativeWindow(EGLNativeWindowType window) const
{
    // Wayland display Errors are fatal.
    // If this function returns non-zero, the display is not valid anymore.
    int error = wl_display_get_error(mWaylandDisplay);
    if (error)
    {
        WARN() << "Wayland window is not valid: " << error << " " << strerror(error);
    }
    return error == 0;
}

SurfaceImpl *DisplayVkWayland::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window)
{
    return new WindowSurfaceVkWayland(state, window, mWaylandDisplay);
}

egl::ConfigSet DisplayVkWayland::generateConfigs()
{
    const std::array<GLenum, 1> kColorFormats = {GL_BGRA8_EXT};

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    if (getCaps().stencil8)
    {
        depthStencilFormats.push_back(GL_STENCIL_INDEX8);
    }
    return egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(),
                                   depthStencilFormats.data(), depthStencilFormats.size(), this);
}

void DisplayVkWayland::checkConfigSupport(egl::Config *config)
{
    // In wayland there is no native visual ID or type
}

const char *DisplayVkWayland::getWSIExtension() const
{
    return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
}

bool IsVulkanWaylandDisplayAvailable()
{
    wl_display *display = wl_display_connect(nullptr);
    if (!display)
    {
        return false;
    }
    wl_display_disconnect(display);
    return true;
}

DisplayImpl *CreateVulkanWaylandDisplay(const egl::DisplayState &state)
{
    return new DisplayVkWayland(state);
}

bool DisplayVkWayland::supportsDmaBufFormat(EGLint format) const
{
    std::vector<EGLint> drmFormats = GetDrmFormats(this);
    return std::find(std::begin(drmFormats), std::end(drmFormats), format) != std::end(drmFormats);
}

egl::Error DisplayVkWayland::queryDmaBufFormats(EGLint maxFormats,
                                                EGLint *formats,
                                                EGLint *numFormats)
{
    std::vector<EGLint> drmFormats = GetDrmFormats(this);

    EGLint formatsSize = static_cast<EGLint>(drmFormats.size());
    if (maxFormats == 0)
    {
        *numFormats = formatsSize;
    }
    else if (maxFormats > 0)
    {
        // Do not copy data beyond the limits of the vector
        maxFormats = std::min(maxFormats, formatsSize);
        std::memcpy(formats, drmFormats.data(), maxFormats * sizeof(EGLint));
    }

    return egl::NoError();
}

egl::Error DisplayVkWayland::queryDmaBufModifiers(EGLint format,
                                                  EGLint maxModifiers,
                                                  EGLuint64KHR *modifiers,
                                                  EGLBoolean *externalOnly,
                                                  EGLint *numModifiers)
{
    std::vector<VkFormat> vkFormats = angle::DrmFourCCFormatToVkFormats(format);

    std::vector<EGLuint64KHR> drmModifiers;
    for (size_t i = 0; i < vkFormats.size(); ++i)
    {
        VkFormat vkFmt = vkFormats[i];

        std::vector<VkDrmFormatModifierPropertiesEXT> vkDrmMods = GetDrmModifiers(this, vkFmt);

        std::vector<EGLuint64KHR> drmMods(vkDrmMods.size());
        std::transform(std::begin(vkDrmMods), std::end(vkDrmMods), std::begin(drmMods),
                       [](VkDrmFormatModifierPropertiesEXT props) {
                           return static_cast<EGLuint64KHR>(props.drmFormatModifier);
                       });
        std::sort(std::begin(drmMods), std::end(drmMods));

        if (i == 0)
        {
            // Just take the modifiers for the first format
            drmModifiers = drmMods;
        }
        else
        {
            // Intersect the modifiers of all the other associated Vulkan formats
            std::vector<EGLuint64KHR> prevMods = drmModifiers;
            drmModifiers.clear();
            std::set_intersection(std::begin(drmMods), std::end(drmMods), std::begin(prevMods),
                                  std::end(prevMods), std::back_inserter(drmModifiers));
        }
    }

    EGLint drmModifiersSize = static_cast<EGLint>(drmModifiers.size());

    if (maxModifiers == 0)
    {
        *numModifiers = drmModifiersSize;
    }
    else if (maxModifiers > 0)
    {
        maxModifiers = std::min(maxModifiers, drmModifiersSize);
        for (EGLint i = 0; i < maxModifiers; ++i)
        {
            modifiers[i] = drmModifiers[i];
        }
    }

    return egl::NoError();
}
}  // namespace rx
