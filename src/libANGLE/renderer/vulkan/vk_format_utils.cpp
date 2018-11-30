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
using SetVkProperties = void (*)(const vk::Format &format, const VkFormatProperties &properties);

void SetTextureVkProperties(const vk::Format &format, const VkFormatProperties &properties)
{
    format.vkProperties.linearTilingFeatures  = properties.linearTilingFeatures;
    format.vkProperties.optimalTilingFeatures = properties.optimalTilingFeatures;
    format.isTextureVkPropertiesSet           = true;
}

void SetBufferVkProperties(const vk::Format &format, const VkFormatProperties &properties)
{
    format.vkProperties.bufferFeatures = properties.bufferFeatures;
    format.isTextureVkPropertiesSet    = true;
}

template <VkFormat vk::Format::*typeFormat,
          bool vk::Format::*isVkPropertiesSet,
          VkFormatFeatureFlags VkFormatProperties::*features,
          SetVkProperties setProperties>
bool HasFormatFeatureBits(VkPhysicalDevice physicalDevice,
                          const vk::Format &format,
                          const VkFormatFeatureFlags featureBits)
{
    if (!(format.*isVkPropertiesSet))
    {
        // If we don't have the actual device features, see if the request features are mandatory.
        // If so, there's no need to query the device.
        const VkFormatProperties &mandatoryProperties =
            vk::GetMandatoryFormatSupport(format.*typeFormat);
        if (IsMaskFlagSet(mandatoryProperties.*features, featureBits))
        {
            return true;
        }

        // Otherwise query the format features and cache it.
        VkFormatProperties deviceProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format.*typeFormat, &deviceProperties);

        setProperties(format, deviceProperties);
    }

    return IsMaskFlagSet(format.vkProperties.*features, featureBits);
}

void AddSampleCounts(VkSampleCountFlags sampleCounts, gl::SupportedSampleSet *outSet)
{
    // The possible bits are VK_SAMPLE_COUNT_n_BIT = n, with n = 1 << b.  At the time of this
    // writing, b is in [0, 6], however, we test all 32 bits in case the enum is extended.
    for (unsigned int i = 0; i < 32; ++i)
    {
        if ((sampleCounts & (1 << i)) != 0)
        {
            outSet->insert(1 << i);
        }
    }
}

