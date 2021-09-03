//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DmaBufImageSiblingVkLinux.cpp: Implements DmaBufImageSiblingVkLinux.

#include "libANGLE/renderer/vulkan/linux/DmaBufImageSiblingVkLinux.h"

#include "common/linux/dma_buf_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{
namespace
{
constexpr size_t kMaxPlaneCount = 4;
template <typename T>
using PerPlane = std::array<T, kMaxPlaneCount>;

uint64_t GetModifier(const egl::AttributeMap &attribs, EGLenum lo, EGLenum hi)
{
    if (!attribs.contains(lo))
    {
        return 0;
    }

    ASSERT(attribs.contains(hi));

    uint64_t modifier = attribs.getAsInt(hi);
    modifier          = modifier << 32 | attribs.getAsInt(lo);

    return modifier;
}

void GetModifiers(const egl::AttributeMap &attribs, PerPlane<uint64_t> *drmModifiersOut)
{
    PerPlane<EGLenum> kModifiersLo = {
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT,
        EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT};

    PerPlane<EGLenum> kModifiersHi = {
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT,
        EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT};

    for (size_t plane = 0; plane < kMaxPlaneCount; ++plane)
    {
        (*drmModifiersOut)[plane] = GetModifier(attribs, kModifiersLo[plane], kModifiersHi[plane]);
    }
}

void GetFormatModifierProperties(RendererVk *renderer,
                                 const vk::Format &vkFormat,
                                 const PerPlane<uint64_t> &drmModifiers,
                                 PerPlane<VkDrmFormatModifierPropertiesEXT> *modifierPropertiesOut)
{
    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(),
                                         vkFormat.actualImageVkFormat(), &formatProperties);

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierProperties(
        formatModifierPropertiesList.drmFormatModifierCount);
    formatModifierPropertiesList.pDrmFormatModifierProperties = formatModifierProperties.data();

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(),
                                         vkFormat.actualImageVkFormat(), &formatProperties);

    // Find the requested DRM modifiers.
    for (size_t plane = 0; plane < kMaxPlaneCount; ++plane)
    {
        uint32_t propertiesIndex = formatModifierPropertiesList.drmFormatModifierCount;
        for (uint32_t index = 0; index < formatModifierPropertiesList.drmFormatModifierCount;
             ++index)
        {
            if (formatModifierPropertiesList.pDrmFormatModifierProperties[index]
                    .drmFormatModifier == drmModifiers[plane])
            {
                propertiesIndex = index;
                break;
            }
        }

        // Return the properties if found.
        if (propertiesIndex < formatModifierPropertiesList.drmFormatModifierCount)
        {
            (*modifierPropertiesOut)[plane] =
                formatModifierPropertiesList.pDrmFormatModifierProperties[propertiesIndex];
        }
    }
}

