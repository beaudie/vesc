//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_format_utils:
//   Helper for Vulkan format code.

#include "libANGLE/renderer/vulkan/vk_format_utils.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/load_functions_table.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{
namespace
{
constexpr VkFormatFeatureFlags kNecessaryBitsFullSupportDepthStencil =
    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
constexpr VkFormatFeatureFlags kNecessaryBitsFullSupportColor =
    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

bool HasFormatFeatureBits(const VkFormatFeatureFlags featureBits,
                          const VkFormatProperties &formatProperties)
{
    return IsMaskFlagSet(formatProperties.optimalTilingFeatures, featureBits);
}

void FillTextureFormatCaps(const VkFormatProperties &formatProperties,
                           gl::TextureCaps *outTextureCaps)
{
    outTextureCaps->texturable =
        HasFormatFeatureBits(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, formatProperties);
    outTextureCaps->filterable =
        HasFormatFeatureBits(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, formatProperties);
    outTextureCaps->textureAttachment =
        HasFormatFeatureBits(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formatProperties) ||
        HasFormatFeatureBits(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, formatProperties);
    outTextureCaps->renderbuffer = outTextureCaps->textureAttachment;
}

bool HasFullTextureFormatSupport(VkPhysicalDevice physicalDevice, VkFormat vkFormat)
{
    VkFormatProperties formatProperties;
    vk::GetFormatProperties(physicalDevice, vkFormat, &formatProperties);

    constexpr uint32_t kBitsColor =
        (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
         VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    constexpr uint32_t kBitsDepth = (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    return HasFormatFeatureBits(kBitsColor, formatProperties) ||
           HasFormatFeatureBits(kBitsDepth, formatProperties);
}

bool HasFullBufferFormatSupport(VkPhysicalDevice physicalDevice, VkFormat vkFormat)
{
    VkFormatProperties formatProperties;
    vk::GetFormatProperties(physicalDevice, vkFormat, &formatProperties);
    return formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
}

using SupportTest = bool (*)(VkPhysicalDevice physicalDevice, VkFormat vkFormat);

template <class FormatInitInfo>
int FindSupportedFormat(VkPhysicalDevice physicalDevice,
                        const FormatInitInfo *info,
                        int numInfo,
                        SupportTest hasSupport)
{
    ASSERT(numInfo > 1);
    const int last = numInfo - 1;

    for (int i = 0; i < last; ++i)
    {
        ASSERT(info[i].format != angle::FormatID::NONE);
        if (hasSupport(physicalDevice, info[i].vkFormat))
            return i;
    }

    // List must contain a supported item.  We failed on all the others so the last one must be it.
    ASSERT(info[last].format != angle::FormatID::NONE);
    ASSERT(hasSupport(physicalDevice, info[last].vkFormat));
    return last;
}

// Taken from:
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/vk_format_utils.cpp.
bool FormatIsPacked(VkFormat format)
{
    bool found = false;

    switch (format)
    {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            found = true;
            break;
        default:
            break;
    }
    return found;
}

struct FormatInfo
{
    size_t size;
    size_t channel_count;
};

const std::map<VkFormat, FormatInfo> gVkFormatTable = {
    {VK_FORMAT_R4G4_UNORM_PACK8, {1, 2}},
    {VK_FORMAT_UNDEFINED, {0, 0}},
    {VK_FORMAT_R4G4B4A4_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_B4G4R4A4_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_R5G6B5_UNORM_PACK16, {2, 3}},
    {VK_FORMAT_B5G6R5_UNORM_PACK16, {2, 3}},
    {VK_FORMAT_R5G5B5A1_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_B5G5R5A1_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_A1R5G5B5_UNORM_PACK16, {2, 4}},
    {VK_FORMAT_R8_UNORM, {1, 1}},
    {VK_FORMAT_R8_SNORM, {1, 1}},
    {VK_FORMAT_R8_USCALED, {1, 1}},
    {VK_FORMAT_R8_SSCALED, {1, 1}},
    {VK_FORMAT_R8_UINT, {1, 1}},
    {VK_FORMAT_R8_SINT, {1, 1}},
    {VK_FORMAT_R8_SRGB, {1, 1}},
    {VK_FORMAT_R8G8_UNORM, {2, 2}},
    {VK_FORMAT_R8G8_SNORM, {2, 2}},
    {VK_FORMAT_R8G8_USCALED, {2, 2}},
    {VK_FORMAT_R8G8_SSCALED, {2, 2}},
    {VK_FORMAT_R8G8_UINT, {2, 2}},
    {VK_FORMAT_R8G8_SINT, {2, 2}},
    {VK_FORMAT_R8G8_SRGB, {2, 2}},
    {VK_FORMAT_R8G8B8_UNORM, {3, 3}},
    {VK_FORMAT_R8G8B8_SNORM, {3, 3}},
    {VK_FORMAT_R8G8B8_USCALED, {3, 3}},
    {VK_FORMAT_R8G8B8_SSCALED, {3, 3}},
    {VK_FORMAT_R8G8B8_UINT, {3, 3}},
    {VK_FORMAT_R8G8B8_SINT, {3, 3}},
    {VK_FORMAT_R8G8B8_SRGB, {3, 3}},
    {VK_FORMAT_B8G8R8_UNORM, {3, 3}},
    {VK_FORMAT_B8G8R8_SNORM, {3, 3}},
    {VK_FORMAT_B8G8R8_USCALED, {3, 3}},
    {VK_FORMAT_B8G8R8_SSCALED, {3, 3}},
    {VK_FORMAT_B8G8R8_UINT, {3, 3}},
    {VK_FORMAT_B8G8R8_SINT, {3, 3}},
    {VK_FORMAT_B8G8R8_SRGB, {3, 3}},
    {VK_FORMAT_R8G8B8A8_UNORM, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SNORM, {4, 4}},
    {VK_FORMAT_R8G8B8A8_USCALED, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SSCALED, {4, 4}},
    {VK_FORMAT_R8G8B8A8_UINT, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SINT, {4, 4}},
    {VK_FORMAT_R8G8B8A8_SRGB, {4, 4}},
    {VK_FORMAT_B8G8R8A8_UNORM, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SNORM, {4, 4}},
    {VK_FORMAT_B8G8R8A8_USCALED, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SSCALED, {4, 4}},
    {VK_FORMAT_B8G8R8A8_UINT, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SINT, {4, 4}},
    {VK_FORMAT_B8G8R8A8_SRGB, {4, 4}},
    {VK_FORMAT_A8B8G8R8_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SINT_PACK32, {4, 4}},
    {VK_FORMAT_A8B8G8R8_SRGB_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A2R10G10B10_SINT_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_UNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SNORM_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_USCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SSCALED_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_UINT_PACK32, {4, 4}},
    {VK_FORMAT_A2B10G10R10_SINT_PACK32, {4, 4}},
    {VK_FORMAT_R16_UNORM, {2, 1}},
    {VK_FORMAT_R16_SNORM, {2, 1}},
    {VK_FORMAT_R16_USCALED, {2, 1}},
    {VK_FORMAT_R16_SSCALED, {2, 1}},
    {VK_FORMAT_R16_UINT, {2, 1}},
    {VK_FORMAT_R16_SINT, {2, 1}},
    {VK_FORMAT_R16_SFLOAT, {2, 1}},
    {VK_FORMAT_R16G16_UNORM, {4, 2}},
    {VK_FORMAT_R16G16_SNORM, {4, 2}},
    {VK_FORMAT_R16G16_USCALED, {4, 2}},
    {VK_FORMAT_R16G16_SSCALED, {4, 2}},
    {VK_FORMAT_R16G16_UINT, {4, 2}},
    {VK_FORMAT_R16G16_SINT, {4, 2}},
    {VK_FORMAT_R16G16_SFLOAT, {4, 2}},
    {VK_FORMAT_R16G16B16_UNORM, {6, 3}},
    {VK_FORMAT_R16G16B16_SNORM, {6, 3}},
    {VK_FORMAT_R16G16B16_USCALED, {6, 3}},
    {VK_FORMAT_R16G16B16_SSCALED, {6, 3}},
    {VK_FORMAT_R16G16B16_UINT, {6, 3}},
    {VK_FORMAT_R16G16B16_SINT, {6, 3}},
    {VK_FORMAT_R16G16B16_SFLOAT, {6, 3}},
    {VK_FORMAT_R16G16B16A16_UNORM, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SNORM, {8, 4}},
    {VK_FORMAT_R16G16B16A16_USCALED, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SSCALED, {8, 4}},
    {VK_FORMAT_R16G16B16A16_UINT, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SINT, {8, 4}},
    {VK_FORMAT_R16G16B16A16_SFLOAT, {8, 4}},
    {VK_FORMAT_R32_UINT, {4, 1}},
    {VK_FORMAT_R32_SINT, {4, 1}},
    {VK_FORMAT_R32_SFLOAT, {4, 1}},
    {VK_FORMAT_R32G32_UINT, {8, 2}},
    {VK_FORMAT_R32G32_SINT, {8, 2}},
    {VK_FORMAT_R32G32_SFLOAT, {8, 2}},
    {VK_FORMAT_R32G32B32_UINT, {12, 3}},
    {VK_FORMAT_R32G32B32_SINT, {12, 3}},
    {VK_FORMAT_R32G32B32_SFLOAT, {12, 3}},
    {VK_FORMAT_R32G32B32A32_UINT, {16, 4}},
    {VK_FORMAT_R32G32B32A32_SINT, {16, 4}},
    {VK_FORMAT_R32G32B32A32_SFLOAT, {16, 4}},
    {VK_FORMAT_R64_UINT, {8, 1}},
    {VK_FORMAT_R64_SINT, {8, 1}},
    {VK_FORMAT_R64_SFLOAT, {8, 1}},
    {VK_FORMAT_R64G64_UINT, {16, 2}},
    {VK_FORMAT_R64G64_SINT, {16, 2}},
    {VK_FORMAT_R64G64_SFLOAT, {16, 2}},
    {VK_FORMAT_R64G64B64_UINT, {24, 3}},
    {VK_FORMAT_R64G64B64_SINT, {24, 3}},
    {VK_FORMAT_R64G64B64_SFLOAT, {24, 3}},
    {VK_FORMAT_R64G64B64A64_UINT, {32, 4}},
    {VK_FORMAT_R64G64B64A64_SINT, {32, 4}},
    {VK_FORMAT_R64G64B64A64_SFLOAT, {32, 4}},
    {VK_FORMAT_B10G11R11_UFLOAT_PACK32, {4, 3}},
    {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, {4, 3}},
    {VK_FORMAT_D16_UNORM, {2, 1}},
    {VK_FORMAT_X8_D24_UNORM_PACK32, {4, 1}},
    {VK_FORMAT_D32_SFLOAT, {4, 1}},
    {VK_FORMAT_S8_UINT, {1, 1}},
    {VK_FORMAT_D16_UNORM_S8_UINT, {3, 2}},
    {VK_FORMAT_D24_UNORM_S8_UINT, {4, 2}},
    {VK_FORMAT_D32_SFLOAT_S8_UINT, {8, 2}},
    {VK_FORMAT_BC1_RGB_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGB_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC1_RGBA_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_BC2_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC2_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_BC3_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC3_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_BC4_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC4_SNORM_BLOCK, {8, 4}},
    {VK_FORMAT_BC5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC5_SNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC6H_UFLOAT_BLOCK, {16, 4}},
    {VK_FORMAT_BC6H_SFLOAT_BLOCK, {16, 4}},
    {VK_FORMAT_BC7_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_BC7_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, {8, 3}},
    {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, {8, 3}},
    {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, {8, 4}},
    {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, {8, 4}},
    {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_EAC_R11_UNORM_BLOCK, {8, 1}},
    {VK_FORMAT_EAC_R11_SNORM_BLOCK, {8, 1}},
    {VK_FORMAT_EAC_R11G11_UNORM_BLOCK, {16, 2}},
    {VK_FORMAT_EAC_R11G11_SNORM_BLOCK, {16, 2}},
    {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x4_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x4_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_5x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_6x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_8x8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x5_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x5_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x6_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x6_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x8_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x8_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x10_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_10x10_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x10_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x10_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x12_UNORM_BLOCK, {16, 4}},
    {VK_FORMAT_ASTC_12x12_SRGB_BLOCK, {16, 4}},
    {VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, {8, 4}},
    {VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG, {8, 4}},
    /* KHR_sampler_YCbCr_conversion */
    {VK_FORMAT_G8B8G8R8_422_UNORM_KHR, {4, 4}},
    {VK_FORMAT_B8G8R8G8_422_UNORM_KHR, {4, 4}},
    {VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR, {8, 4}},
    {VK_FORMAT_G16B16G16R16_422_UNORM_KHR, {8, 4}},
    {VK_FORMAT_B16G16R16G16_422_UNORM_KHR, {8, 4}}};

size_t FormatChannelCount(VkFormat format)
{
    auto item = gVkFormatTable.find(format);
    if (item != gVkFormatTable.end())
    {
        return item->second.channel_count;
    }
    return 0;
}

size_t FormatSize(VkFormat format)
{
    auto item = gVkFormatTable.find(format);
    if (item != gVkFormatTable.end())
    {
        return item->second.size;
    }
    return 0;
}

}  // anonymous namespace

namespace vk
{

void GetFormatProperties(VkPhysicalDevice physicalDevice,
                         VkFormat vkFormat,
                         VkFormatProperties *propertiesOut)
{
    // Try filling out the info from our hard coded format data, if we can't find the
    // information we need, we'll make the call to Vulkan.
    const VkFormatProperties &formatProperties = vk::GetMandatoryFormatSupport(vkFormat);

    // Once we filled what we could with the mandatory texture caps, we verify if
    // all the bits we need to satify all our checks are present, and if so we can
    // skip the device call.
    if (!IsMaskFlagSet(formatProperties.optimalTilingFeatures, kNecessaryBitsFullSupportColor) &&
        !IsMaskFlagSet(formatProperties.optimalTilingFeatures,
                       kNecessaryBitsFullSupportDepthStencil))
    {
        vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, propertiesOut);
    }
    else
    {
        *propertiesOut = formatProperties;
    }
}

size_t GetFormatAlignment(VkFormat format)
{
    if (FormatIsPacked(format))
    {
        return FormatSize(format);
    }
    else
    {
        return FormatSize(format) / FormatChannelCount(format);
    }
}

// Format implementation.
Format::Format()
    : angleFormatID(angle::FormatID::NONE),
      internalFormat(GL_NONE),
      textureFormatID(angle::FormatID::NONE),
      vkTextureFormat(VK_FORMAT_UNDEFINED),
      bufferFormatID(angle::FormatID::NONE),
      vkBufferFormat(VK_FORMAT_UNDEFINED),
      textureInitializerFunction(nullptr),
      textureLoadFunctions()
{
}

void Format::initTextureFallback(VkPhysicalDevice physicalDevice,
                                 const TextureFormatInitInfo *info,
                                 int numInfo)
{
    int i = FindSupportedFormat(physicalDevice, info, numInfo, HasFullTextureFormatSupport);
    textureFormatID            = info[i].format;
    vkTextureFormat            = info[i].vkFormat;
    textureInitializerFunction = info[i].initializer;
}

void Format::initBufferFallback(VkPhysicalDevice physicalDevice,
                                const BufferFormatInitInfo *info,
                                int numInfo)
{
    int i          = FindSupportedFormat(physicalDevice, info, numInfo, HasFullBufferFormatSupport);
    bufferFormatID = info[i].format;
    vkBufferFormat = info[i].vkFormat;
    vertexLoadFunction           = info[i].vertexLoadFunction;
    vertexLoadRequiresConversion = info[i].vertexLoadRequiresConversion;
}

const angle::Format &Format::textureFormat() const
{
    return angle::Format::Get(textureFormatID);
}

const angle::Format &Format::bufferFormat() const
{
    return angle::Format::Get(bufferFormatID);
}

const angle::Format &Format::angleFormat() const
{
    return angle::Format::Get(angleFormatID);
}

bool operator==(const Format &lhs, const Format &rhs)
{
    return &lhs == &rhs;
}

bool operator!=(const Format &lhs, const Format &rhs)
{
    return &lhs != &rhs;
}

// FormatTable implementation.
FormatTable::FormatTable()
{
}

FormatTable::~FormatTable()
{
}

void FormatTable::initialize(VkPhysicalDevice physicalDevice,
                             gl::TextureCapsMap *outTextureCapsMap,
                             std::vector<GLenum> *outCompressedTextureFormats)
{
    for (size_t formatIndex = 0; formatIndex < angle::kNumANGLEFormats; ++formatIndex)
    {
        const auto formatID              = static_cast<angle::FormatID>(formatIndex);
        const angle::Format &angleFormat = angle::Format::Get(formatID);
        mFormatData[formatIndex].initialize(physicalDevice, angleFormat);
        const GLenum internalFormat = mFormatData[formatIndex].internalFormat;
        mFormatData[formatIndex].textureLoadFunctions =
            GetLoadFunctionsMap(internalFormat, mFormatData[formatIndex].textureFormatID);
        mFormatData[formatIndex].angleFormatID = formatID;

        if (!mFormatData[formatIndex].valid())
        {
            continue;
        }

        const VkFormat vkFormat = mFormatData[formatIndex].vkTextureFormat;

        // Try filling out the info from our hard coded format data, if we can't find the
        // information we need, we'll make the call to Vulkan.
        VkFormatProperties formatProperties;
        GetFormatProperties(physicalDevice, vkFormat, &formatProperties);
        gl::TextureCaps textureCaps;
        FillTextureFormatCaps(formatProperties, &textureCaps);
        outTextureCapsMap->set(formatID, textureCaps);

        if (angleFormat.isBlock)
        {
            outCompressedTextureFormats->push_back(internalFormat);
        }
    }
}

const Format &FormatTable::operator[](GLenum internalFormat) const
{
    angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
    return mFormatData[static_cast<size_t>(formatID)];
}

const Format &FormatTable::operator[](angle::FormatID formatID) const
{
    return mFormatData[static_cast<size_t>(formatID)];
}

}  // namespace vk

}  // namespace rx