void FillTextureFormatCaps(VkPhysicalDevice physicalDevice,
                           const VkPhysicalDeviceLimits &physicalDeviceLimits,
                           const vk::Format &format,
                           gl::TextureCaps *outTextureCaps)
{
    bool hasColorAttachmentFeatureBit = vk::HasTextureFormatFeatureBits(
        physicalDevice, format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    bool hasDepthAttachmentFeatureBit = vk::HasTextureFormatFeatureBits(
        physicalDevice, format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    outTextureCaps->texturable = vk::HasTextureFormatFeatureBits(
        physicalDevice, format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    outTextureCaps->filterable = vk::HasTextureFormatFeatureBits(
        physicalDevice, format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    outTextureCaps->textureAttachment =
        hasColorAttachmentFeatureBit || hasDepthAttachmentFeatureBit;
    outTextureCaps->renderbuffer = outTextureCaps->textureAttachment;

    if (outTextureCaps->renderbuffer)
    {
        if (hasColorAttachmentFeatureBit)
        {
            AddSampleCounts(physicalDeviceLimits.framebufferColorSampleCounts,
                            &outTextureCaps->sampleCounts);
        }
        if (hasDepthAttachmentFeatureBit)
        {
            AddSampleCounts(physicalDeviceLimits.framebufferDepthSampleCounts,
                            &outTextureCaps->sampleCounts);
            AddSampleCounts(physicalDeviceLimits.framebufferStencilSampleCounts,
                            &outTextureCaps->sampleCounts);
        }
    }
}

bool HasFullTextureFormatSupport(VkPhysicalDevice physicalDevice,
                                 VkFormat vkFormat,
                                 vk::Format *format)
{
    ASSERT(!format->isTextureVkPropertiesSet);
    vk::Format testFormat;
    testFormat.vkTextureFormat = vkFormat;

    constexpr uint32_t kBitsColor =
        (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
         VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    constexpr uint32_t kBitsDepth = (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (HasTextureFormatFeatureBits(physicalDevice, testFormat, kBitsColor) ||
        HasTextureFormatFeatureBits(physicalDevice, testFormat, kBitsDepth))
    {
        // If supported, transfer the VkFormatProperties data that may have been cached in
        // testFormat
        format->isTextureVkPropertiesSet           = testFormat.isTextureVkPropertiesSet;
        format->vkProperties.linearTilingFeatures  = testFormat.vkProperties.linearTilingFeatures;
        format->vkProperties.optimalTilingFeatures = testFormat.vkProperties.optimalTilingFeatures;
        return true;
    }

    return false;
}

bool HasFullBufferFormatSupport(VkPhysicalDevice physicalDevice,
                                VkFormat vkFormat,
                                vk::Format *format)
{
    ASSERT(!format->isBufferVkPropertiesSet);
    vk::Format testFormat;
    testFormat.vkBufferFormat = vkFormat;

    if (HasBufferFormatFeatureBits(physicalDevice, testFormat, VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT))
    {
        // If supported, transfer the VkFormatProperties data that may have been cached in
        // testFormat
        format->isBufferVkPropertiesSet     = testFormat.isBufferVkPropertiesSet;
        format->vkProperties.bufferFeatures = testFormat.vkProperties.bufferFeatures;
        return true;
    }

    return false;
}

using SupportTest = bool (*)(VkPhysicalDevice physicalDevice,
                             VkFormat vkFormat,
                             vk::Format *format);

template <class FormatInitInfo>
int FindSupportedFormat(VkPhysicalDevice physicalDevice,
                        const FormatInitInfo *info,
                        int numInfo,
                        SupportTest hasSupport,
                        vk::Format *format)
{
    ASSERT(numInfo > 0);
    const int last = numInfo - 1;

    for (int i = 0; i < last; ++i)
    {
        ASSERT(info[i].format != angle::FormatID::NONE);
        if (hasSupport(physicalDevice, info[i].vkFormat, format))
            return i;
    }

    // List must contain a supported item.  We failed on all the others so the last one must be it.
    ASSERT(info[last].format != angle::FormatID::NONE);
    ASSERT(hasSupport(physicalDevice, info[last].vkFormat, format));
    return last;
}

}  // anonymous namespace

namespace vk
{

// Format implementation.
Format::Format()
    : angleFormatID(angle::FormatID::NONE),
      internalFormat(GL_NONE),
      textureFormatID(angle::FormatID::NONE),
      vkTextureFormat(VK_FORMAT_UNDEFINED),
      bufferFormatID(angle::FormatID::NONE),
      vkBufferFormat(VK_FORMAT_UNDEFINED),
      textureInitializerFunction(nullptr),
      textureLoadFunctions(),
      vertexLoadRequiresConversion(false),
      vkBufferFormatIsPacked(false),
      isTextureVkPropertiesSet(false),
      isBufferVkPropertiesSet(false)
{}

void Format::initTextureFallback(VkPhysicalDevice physicalDevice,
                                 const TextureFormatInitInfo *info,
                                 int numInfo,
                                 const angle::FeaturesVk &featuresVk)
{
    size_t skip = featuresVk.forceFallbackFormat ? 1 : 0;
    int i       = FindSupportedFormat(physicalDevice, info + skip, numInfo - skip,
                                HasFullTextureFormatSupport, this);
    i += skip;

    textureFormatID            = info[i].format;
    vkTextureFormat            = info[i].vkFormat;
    textureInitializerFunction = info[i].initializer;
}

void Format::initBufferFallback(VkPhysicalDevice physicalDevice,
                                const BufferFormatInitInfo *info,
                                int numInfo)
{
    int i = FindSupportedFormat(physicalDevice, info, numInfo, HasFullBufferFormatSupport, this);
    bufferFormatID = info[i].format;
    vkBufferFormat = info[i].vkFormat;
    vkBufferFormatIsPacked       = info[i].vkFormatIsPacked;
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
FormatTable::FormatTable() {}

FormatTable::~FormatTable() {}

void FormatTable::initialize(VkPhysicalDevice physicalDevice,
                             const VkPhysicalDeviceProperties &physicalDeviceProperties,
                             const angle::FeaturesVk &featuresVk,
                             gl::TextureCapsMap *outTextureCapsMap,
                             std::vector<GLenum> *outCompressedTextureFormats)
{
    for (size_t formatIndex = 0; formatIndex < angle::kNumANGLEFormats; ++formatIndex)
    {
        vk::Format &format               = mFormatData[formatIndex];
        const auto formatID              = static_cast<angle::FormatID>(formatIndex);
        const angle::Format &angleFormat = angle::Format::Get(formatID);

        format.initialize(physicalDevice, angleFormat, featuresVk);
        const GLenum internalFormat = format.internalFormat;
        format.textureLoadFunctions = GetLoadFunctionsMap(internalFormat, format.textureFormatID);
        format.angleFormatID        = formatID;

        if (!format.valid())
        {
            continue;
        }

        gl::TextureCaps textureCaps;
        FillTextureFormatCaps(physicalDevice, physicalDeviceProperties.limits, format,
                              &textureCaps);
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

// These functions look at the mandatory format for support, and fallback to querying the device (if
// necessary) to test the availability of the bits.
bool HasLinearTextureFormatFeatureBits(VkPhysicalDevice physicalDevice,
                                       const Format &format,
                                       const VkFormatFeatureFlags featureBits)
{
    return HasFormatFeatureBits<&Format::vkTextureFormat, &Format::isTextureVkPropertiesSet,
                                &VkFormatProperties::linearTilingFeatures, SetTextureVkProperties>(
        physicalDevice, format, featureBits);
}

bool HasTextureFormatFeatureBits(VkPhysicalDevice physicalDevice,
                                 const Format &format,
                                 const VkFormatFeatureFlags featureBits)
{
    return HasFormatFeatureBits<&Format::vkTextureFormat, &Format::isTextureVkPropertiesSet,
                                &VkFormatProperties::optimalTilingFeatures, SetTextureVkProperties>(
        physicalDevice, format, featureBits);
}

bool HasBufferFormatFeatureBits(VkPhysicalDevice physicalDevice,
                                const Format &format,
                                const VkFormatFeatureFlags featureBits)
{
    return HasFormatFeatureBits<&Format::vkBufferFormat, &Format::isBufferVkPropertiesSet,
                                &VkFormatProperties::bufferFeatures, SetBufferVkProperties>(
        physicalDevice, format, featureBits);
}

}  // namespace vk

size_t GetVertexInputAlignment(const vk::Format &format)
{
    const angle::Format &bufferFormat = format.bufferFormat();
    size_t pixelBytes                 = bufferFormat.pixelBytes;
    return format.vkBufferFormatIsPacked ? pixelBytes : (pixelBytes / bufferFormat.channelCount());
}
}  // namespace rx