VkImageUsageFlags GetUsageFlags(RendererVk *renderer,
                                const vk::Format &vkFormat,
                                const PerPlane<VkDrmFormatModifierPropertiesEXT> &properties,
                                bool *texturableOut,
                                bool *renderableOut)
{
    const angle::Format &format     = vkFormat.actualImageFormat();
    const bool isDepthStencilFormat = format.depthBits > 0 || format.stencilBits > 0;

    // Check what format features are exposed for this modifier.
    constexpr uint32_t kTextureableRequiredBits =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    constexpr uint32_t kColorRenderableRequiredBits = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    constexpr uint32_t kDepthStencilRenderableRequiredBits =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    *texturableOut =
        IsMaskFlagSet(properties[0].drmFormatModifierTilingFeatures, kTextureableRequiredBits);
    *renderableOut = IsMaskFlagSet(
        properties[0].drmFormatModifierTilingFeatures,
        isDepthStencilFormat ? kDepthStencilRenderableRequiredBits : kColorRenderableRequiredBits);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (*texturableOut)
    {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (*renderableOut)
    {
        usage |= isDepthStencilFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                      : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    return usage;
}

bool IsFormatSupported(RendererVk *renderer,
                       const vk::Format &vkFormat,
                       PerPlane<uint64_t> drmModifier,
                       VkImageUsageFlags usageFlags,
                       VkImageCreateFlags createFlags)
{
    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.pNext  = &externalMemoryImageCreateInfo;
    imageFormatInfo.format = vkFormat.actualImageVkFormat();
    imageFormatInfo.type   = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;  // VK_IMAGE_TILING_OPTIMAL;
    imageFormatInfo.usage  = usageFlags;
    imageFormatInfo.flags  = createFlags;

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmFormatModifierInfo = {};
    drmFormatModifierInfo.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
    drmFormatModifierInfo.drmFormatModifier = drmModifier[0];
    drmFormatModifierInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    if (drmModifier[0] != 0)
    {
        imageFormatInfo.pNext = &drmFormatModifierInfo;
    }

    VkImageFormatProperties2 imageFormatProperties = {};
    imageFormatProperties.sType                    = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

    return vkGetPhysicalDeviceImageFormatProperties2(renderer->getPhysicalDevice(),
                                                     &imageFormatInfo, &imageFormatProperties) !=
           VK_ERROR_FORMAT_NOT_SUPPORTED;

    // TODO: need to verify returned maxExtent against image extents?
}

VkChromaLocation GetChromaLocation(const egl::AttributeMap &attribs, EGLenum hint)
{
    return attribs.getAsInt(hint, EGL_YUV_CHROMA_SITING_0_EXT) == EGL_YUV_CHROMA_SITING_0_EXT
               ? VK_CHROMA_LOCATION_COSITED_EVEN
               : VK_CHROMA_LOCATION_MIDPOINT;
}

VkSamplerYcbcrModelConversion GetYcbcrModel(const egl::AttributeMap &attribs)
{
    switch (attribs.getAsInt(EGL_YUV_COLOR_SPACE_HINT_EXT, EGL_ITU_REC601_EXT))
    {
        case EGL_ITU_REC601_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
        case EGL_ITU_REC709_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
        case EGL_ITU_REC2020_EXT:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
        default:
            UNREACHABLE();
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
    }
}

VkSamplerYcbcrRange GetYcbcrRange(const egl::AttributeMap &attribs)
{
    return attribs.getAsInt(EGL_SAMPLE_RANGE_HINT_EXT, EGL_YUV_FULL_RANGE_EXT) ==
                   EGL_YUV_FULL_RANGE_EXT
               ? VK_SAMPLER_YCBCR_RANGE_ITU_FULL
               : VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
}

}  // anonymous namespace

DmaBufImageSiblingVkLinux::DmaBufImageSiblingVkLinux(const egl::AttributeMap &attribs)
    : mAttribs(attribs),
      mFormat(GL_NONE),
      mRenderable(false),
      mTextureable(false),
      mYUV(false),
      mSamples(0),
      mImage(nullptr)
{
    ASSERT(mAttribs.contains(EGL_WIDTH));
    ASSERT(mAttribs.contains(EGL_HEIGHT));
    mSize.width  = mAttribs.getAsInt(EGL_WIDTH);
    mSize.height = mAttribs.getAsInt(EGL_HEIGHT);
    mSize.depth  = 1;

    int fourCCFormat = mAttribs.getAsInt(EGL_LINUX_DRM_FOURCC_EXT);
    mFormat          = gl::Format(angle::DrmFourCCFormatToGLInternalFormat(fourCCFormat, &mYUV));

    mHasProtectedContent = mAttribs.getAsInt(EGL_PROTECTED_CONTENT_EXT, false);
}

DmaBufImageSiblingVkLinux::~DmaBufImageSiblingVkLinux() {}

egl::Error DmaBufImageSiblingVkLinux::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), displayVk, EGL_BAD_PARAMETER);
}

