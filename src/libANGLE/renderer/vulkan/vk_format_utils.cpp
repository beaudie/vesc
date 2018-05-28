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
    outTextureCaps->renderable =
        HasFormatFeatureBits(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formatProperties) ||
        HasFormatFeatureBits(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, formatProperties);
}

template <class Type, size_t Components, bool Normalized>
struct Native
{
    static constexpr VertexCopyFunction copy =
        CopyNativeVertexData<Type, Components, Components, 0>;
};

template <class Type, size_t Components, bool Normalized>
struct Fixed
{
    static constexpr VertexCopyFunction copy = Copy32FixedTo32FVertexData<Components, Components>;
};

template <class Type, size_t Components, bool Normalized>
struct ToFloat
{
    static constexpr VertexCopyFunction copy =
        CopyTo32FVertexData<Type, Components, Components, Normalized>;
};

template <template <class, size_t, bool> class Copy>
VertexCopyFunction GetCopyFunction(angle::Format::ID id)
{
    switch (id)
    {
        case angle::Format::ID::R8_SSCALED:
            return Copy<GLbyte, 1, false>::copy;
        case angle::Format::ID::R8_USCALED:
            return Copy<GLubyte, 1, false>::copy;
        case angle::Format::ID::R8_SNORM:
            return Copy<GLbyte, 1, true>::copy;
        case angle::Format::ID::R8_UNORM:
            return Copy<GLubyte, 1, true>::copy;
        case angle::Format::ID::R8G8_SSCALED:
            return Copy<GLbyte, 2, false>::copy;
        case angle::Format::ID::R8G8_USCALED:
            return Copy<GLubyte, 2, false>::copy;
        case angle::Format::ID::R8G8_SNORM:
            return Copy<GLbyte, 2, true>::copy;
        case angle::Format::ID::R8G8_UNORM:
            return Copy<GLubyte, 2, true>::copy;
        case angle::Format::ID::R8G8B8_SSCALED:
            return Copy<GLbyte, 3, false>::copy;
        case angle::Format::ID::R8G8B8_USCALED:
            return Copy<GLubyte, 3, false>::copy;
        case angle::Format::ID::R8G8B8_SNORM:
            return Copy<GLbyte, 3, true>::copy;
        case angle::Format::ID::R8G8B8_UNORM:
            return Copy<GLubyte, 3, true>::copy;
        case angle::Format::ID::R8G8B8A8_SSCALED:
            return Copy<GLbyte, 4, false>::copy;
        case angle::Format::ID::R8G8B8A8_USCALED:
            return Copy<GLubyte, 4, false>::copy;
        case angle::Format::ID::R8G8B8A8_SNORM:
            return Copy<GLbyte, 4, true>::copy;
        case angle::Format::ID::R8G8B8A8_UNORM:
            return Copy<GLubyte, 4, true>::copy;
        case angle::Format::ID::R16_SSCALED:
            return Copy<GLshort, 1, false>::copy;
        case angle::Format::ID::R16_USCALED:
            return Copy<GLushort, 1, false>::copy;
        case angle::Format::ID::R16_SNORM:
            return Copy<GLshort, 1, true>::copy;
        case angle::Format::ID::R16_UNORM:
            return Copy<GLushort, 1, true>::copy;
        case angle::Format::ID::R16G16_SSCALED:
            return Copy<GLshort, 2, false>::copy;
        case angle::Format::ID::R16G16_USCALED:
            return Copy<GLushort, 2, false>::copy;
        case angle::Format::ID::R16G16_SNORM:
            return Copy<GLshort, 2, true>::copy;
        case angle::Format::ID::R16G16_UNORM:
            return Copy<GLushort, 2, true>::copy;
        case angle::Format::ID::R16G16B16_SSCALED:
            return Copy<GLshort, 3, false>::copy;
        case angle::Format::ID::R16G16B16_USCALED:
            return Copy<GLushort, 3, false>::copy;
        case angle::Format::ID::R16G16B16_SNORM:
            return Copy<GLshort, 3, true>::copy;
        case angle::Format::ID::R16G16B16_UNORM:
            return Copy<GLushort, 3, true>::copy;
        case angle::Format::ID::R16G16B16A16_SSCALED:
            return Copy<GLshort, 4, false>::copy;
        case angle::Format::ID::R16G16B16A16_USCALED:
            return Copy<GLushort, 4, false>::copy;
        case angle::Format::ID::R16G16B16A16_SNORM:
            return Copy<GLshort, 4, true>::copy;
        case angle::Format::ID::R16G16B16A16_UNORM:
            return Copy<GLushort, 4, true>::copy;
        case angle::Format::ID::R32_SSCALED:
            return Copy<GLint, 1, false>::copy;
        case angle::Format::ID::R32_USCALED:
            return Copy<GLuint, 1, false>::copy;
        case angle::Format::ID::R32_SNORM:
            return Copy<GLint, 1, true>::copy;
        case angle::Format::ID::R32_UNORM:
            return Copy<GLuint, 1, true>::copy;
        case angle::Format::ID::R32G32_SSCALED:
            return Copy<GLint, 2, false>::copy;
        case angle::Format::ID::R32G32_USCALED:
            return Copy<GLuint, 2, false>::copy;
        case angle::Format::ID::R32G32_SNORM:
            return Copy<GLint, 2, true>::copy;
        case angle::Format::ID::R32G32_UNORM:
            return Copy<GLuint, 2, true>::copy;
        case angle::Format::ID::R32G32B32_SSCALED:
            return Copy<GLint, 3, false>::copy;
        case angle::Format::ID::R32G32B32_USCALED:
            return Copy<GLuint, 3, false>::copy;
        case angle::Format::ID::R32G32B32_SNORM:
            return Copy<GLint, 3, true>::copy;
        case angle::Format::ID::R32G32B32_UNORM:
            return Copy<GLuint, 3, true>::copy;
        case angle::Format::ID::R32G32B32A32_SSCALED:
            return Copy<GLint, 4, false>::copy;
        case angle::Format::ID::R32G32B32A32_USCALED:
            return Copy<GLuint, 4, false>::copy;
        case angle::Format::ID::R32G32B32A32_SNORM:
            return Copy<GLint, 4, true>::copy;
        case angle::Format::ID::R32G32B32A32_UNORM:
            return Copy<GLuint, 4, true>::copy;
        case angle::Format::ID::R32_FIXED:
            return Copy<GLint, 1, false>::copy;
        case angle::Format::ID::R32G32_FIXED:
            return Copy<GLint, 2, false>::copy;
        case angle::Format::ID::R32G32B32_FIXED:
            return Copy<GLint, 3, false>::copy;
        case angle::Format::ID::R32G32B32A32_FIXED:
            return Copy<GLint, 4, false>::copy;
        case angle::Format::ID::R32_FLOAT:
            return Copy<GLfloat, 1, false>::copy;
        case angle::Format::ID::R32G32_FLOAT:
            return Copy<GLfloat, 2, false>::copy;
        case angle::Format::ID::R32G32B32_FLOAT:
            return Copy<GLfloat, 3, false>::copy;
        case angle::Format::ID::R32G32B32A32_FLOAT:
            return Copy<GLfloat, 4, false>::copy;
        default:
            return nullptr;
    }
}