angle::Result DmaBufImageSiblingVkLinux::initImpl(DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();

    const vk::Format &vkFormat      = renderer->getFormat(mFormat.info->sizedInternalFormat);
    const angle::Format &format     = vkFormat.actualImageFormat();
    const bool isDepthStencilFormat = format.depthBits > 0 || format.stencilBits > 0;

    PerPlane<uint64_t> planeModifiers = {};
    GetModifiers(mAttribs, &planeModifiers);

    // First, check the possible features for the format and determine usage and create flags.
    PerPlane<VkDrmFormatModifierPropertiesEXT> modifierProperties = {};
    GetFormatModifierProperties(renderer, vkFormat, planeModifiers, &modifierProperties);

    const VkImageUsageFlags usageFlags =
        GetUsageFlags(renderer, vkFormat, modifierProperties, &mTextureable, &mRenderable);

    const VkImageCreateFlags createFlags =
        vk::kVkImageCreateFlagsNone | (hasProtectedContent() ? VK_IMAGE_CREATE_PROTECTED_BIT : 0);

    // TODO: what about the memory plane counts of the other EGL planes?
    std::vector<VkSubresourceLayout> planes(modifierProperties[0].drmFormatModifierPlaneCount);

    // TODO: verify this, looks like Vulkan's plane count is "memory planes", but EGL's is "format
    // planes", and memory planes could legitimately be larger. If they don't match, what should we
    // do? Also, does this function have a "benign" failure mode?
    ASSERT(modifierProperties[0].drmFormatModifierPlaneCount <= kMaxPlaneCount);
    for (size_t plane = 0; plane < modifierProperties[0].drmFormatModifierPlaneCount; ++plane)
    {
        planes[plane].offset     = TODO;
        planes[plane].size       = TODO;
        planes[plane].rowPitch   = TODO;
        planes[plane].arrayPitch = TODO;
        planes[plane].depthPitch = TODO;
    }

    VkImageDrmFormatModifierExplicitCreateInfoEXT imageDrmModifierCreateInfo = {};
    imageDrmModifierCreateInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
    imageDrmModifierCreateInfo.drmFormatModifier = modifierProperties[0].drmFormatModifier;
    imageDrmModifierCreateInfo.drmFormatModifierPlaneCount =
        modifierProperties[0].drmFormatModifierPlaneCount;
    imageDrmModifierCreateInfo.pPlaneLayouts = planes.data();

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext       = &imageDrmModifierCreateInfo;
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    // Verify that such a usage is compatible with the provided modifiers, if any.
    // TODO: not needed because GetFormatModifierProperties already does that?
    if (!IsFormatSupported(renderer, vkFormat, planeModifiers, usageFlags, createFlags))
    {
        mTextureable = false;
        mRenderable  = false;
    }

    // Create the image
    mImage = new vk::ImageHelper();

    VkExtent3D vkExtents;
    gl_vk::GetExtent(mSize, &vkExtents);

    constexpr bool kIsRobustInitEnabled = false;

    ANGLE_TRY(mImage->initExternal(displayVk, gl::TextureType::_2D, vkExtents,
                                   vkFormat.intendedFormatID, vkFormat.actualImageFormatID, 1,
                                   usageFlags, createFlags, vk::ImageLayout::ExternalPreInitialized,
                                   &externalMemoryImageCreateInfo, gl::LevelIndex(0), 1, 1,
                                   kIsRobustInitEnabled, nullptr, hasProtectedContent()));

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo = {};
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.image = mImage->getImage().getHandle();

    VkMemoryRequirements externalMemoryRequirements;
    mImage->getImage().getMemoryRequirements(renderer->getDevice(), &externalMemoryRequirements);

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                  (hasProtectedContent() ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0);

    if (mYUV)
    {
        const VkChromaLocation xChromaOffset =
            GetChromaLocation(mAttribs, EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT);
        const VkChromaLocation yChromaOffset =
            GetChromaLocation(mAttribs, EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT);
        const VkSamplerYcbcrModelConversion model = GetYcbcrModel(mAttribs);
        const VkSamplerYcbcrRange range           = GetYcbcrRange(mAttribs);

        ANGLE_VK_CHECK(displayVk, renderer->getFeatures().supportsYUVSamplerConversion.enabled,
                       VK_ERROR_FEATURE_NOT_PRESENT);
        VkSamplerYcbcrConversionCreateInfo yuvConversionInfo = {};
        yuvConversionInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
        yuvConversionInfo.format        = vkFormat.actualImageVkFormat();
        yuvConversionInfo.xChromaOffset = xChromaOffset;
        yuvConversionInfo.yChromaOffset = yChromaOffset;
        yuvConversionInfo.ycbcrModel    = model;
        yuvConversionInfo.ycbcrRange    = range;
        yuvConversionInfo.chromaFilter  = VK_FILTER_NEAREST;
        // yuvConversionInfo.components    = {}; // TODO: swizzle?

        ANGLE_TRY(mImage->initExternalMemory(
            displayVk, renderer->getMemoryProperties(), externalMemoryRequirements,
            &yuvConversionInfo, &dedicatedAllocInfo, VK_QUEUE_FAMILY_FOREIGN_EXT, flags));
    }
    else
    {
        ANGLE_TRY(mImage->initExternalMemory(
            displayVk, renderer->getMemoryProperties(), externalMemoryRequirements, nullptr,
            &dedicatedAllocInfo, VK_QUEUE_FAMILY_FOREIGN_EXT, flags));
    }

    return angle::Result::Continue;
}

void DmaBufImageSiblingVkLinux::onDestroy(const egl::Display *display)
{
    ASSERT(mImage == nullptr);
}

gl::Format DmaBufImageSiblingVkLinux::getFormat() const
{
    return mFormat;
}

bool DmaBufImageSiblingVkLinux::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool DmaBufImageSiblingVkLinux::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

bool DmaBufImageSiblingVkLinux::isYUV() const
{
    return mYUV;
}

bool DmaBufImageSiblingVkLinux::hasProtectedContent() const
{
    return mHasProtectedContent;
}

gl::Extents DmaBufImageSiblingVkLinux::getSize() const
{
    return mSize;
}

size_t DmaBufImageSiblingVkLinux::getSamples() const
{
    return mSamples;
}

// ExternalImageSiblingVk interface
vk::ImageHelper *DmaBufImageSiblingVkLinux::getImage() const
{
    return mImage;
}

void DmaBufImageSiblingVkLinux::release(RendererVk *renderer)
{
    if (mImage != nullptr)
    {
        // TODO: Handle the case where the EGLImage is used in two contexts not in the same share
        // group.  https://issuetracker.google.com/169868803
        mImage->releaseImage(renderer);
        mImage->releaseStagingBuffer(renderer);
        SafeDelete(mImage);
    }
}

}  // namespace rx