bool IsFixedFormat(angle::Format::ID id)
{
    switch (id)
    {
        case angle::Format::ID::R32_FIXED:
        case angle::Format::ID::R32G32_FIXED:
        case angle::Format::ID::R32G32B32_FIXED:
        case angle::Format::ID::R32G32B32A32_FIXED:
            return true;
        default:
            return false;
    }
}

bool IsFloatFormat(angle::Format::ID id)
{
    switch (id)
    {
        case angle::Format::ID::R32_FLOAT:
        case angle::Format::ID::R32G32_FLOAT:
        case angle::Format::ID::R32G32B32_FLOAT:
        case angle::Format::ID::R32G32B32A32_FLOAT:
            return true;
        default:
            return false;
    }
}

void ChangeToFloat(const angle::Format &src, vk::Format *dst)
{
    unsigned components =
        (src.redBits > 0) + (src.greenBits > 0) + (src.blueBits > 0) + (src.alphaBits > 0);
    switch (components)
    {
        case 1:
            dst->bufferFormatID = angle::Format::ID::R32_FLOAT;
            dst->vkBufferFormat = VK_FORMAT_R32_SFLOAT;
            break;
        case 2:
            dst->bufferFormatID = angle::Format::ID::R32G32_FLOAT;
            dst->vkBufferFormat = VK_FORMAT_R32G32_SFLOAT;
            break;
        case 3:
            dst->bufferFormatID = angle::Format::ID::R32G32B32_FLOAT;
            dst->vkBufferFormat = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case 4:
            dst->bufferFormatID = angle::Format::ID::R32G32B32A32_FLOAT;
            dst->vkBufferFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
    }
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

bool HasFullFormatSupport(VkPhysicalDevice physicalDevice, VkFormat vkFormat)
{
    VkFormatProperties formatProperties;
    GetFormatProperties(physicalDevice, vkFormat, &formatProperties);

    constexpr uint32_t kBitsColor =
        (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
         VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    constexpr uint32_t kBitsDepth = (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    return HasFormatFeatureBits(kBitsColor, formatProperties) ||
           HasFormatFeatureBits(kBitsDepth, formatProperties);
}

// Format implementation.
Format::Format()
    : internalFormat(GL_NONE),
      textureFormatID(angle::Format::ID::NONE),
      vkTextureFormat(VK_FORMAT_UNDEFINED),
      bufferFormatID(angle::Format::ID::NONE),
      vkBufferFormat(VK_FORMAT_UNDEFINED),
      dataInitializerFunction(nullptr),
      vertexCopyFunction(nullptr),
      nativeVertexFormat(false)
{
}

const angle::Format &Format::textureFormat() const
{
    return angle::Format::Get(textureFormatID);
}

const angle::Format &Format::bufferFormat() const
{
    return angle::Format::Get(bufferFormatID);
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
        const auto formatID              = static_cast<angle::Format::ID>(formatIndex);
        const angle::Format &angleFormat = angle::Format::Get(formatID);
        mFormatData[formatIndex].initialize(physicalDevice, angleFormat);
        const GLenum internalFormat = mFormatData[formatIndex].internalFormat;
        mFormatData[formatIndex].loadFunctions =
            GetLoadFunctionsMap(internalFormat, mFormatData[formatIndex].textureFormatID);

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

        // formatID/angleFormat is the source format of vertex data.
        // mFormatData[formatIndex].vkBufferFormat is the destination format to use, if possible.
        if (IsFixedFormat(formatID))
        {
            // Fixed source - table should contain float as the destination format.  Conversion is
            // needed.
            mFormatData[formatIndex].nativeVertexFormat = false;
            mFormatData[formatIndex].vertexCopyFunction = GetCopyFunction<Fixed>(formatID);
        }
        else if (IsFloatFormat(formatID))
        {
            // Float support is guaranteed.  See Vulkan spec 1.1 section 30.3.3 table 49.
            mFormatData[formatIndex].nativeVertexFormat = true;
            mFormatData[formatIndex].vertexCopyFunction = GetCopyFunction<Native>(formatID);
        }
        else
        {
            // Source is not fixed or float so must be integer.  Check for support.
            GetFormatProperties(physicalDevice, mFormatData[formatIndex].vkBufferFormat,
                                &formatProperties);
            if (formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
            {
                // Support found.
                mFormatData[formatIndex].nativeVertexFormat = true;
                mFormatData[formatIndex].vertexCopyFunction = GetCopyFunction<Native>(formatID);
            }
            else
            {
                // Unsupported integer source, convert to float.
                ChangeToFloat(angleFormat, &mFormatData[formatIndex]);
                mFormatData[formatIndex].nativeVertexFormat = false;
                mFormatData[formatIndex].vertexCopyFunction = GetCopyFunction<ToFloat>(formatID);
            }
        }
    }
}

const Format &FormatTable::operator[](GLenum internalFormat) const
{
    angle::Format::ID formatID = angle::Format::InternalFormatToID(internalFormat);
    return mFormatData[static_cast<size_t>(formatID)];
}

const Format &FormatTable::operator[](angle::Format::ID formatID) const
{
    return mFormatData[static_cast<size_t>(formatID)];
}

}  // namespace vk

}  // namespace rx
