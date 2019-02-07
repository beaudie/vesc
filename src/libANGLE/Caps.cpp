//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/Caps.h"

#include "common/angleutils.h"
#include "common/debug.h"

#include "libANGLE/formatutils.h"

#include "angle_gl.h"

#include <algorithm>
#include <sstream>

static void InsertExtensionString(const std::string &extension,
                                  bool supported,
                                  std::vector<std::string> *extensionVector)
{
    if (supported)
    {
        extensionVector->push_back(extension);
    }
}

namespace gl
{

TextureCaps::TextureCaps()
    : texturable(false),
      filterable(false),
      textureAttachment(false),
      renderbuffer(false),
      sampleCounts()
{}

TextureCaps::TextureCaps(const TextureCaps &other) = default;

TextureCaps::~TextureCaps() = default;

GLuint TextureCaps::getMaxSamples() const
{
    return !sampleCounts.empty() ? *sampleCounts.rbegin() : 0;
}

GLuint TextureCaps::getNearestSamples(GLuint requestedSamples) const
{
    if (requestedSamples == 0)
    {
        return 0;
    }

    for (SupportedSampleSet::const_iterator i = sampleCounts.begin(); i != sampleCounts.end(); i++)
    {
        GLuint samples = *i;
        if (samples >= requestedSamples)
        {
            return samples;
        }
    }

    return 0;
}

TextureCaps GenerateMinimumTextureCaps(GLenum sizedInternalFormat,
                                       const Version &clientVersion,
                                       const Extensions &extensions)
{
    TextureCaps caps;

    const InternalFormat &internalFormatInfo = GetSizedInternalFormatInfo(sizedInternalFormat);
    caps.texturable        = internalFormatInfo.textureSupport(clientVersion, extensions);
    caps.filterable        = internalFormatInfo.filterSupport(clientVersion, extensions);
    caps.textureAttachment = internalFormatInfo.textureAttachmentSupport(clientVersion, extensions);
    caps.renderbuffer      = internalFormatInfo.renderbufferSupport(clientVersion, extensions);

    caps.sampleCounts.insert(0);
    if (internalFormatInfo.isRequiredRenderbufferFormat(clientVersion))
    {
        if ((clientVersion.major >= 3 && clientVersion.minor >= 1) ||
            (clientVersion.major >= 3 && internalFormatInfo.componentType != GL_UNSIGNED_INT &&
             internalFormatInfo.componentType != GL_INT))
        {
            caps.sampleCounts.insert(4);
        }
    }

    return caps;
}

TextureCapsMap::TextureCapsMap() {}

TextureCapsMap::~TextureCapsMap() {}

void TextureCapsMap::insert(GLenum internalFormat, const TextureCaps &caps)
{
    angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
    get(formatID)            = caps;
}

void TextureCapsMap::clear()
{
    mFormatData.fill(TextureCaps());
}

const TextureCaps &TextureCapsMap::get(GLenum internalFormat) const
{
    angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
    return get(formatID);
}

const TextureCaps &TextureCapsMap::get(angle::FormatID formatID) const
{
    return mFormatData[static_cast<size_t>(formatID)];
}

TextureCaps &TextureCapsMap::get(angle::FormatID formatID)
{
    return mFormatData[static_cast<size_t>(formatID)];
}

void TextureCapsMap::set(angle::FormatID formatID, const TextureCaps &caps)
{
    get(formatID) = caps;
}

void InitMinimumTextureCapsMap(const Version &clientVersion,
                               const Extensions &extensions,
                               TextureCapsMap *capsMap)
{
    for (GLenum internalFormat : GetAllSizedInternalFormats())
    {
        capsMap->insert(internalFormat,
                        GenerateMinimumTextureCaps(internalFormat, clientVersion, extensions));
    }
}

Extensions::Extensions()
    : elementIndexUint(false),
      packedDepthStencil(false),
      getProgramBinary(false),
      rgb8rgba8(false),
      textureFormatBGRA8888(false),
      readFormatBGRA(false),
      pixelBufferObject(false),
      mapBuffer(false),
      mapBufferRange(false),
      colorBufferHalfFloat(false),
      textureHalfFloat(false),
      textureHalfFloatLinear(false),
      textureFloat(false),
      textureFloatLinear(false),
      textureRG(false),
      textureCompressionDXT1(false),
      textureCompressionDXT3(false),
      textureCompressionDXT5(false),
      textureCompressionS3TCsRGB(false),
      textureCompressionASTCHDR(false),
      textureCompressionASTCLDR(false),
      textureCompressionBPTC(false),
      compressedETC1RGB8Texture(false),
      compressedETC2RGB8Texture(false),
      compressedETC2sRGB8Texture(false),
      compressedETC2PunchthroughARGB8Texture(false),
      compressedETC2PunchthroughAsRGB8AlphaTexture(false),
      compressedETC2RGBA8Texture(false),
      compressedETC2sRGB8Alpha8Texture(false),
      compressedEACR11UnsignedTexture(false),
      compressedEACR11SignedTexture(false),
      compressedEACRG11UnsignedTexture(false),
      compressedEACRG11SignedTexture(false),
      compressedTextureETC(false),
      sRGB(false),
      depthTextures(false),
      depth32(false),
      textureStorage(false),
      textureNPOT(false),
      drawBuffers(false),
      textureFilterAnisotropic(false),
      maxTextureAnisotropy(0.0f),
      occlusionQueryBoolean(false),
      fence(false),
      disjointTimerQuery(false),
      queryCounterBitsTimeElapsed(0),
      queryCounterBitsTimestamp(0),
      robustness(false),
      robustBufferAccessBehavior(false),
      blendMinMax(false),
      framebufferBlit(false),
      framebufferMultisample(false),
      instancedArrays({false, false}),
      packReverseRowOrder(false),
      standardDerivatives(false),
      shaderTextureLOD(false),
      fragDepth(false),
      multiview(false),
      maxViews(1u),
      textureUsage(false),
      translatedShaderSource(false),
      fboRenderMipmap(false),
      discardFramebuffer(false),
      debugMarker(false),
      eglImage(false),
      eglImageExternal(false),
      eglImageExternalEssl3(false),
      eglSync(false),
      eglStreamConsumerExternal(false),
      unpackSubimage(false),
      packSubimage(false),
      vertexArrayObject(false),
      debug(false),
      maxDebugMessageLength(0),
      maxDebugLoggedMessages(0),
      maxDebugGroupStackDepth(0),
      maxLabelLength(0),
      noError(false),
      lossyETCDecode(false),
      bindUniformLocation(false),
      syncQuery(false),
      copyTexture(false),
      copyCompressedTexture(false),
      copyTexture3d(false),
      webglCompatibility(false),
      requestExtension(false),
      bindGeneratesResource(false),
      robustClientMemory(false),
      textureBorderClamp(false),
      textureSRGBDecode(false),
      sRGBWriteControl(false),
      colorBufferFloatRGB(false),
      colorBufferFloatRGBA(false),
      colorBufferFloat(false),
      multisampleCompatibility(false),
      framebufferMixedSamples(false),
      textureNorm16(false),
      pathRendering(false),
      surfacelessContext(false),
      clientArrays(false),
      robustResourceInitialization(false),
      programCacheControl(false),
      textureRectangle(false),
      geometryShader(false),
      pointSizeArray(false),
      textureCubeMap(false),
      pointSprite(false),
      drawTexture(false),
      explicitContextGles1(false),
      explicitContext(false),
      parallelShaderCompile(false),
      textureStorageMultisample2DArray(false),
      multiviewMultisample(false),
      blendFuncExtended(false),
      maxDualSourceDrawBuffers(0),
      memorySize(false),
      textureMultisample(false),
      multiDraw(false)
{}

Extensions::Extensions(const Extensions &other) = default;

std::vector<std::string> Extensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    for (const auto &extensionInfo : GetExtensionInfoMap())
    {
        if (extensionInfo.second.ExtensionsMember.get(*this))
        {
            extensionStrings.push_back(extensionInfo.first);
        }
    }

    return extensionStrings;
}

Limitations::Limitations()
    : noFrontFacingSupport(false),
      noSampleAlphaToCoverageSupport(false),
      attributeZeroRequiresZeroDivisorInEXT(false),
      noSeparateStencilRefsAndMasks(false),
      shadersRequireIndexedLoopValidation(false),
      noSimultaneousConstantColorAndAlphaBlendFunc(false),
      noFlexibleVaryingPacking(false),
      noDoubleBoundTransformFeedbackBuffers(false)
{}

static bool GetFormatSupportBase(const TextureCapsMap &textureCaps,
                                 const GLenum *requiredFormats,
                                 size_t requiredFormatsSize,
                                 bool requiresTexturing,
                                 bool requiresFiltering,
                                 bool requiresAttachingTexture,
                                 bool requiresRenderbufferSupport)
{
    for (size_t i = 0; i < requiredFormatsSize; i++)
    {
        const TextureCaps &cap = textureCaps.get(requiredFormats[i]);

        if (requiresTexturing && !cap.texturable)
        {
            return false;
        }

        if (requiresFiltering && !cap.filterable)
        {
            return false;
        }

        if (requiresAttachingTexture && !cap.textureAttachment)
        {
            return false;
        }

        if (requiresRenderbufferSupport && !cap.renderbuffer)
        {
            return false;
        }
    }

    return true;
}

template <size_t N>
static bool GetFormatSupport(const TextureCapsMap &textureCaps,
                             const GLenum (&requiredFormats)[N],
                             bool requiresTexturing,
                             bool requiresFiltering,
                             bool requiresAttachingTexture,
                             bool requiresRenderbufferSupport)
{
    return GetFormatSupportBase(textureCaps, requiredFormats, N, requiresTexturing,
                                requiresFiltering, requiresAttachingTexture,
                                requiresRenderbufferSupport);
}

// Check for GL_OES_packed_depth_stencil
static bool DeterminePackedDepthStencilSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH24_STENCIL8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true);
}

// Checks for GL_OES_rgb8_rgba8 support
static bool DetermineRGB8AndRGBA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGB8,
        GL_RGBA8,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, false, true);
}

// Checks for GL_EXT_texture_format_BGRA8888 support
static bool DetermineBGRA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_BGRA8_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, true, true);
}

// Checks for GL_OES_color_buffer_half_float support
static bool DetermineColorBufferHalfFloatSupport(const TextureCapsMap &textureCaps)
{
    // EXT_color_buffer_half_float issue #2 states that an implementation doesn't need to support
    // rendering to any of the formats but is expected to be able to render to at least one. WebGL
    // requires that at least RGBA16F is renderable so we make the same requirement.
    constexpr GLenum requiredFormats[] = {
        GL_RGBA16F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true);
}

// Checks for GL_OES_texture_half_float support
static bool DetermineHalfFloatTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA16F, GL_RGB16F, GL_LUMINANCE_ALPHA16F_EXT, GL_LUMINANCE16F_EXT, GL_ALPHA16F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false, false);
}

// Checks for GL_OES_texture_half_float_linear support
static bool DetermineHalfFloatTextureFilteringSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA16F, GL_RGB16F, GL_LUMINANCE_ALPHA16F_EXT, GL_LUMINANCE16F_EXT, GL_ALPHA16F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false, false);
}

// Checks for GL_OES_texture_float support
static bool DetermineFloatTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F, GL_RGB32F, GL_LUMINANCE_ALPHA32F_EXT, GL_LUMINANCE32F_EXT, GL_ALPHA32F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false, false);
}

// Checks for GL_OES_texture_float_linear support
static bool DetermineFloatTextureFilteringSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F, GL_RGB32F, GL_LUMINANCE_ALPHA32F_EXT, GL_LUMINANCE32F_EXT, GL_ALPHA32F_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false, false);
}

// Checks for GL_EXT_texture_rg support
static bool DetermineRGTextureSupport(const TextureCapsMap &textureCaps,
                                      bool checkHalfFloatFormats,
                                      bool checkFloatFormats)
{
    constexpr GLenum requiredFormats[] = {
        GL_R8,
        GL_RG8,
    };
    constexpr GLenum requiredHalfFloatFormats[] = {
        GL_R16F,
        GL_RG16F,
    };
    constexpr GLenum requiredFloatFormats[] = {
        GL_R32F,
        GL_RG32F,
    };

    if (checkHalfFloatFormats &&
        !GetFormatSupport(textureCaps, requiredHalfFloatFormats, true, false, false, false))
    {
        return false;
    }

    if (checkFloatFormats &&
        !GetFormatSupport(textureCaps, requiredFloatFormats, true, false, false, false))
    {
        return false;
    }

    return GetFormatSupport(textureCaps, requiredFormats, true, true, true, true);
}

// Check for GL_EXT_texture_compression_dxt1
static bool DetermineDXT1TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_ANGLE_texture_compression_dxt3
static bool DetermineDXT3TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_ANGLE_texture_compression_dxt5
static bool DetermineDXT5TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_EXT_texture_compression_s3tc_srgb
static bool DetermineS3TCsRGBTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_KHR_texture_compression_astc_hdr and GL_KHR_texture_compression_astc_ldr
static bool DetermineASTCTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_ASTC_4x4_KHR,           GL_COMPRESSED_RGBA_ASTC_5x4_KHR,
        GL_COMPRESSED_RGBA_ASTC_5x5_KHR,           GL_COMPRESSED_RGBA_ASTC_6x5_KHR,
        GL_COMPRESSED_RGBA_ASTC_6x6_KHR,           GL_COMPRESSED_RGBA_ASTC_8x5_KHR,
        GL_COMPRESSED_RGBA_ASTC_8x6_KHR,           GL_COMPRESSED_RGBA_ASTC_8x8_KHR,
        GL_COMPRESSED_RGBA_ASTC_10x5_KHR,          GL_COMPRESSED_RGBA_ASTC_10x6_KHR,
        GL_COMPRESSED_RGBA_ASTC_10x8_KHR,          GL_COMPRESSED_RGBA_ASTC_10x10_KHR,
        GL_COMPRESSED_RGBA_ASTC_12x10_KHR,         GL_COMPRESSED_RGBA_ASTC_12x12_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,   GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,
        GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_ETC1_RGB8_OES
static bool DetermineETC1RGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_ETC1_RGB8_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_RGB8_texture
static bool DetermineETC2RGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB8_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_sRGB8_texture
static bool DetermineETC2sRGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_punchthroughA_RGBA8_texture
static bool DetermineETC2PunchthroughARGB8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture
static bool DetermineETC2PunchthroughAsRGB8AlphaTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_RGBA8_texture
static bool DetermineETC2RGBA8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA8_ETC2_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_ETC2_sRGB8_alpha8_texture
static bool DetermineETC2sRGB8Alpha8TextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_EAC_R11_unsigned_texture
static bool DetermineEACR11UnsignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_R11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_EAC_R11_signed_texture
static bool DetermineEACR11SignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SIGNED_R11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_EAC_RG11_unsigned_texture
static bool DetermineEACRG11UnsignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RG11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for OES_compressed_EAC_RG11_signed_texture
static bool DetermineEACRG11SignedTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_SIGNED_RG11_EAC,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

// Check for GL_EXT_sRGB
static bool DetermineSRGBTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {
        GL_SRGB8,
        GL_SRGB8_ALPHA8,
    };

    constexpr GLenum requiredRenderFormats[] = {
        GL_SRGB8_ALPHA8,
    };

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false) &&
           GetFormatSupport(textureCaps, requiredRenderFormats, true, false, true, true);
}

// Check for GL_ANGLE_depth_texture
static bool DetermineDepthTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT32_OES,
        GL_DEPTH24_STENCIL8_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, true, true, true);
}

// Check for GL_OES_depth32
static bool DetermineDepth32Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_DEPTH_COMPONENT32_OES,
    };

    return GetFormatSupport(textureCaps, requiredFormats, false, false, true, true);
}

// Check for GL_CHROMIUM_color_buffer_float_rgb
static bool DetermineColorBufferFloatRGBSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGB32F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false);
}

// Check for GL_CHROMIUM_color_buffer_float_rgba
static bool DetermineColorBufferFloatRGBASupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_RGBA32F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, false);
}

// Check for GL_EXT_color_buffer_float
static bool DetermineColorBufferFloatSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_R16F, GL_RG16F, GL_RGBA16F, GL_R32F, GL_RG32F, GL_RGBA32F, GL_R11F_G11F_B10F,
    };

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true, true);
}

// Check for GL_EXT_texture_norm16
static bool DetermineTextureNorm16Support(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFilterFormats[] = {
        GL_R16_EXT,       GL_RG16_EXT,       GL_RGB16_EXT,       GL_RGBA16_EXT,
        GL_R16_SNORM_EXT, GL_RG16_SNORM_EXT, GL_RGB16_SNORM_EXT, GL_RGBA16_SNORM_EXT,
    };

    constexpr GLenum requiredRenderFormats[] = {
        GL_R16_EXT,
        GL_RG16_EXT,
        GL_RGBA16_EXT,
    };

    return GetFormatSupport(textureCaps, requiredFilterFormats, true, true, false, false) &&
           GetFormatSupport(textureCaps, requiredRenderFormats, true, false, true, true);
}

// Check for EXT_texture_compression_bptc
static bool DetermineBPTCTextureSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {
        GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,
        GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

bool DetermineCompressedTextureETCSupport(const TextureCapsMap &textureCaps)
{
    constexpr GLenum requiredFormats[] = {GL_COMPRESSED_R11_EAC,
                                          GL_COMPRESSED_SIGNED_R11_EAC,
                                          GL_COMPRESSED_RG11_EAC,
                                          GL_COMPRESSED_SIGNED_RG11_EAC,
                                          GL_COMPRESSED_RGB8_ETC2,
                                          GL_COMPRESSED_SRGB8_ETC2,
                                          GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                          GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                          GL_COMPRESSED_RGBA8_ETC2_EAC,
                                          GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC};

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false, false);
}

void Extensions::setTextureExtensionSupport(const TextureCapsMap &textureCaps)
{
    // TODO(ynovikov): rgb8rgba8, colorBufferHalfFloat, textureHalfFloat, textureHalfFloatLinear,
    // textureFloat, textureFloatLinear, textureRG, sRGB, colorBufferFloatRGB, colorBufferFloatRGBA
    // and colorBufferFloat were verified. Verify the rest.
    packedDepthStencil    = DeterminePackedDepthStencilSupport(textureCaps);
    rgb8rgba8             = DetermineRGB8AndRGBA8TextureSupport(textureCaps);
    textureFormatBGRA8888 = DetermineBGRA8TextureSupport(textureCaps);
    textureHalfFloat      = DetermineHalfFloatTextureSupport(textureCaps);
    textureHalfFloatLinear =
        textureHalfFloat && DetermineHalfFloatTextureFilteringSupport(textureCaps);
    textureFloat           = DetermineFloatTextureSupport(textureCaps);
    textureFloatLinear     = textureFloat && DetermineFloatTextureFilteringSupport(textureCaps);
    textureRG              = DetermineRGTextureSupport(textureCaps, textureHalfFloat, textureFloat);
    colorBufferHalfFloat   = textureHalfFloat && DetermineColorBufferHalfFloatSupport(textureCaps);
    textureCompressionDXT1 = DetermineDXT1TextureSupport(textureCaps);
    textureCompressionDXT3 = DetermineDXT3TextureSupport(textureCaps);
    textureCompressionDXT5 = DetermineDXT5TextureSupport(textureCaps);
    textureCompressionS3TCsRGB = DetermineS3TCsRGBTextureSupport(textureCaps);
    textureCompressionASTCHDR  = DetermineASTCTextureSupport(textureCaps);
    textureCompressionASTCLDR  = textureCompressionASTCHDR;
    compressedETC1RGB8Texture  = DetermineETC1RGB8TextureSupport(textureCaps);
    compressedETC2RGB8Texture  = DetermineETC2RGB8TextureSupport(textureCaps);
    compressedETC2sRGB8Texture = DetermineETC2sRGB8TextureSupport(textureCaps);
    compressedETC2PunchthroughARGB8Texture =
        DetermineETC2PunchthroughARGB8TextureSupport(textureCaps);
    compressedETC2PunchthroughAsRGB8AlphaTexture =
        DetermineETC2PunchthroughAsRGB8AlphaTextureSupport(textureCaps);
    compressedETC2RGBA8Texture       = DetermineETC2RGBA8TextureSupport(textureCaps);
    compressedETC2sRGB8Alpha8Texture = DetermineETC2sRGB8Alpha8TextureSupport(textureCaps);
    compressedEACR11UnsignedTexture  = DetermineEACR11UnsignedTextureSupport(textureCaps);
    compressedEACR11SignedTexture    = DetermineEACR11SignedTextureSupport(textureCaps);
    compressedEACRG11UnsignedTexture = DetermineEACRG11UnsignedTextureSupport(textureCaps);
    compressedEACRG11SignedTexture   = DetermineEACRG11SignedTextureSupport(textureCaps);
    sRGB                             = DetermineSRGBTextureSupport(textureCaps);
    depthTextures                    = DetermineDepthTextureSupport(textureCaps);
    depth32                          = DetermineDepth32Support(textureCaps);
    colorBufferFloatRGB              = DetermineColorBufferFloatRGBSupport(textureCaps);
    colorBufferFloatRGBA             = DetermineColorBufferFloatRGBASupport(textureCaps);
    colorBufferFloat                 = DetermineColorBufferFloatSupport(textureCaps);
    textureNorm16                    = DetermineTextureNorm16Support(textureCaps);
    textureCompressionBPTC           = DetermineBPTCTextureSupport(textureCaps);
}

const ExtensionInfoMap &GetExtensionInfoMap()
{
    auto buildExtensionInfoMap = []() {
        auto enableableExtension = [](ExtensionRef member) {
            ExtensionInfo info;
            info.Requestable      = true;
            info.ExtensionsMember = member;
            return info;
        };

        auto esOnlyExtension = [](ExtensionRef member) {
            ExtensionInfo info;
            info.ExtensionsMember = member;
            return info;
        };

        // clang-format off
        ExtensionInfoMap map;
        map["GL_OES_element_index_uint"] = enableableExtension(EXTENSIONREF(elementIndexUint));
        map["GL_OES_packed_depth_stencil"] = esOnlyExtension(EXTENSIONREF(packedDepthStencil));
        map["GL_OES_get_program_binary"] = enableableExtension(EXTENSIONREF(getProgramBinary));
        map["GL_OES_rgb8_rgba8"] = enableableExtension(EXTENSIONREF(rgb8rgba8));
        map["GL_EXT_texture_format_BGRA8888"] = enableableExtension(EXTENSIONREF(textureFormatBGRA8888));
        map["GL_EXT_read_format_bgra"] = esOnlyExtension(EXTENSIONREF(readFormatBGRA));
        map["GL_NV_pixel_buffer_object"] = enableableExtension(EXTENSIONREF(pixelBufferObject));
        map["GL_OES_mapbuffer"] = enableableExtension(EXTENSIONREF(mapBuffer));
        map["GL_EXT_map_buffer_range"] = enableableExtension(EXTENSIONREF(mapBufferRange));
        map["GL_EXT_color_buffer_half_float"] = enableableExtension(EXTENSIONREF(colorBufferHalfFloat));
        map["GL_OES_texture_half_float"] = enableableExtension(EXTENSIONREF(textureHalfFloat));
        map["GL_OES_texture_half_float_linear"] = enableableExtension(EXTENSIONREF(textureHalfFloatLinear));
        map["GL_OES_texture_float"] = enableableExtension(EXTENSIONREF(textureFloat));
        map["GL_OES_texture_float_linear"] = enableableExtension(EXTENSIONREF(textureFloatLinear));
        map["GL_EXT_texture_rg"] = enableableExtension(EXTENSIONREF(textureRG));
        map["GL_EXT_texture_compression_dxt1"] = enableableExtension(EXTENSIONREF(textureCompressionDXT1));
        map["GL_ANGLE_texture_compression_dxt3"] = enableableExtension(EXTENSIONREF(textureCompressionDXT3));
        map["GL_ANGLE_texture_compression_dxt5"] = enableableExtension(EXTENSIONREF(textureCompressionDXT5));
        map["GL_EXT_texture_compression_s3tc_srgb"] = enableableExtension(EXTENSIONREF(textureCompressionS3TCsRGB));
        map["GL_KHR_texture_compression_astc_hdr"] = enableableExtension(EXTENSIONREF(textureCompressionASTCHDR));
        map["GL_KHR_texture_compression_astc_ldr"] = enableableExtension(EXTENSIONREF(textureCompressionASTCLDR));
        map["GL_EXT_texture_compression_bptc"] = enableableExtension(EXTENSIONREF(textureCompressionBPTC));
        map["GL_OES_compressed_ETC1_RGB8_texture"] = enableableExtension(EXTENSIONREF(compressedETC1RGB8Texture));
        map["OES_compressed_ETC2_RGB8_texture"] = enableableExtension(EXTENSIONREF(compressedETC2RGB8Texture));
        map["OES_compressed_ETC2_sRGB8_texture"] = enableableExtension(EXTENSIONREF(compressedETC2sRGB8Texture));
        map["OES_compressed_ETC2_punchthroughA_RGBA8_texture"] = enableableExtension(EXTENSIONREF(compressedETC2PunchthroughARGB8Texture));
        map["OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture"] = enableableExtension(EXTENSIONREF(compressedETC2PunchthroughAsRGB8AlphaTexture));
        map["OES_compressed_ETC2_RGBA8_texture"] = enableableExtension(EXTENSIONREF(compressedETC2RGBA8Texture));
        map["OES_compressed_ETC2_sRGB8_alpha8_texture"] = enableableExtension(EXTENSIONREF(compressedETC2sRGB8Alpha8Texture));
        map["OES_compressed_EAC_R11_unsigned_texture"] = enableableExtension(EXTENSIONREF(compressedEACR11UnsignedTexture));
        map["OES_compressed_EAC_R11_signed_texture"] = enableableExtension(EXTENSIONREF(compressedEACR11SignedTexture));
        map["OES_compressed_EAC_RG11_unsigned_texture"] = enableableExtension(EXTENSIONREF(compressedEACRG11UnsignedTexture));
        map["OES_compressed_EAC_RG11_signed_texture"] = enableableExtension(EXTENSIONREF(compressedEACRG11SignedTexture));
        map["GL_CHROMIUM_compressed_texture_etc"] = enableableExtension(EXTENSIONREF(compressedTextureETC));
        map["GL_EXT_sRGB"] = enableableExtension(EXTENSIONREF(sRGB));
        map["GL_ANGLE_depth_texture"] = esOnlyExtension(EXTENSIONREF(depthTextures));
        map["GL_OES_depth32"] = esOnlyExtension(EXTENSIONREF(depth32));
        map["GL_EXT_texture_storage"] = enableableExtension(EXTENSIONREF(textureStorage));
        map["GL_OES_texture_npot"] = enableableExtension(EXTENSIONREF(textureNPOT));
        map["GL_EXT_draw_buffers"] = enableableExtension(EXTENSIONREF(drawBuffers));
        map["GL_EXT_texture_filter_anisotropic"] = enableableExtension(EXTENSIONREF(textureFilterAnisotropic));
        map["GL_EXT_occlusion_query_boolean"] = enableableExtension(EXTENSIONREF(occlusionQueryBoolean));
        map["GL_NV_fence"] = esOnlyExtension(EXTENSIONREF(fence));
        map["GL_EXT_disjoint_timer_query"] = enableableExtension(EXTENSIONREF(disjointTimerQuery));
        map["GL_EXT_robustness"] = esOnlyExtension(EXTENSIONREF(robustness));
        map["GL_KHR_robust_buffer_access_behavior"] = esOnlyExtension(EXTENSIONREF(robustBufferAccessBehavior));
        map["GL_EXT_blend_minmax"] = enableableExtension(EXTENSIONREF(blendMinMax));
        map["GL_ANGLE_framebuffer_blit"] = enableableExtension(EXTENSIONREF(framebufferBlit));
        map["GL_ANGLE_framebuffer_multisample"] = enableableExtension(EXTENSIONREF(framebufferMultisample));
        map["GL_ANGLE_instanced_arrays"] = enableableExtension(EXTENSIONREF(instancedArrays.angle));
        map["GL_EXT_instanced_arrays"] = enableableExtension(EXTENSIONREF(instancedArrays.ext));
        map["GL_ANGLE_pack_reverse_row_order"] = enableableExtension(EXTENSIONREF(packReverseRowOrder));
        map["GL_OES_standard_derivatives"] = enableableExtension(EXTENSIONREF(standardDerivatives));
        map["GL_EXT_shader_texture_lod"] = enableableExtension(EXTENSIONREF(shaderTextureLOD));
        map["GL_EXT_frag_depth"] = enableableExtension(EXTENSIONREF(fragDepth));
        map["GL_ANGLE_multiview"] = enableableExtension(EXTENSIONREF(multiview));
        map["GL_ANGLE_texture_usage"] = enableableExtension(EXTENSIONREF(textureUsage));
        map["GL_ANGLE_translated_shader_source"] = esOnlyExtension(EXTENSIONREF(translatedShaderSource));
        map["GL_OES_fbo_render_mipmap"] = enableableExtension(EXTENSIONREF(fboRenderMipmap));
        map["GL_EXT_discard_framebuffer"] = esOnlyExtension(EXTENSIONREF(discardFramebuffer));
        map["GL_EXT_debug_marker"] = esOnlyExtension(EXTENSIONREF(debugMarker));
        map["GL_OES_EGL_image"] = enableableExtension(EXTENSIONREF(eglImage));
        map["GL_OES_EGL_image_external"] = enableableExtension(EXTENSIONREF(eglImageExternal));
        map["GL_OES_EGL_image_external_essl3"] = enableableExtension(EXTENSIONREF(eglImageExternalEssl3));
        map["GL_OES_EGL_sync"] = esOnlyExtension(EXTENSIONREF(eglSync));
        map["GL_NV_EGL_stream_consumer_external"] = enableableExtension(EXTENSIONREF(eglStreamConsumerExternal));
        map["GL_EXT_unpack_subimage"] = enableableExtension(EXTENSIONREF(unpackSubimage));
        map["GL_NV_pack_subimage"] = enableableExtension(EXTENSIONREF(packSubimage));
        map["GL_EXT_color_buffer_float"] = enableableExtension(EXTENSIONREF(colorBufferFloat));
        map["GL_OES_vertex_array_object"] = enableableExtension(EXTENSIONREF(vertexArrayObject));
        map["GL_KHR_debug"] = esOnlyExtension(EXTENSIONREF(debug));
        map["GL_OES_texture_border_clamp"] = enableableExtension(EXTENSIONREF(textureBorderClamp));
        // TODO(jmadill): Enable this when complete.
        //map["GL_KHR_no_error"] = esOnlyExtension(EXTENSIONREF(noError));
        map["GL_ANGLE_lossy_etc_decode"] = enableableExtension(EXTENSIONREF(lossyETCDecode));
        map["GL_CHROMIUM_bind_uniform_location"] = esOnlyExtension(EXTENSIONREF(bindUniformLocation));
        map["GL_CHROMIUM_sync_query"] = enableableExtension(EXTENSIONREF(syncQuery));
        map["GL_CHROMIUM_copy_texture"] = esOnlyExtension(EXTENSIONREF(copyTexture));
        map["GL_CHROMIUM_copy_compressed_texture"] = esOnlyExtension(EXTENSIONREF(copyCompressedTexture));
        map["GL_ANGLE_copy_texture_3d"] = enableableExtension(EXTENSIONREF(copyTexture3d));
        map["GL_ANGLE_webgl_compatibility"] = esOnlyExtension(EXTENSIONREF(webglCompatibility));
        map["GL_ANGLE_request_extension"] = esOnlyExtension(EXTENSIONREF(requestExtension));
        map["GL_CHROMIUM_bind_generates_resource"] = esOnlyExtension(EXTENSIONREF(bindGeneratesResource));
        map["GL_ANGLE_robust_client_memory"] = esOnlyExtension(EXTENSIONREF(robustClientMemory));
        map["GL_EXT_texture_sRGB_decode"] = esOnlyExtension(EXTENSIONREF(textureSRGBDecode));
        map["GL_EXT_sRGB_write_control"] = esOnlyExtension(EXTENSIONREF(sRGBWriteControl));
        map["GL_CHROMIUM_color_buffer_float_rgb"] = enableableExtension(EXTENSIONREF(colorBufferFloatRGB));
        map["GL_CHROMIUM_color_buffer_float_rgba"] = enableableExtension(EXTENSIONREF(colorBufferFloatRGBA));
        map["GL_EXT_multisample_compatibility"] = esOnlyExtension(EXTENSIONREF(multisampleCompatibility));
        map["GL_CHROMIUM_framebuffer_mixed_samples"] = esOnlyExtension(EXTENSIONREF(framebufferMixedSamples));
        map["GL_EXT_texture_norm16"] = esOnlyExtension(EXTENSIONREF(textureNorm16));
        map["GL_CHROMIUM_path_rendering"] = esOnlyExtension(EXTENSIONREF(pathRendering));
        map["GL_OES_surfaceless_context"] = esOnlyExtension(EXTENSIONREF(surfacelessContext));
        map["GL_ANGLE_client_arrays"] = esOnlyExtension(EXTENSIONREF(clientArrays));
        map["GL_ANGLE_robust_resource_initialization"] = esOnlyExtension(EXTENSIONREF(robustResourceInitialization));
        map["GL_ANGLE_program_cache_control"] = esOnlyExtension(EXTENSIONREF(programCacheControl));
        map["GL_ANGLE_texture_rectangle"] = enableableExtension(EXTENSIONREF(textureRectangle));
        map["GL_EXT_geometry_shader"] = enableableExtension(EXTENSIONREF(geometryShader));
        map["GL_ANGLE_explicit_context_gles1"] = enableableExtension(EXTENSIONREF(explicitContextGles1));
        map["GL_ANGLE_explicit_context"] = enableableExtension(EXTENSIONREF(explicitContext));
        map["GL_KHR_parallel_shader_compile"] = enableableExtension(EXTENSIONREF(parallelShaderCompile));
        map["GL_OES_texture_storage_multisample_2d_array"] = enableableExtension(EXTENSIONREF(textureStorageMultisample2DArray));
        map["GL_ANGLE_multiview_multisample"] = enableableExtension(EXTENSIONREF(multiviewMultisample));
        map["GL_EXT_blend_func_extended"] = enableableExtension(EXTENSIONREF(blendFuncExtended));
        map["GL_ANGLE_texture_multisample"] = enableableExtension(EXTENSIONREF(textureMultisample));
        map["GL_ANGLE_multi_draw"] = enableableExtension(EXTENSIONREF(multiDraw));
        map["GL_ANGLE_provoking_vertex"] = enableableExtension(EXTENSIONREF(provokingVertex));
        // GLES1 extensinos
        map["GL_OES_point_size_array"] = enableableExtension(EXTENSIONREF(pointSizeArray));
        map["GL_OES_texture_cube_map"] = enableableExtension(EXTENSIONREF(textureCubeMap));
        map["GL_OES_point_sprite"] = enableableExtension(EXTENSIONREF(pointSprite));
        map["GL_OES_draw_texture"] = enableableExtension(EXTENSIONREF(drawTexture));
        map["GL_ANGLE_memory_size"] = enableableExtension(EXTENSIONREF(memorySize));
        // clang-format on

        return map;
    };

    static const ExtensionInfoMap extensionInfo = buildExtensionInfoMap();
    return extensionInfo;
}

TypePrecision::TypePrecision() : range({{0, 0}}), precision(0) {}

TypePrecision::TypePrecision(const TypePrecision &other) = default;

void TypePrecision::setIEEEFloat()
{
    range     = {{127, 127}};
    precision = 23;
}

void TypePrecision::setTwosComplementInt(unsigned int bits)
{
    range     = {{static_cast<GLint>(bits) - 1, static_cast<GLint>(bits) - 2}};
    precision = 0;
}

void TypePrecision::setSimulatedFloat(unsigned int r, unsigned int p)
{
    range     = {{static_cast<GLint>(r), static_cast<GLint>(r)}};
    precision = static_cast<GLint>(p);
}

void TypePrecision::setSimulatedInt(unsigned int r)
{
    range     = {{static_cast<GLint>(r), static_cast<GLint>(r)}};
    precision = 0;
}

void TypePrecision::get(GLint *returnRange, GLint *returnPrecision) const
{
    std::copy(range.begin(), range.end(), returnRange);
    *returnPrecision = precision;
}

Caps::Caps()
    : maxElementIndex(0),
      max3DTextureSize(0),
      max2DTextureSize(0),
      maxRectangleTextureSize(0),
      maxArrayTextureLayers(0),
      maxLODBias(0),
      maxCubeMapTextureSize(0),
      maxRenderbufferSize(0),
      minAliasedPointSize(1.0f),
      maxAliasedPointSize(1.0f),
      minAliasedLineWidth(0),
      maxAliasedLineWidth(0),

      // Table 20.40
      maxDrawBuffers(0),
      maxFramebufferWidth(0),
      maxFramebufferHeight(0),
      maxFramebufferSamples(0),
      maxColorAttachments(0),
      maxViewportWidth(0),
      maxViewportHeight(0),
      maxSampleMaskWords(0),
      maxColorTextureSamples(0),
      maxDepthTextureSamples(0),
      maxIntegerSamples(0),
      maxServerWaitTimeout(0),

      // Table 20.41
      maxVertexAttribRelativeOffset(0),
      maxVertexAttribBindings(0),
      maxVertexAttribStride(0),
      maxElementsIndices(0),
      maxElementsVertices(0),

      // Table 20.43
      // Table 20.44
      // Table 20.45
      // Table 20.43gs (GL_EXT_geometry_shader)
      // Table 20.46 (GL_EXT_geometry_shader)
      maxShaderUniformBlocks({}),
      maxShaderTextureImageUnits({}),
      maxShaderStorageBlocks({}),
      maxShaderUniformComponents({}),
      maxShaderAtomicCounterBuffers({}),
      maxShaderAtomicCounters({}),
      maxShaderImageUniforms({}),
      maxCombinedShaderUniformComponents({}),

      // Table 20.43
      maxVertexAttributes(0),
      maxVertexUniformVectors(0),
      maxVertexOutputComponents(0),

      // Table 20.44
      maxFragmentUniformVectors(0),
      maxFragmentInputComponents(0),
      minProgramTextureGatherOffset(0),
      maxProgramTextureGatherOffset(0),
      minProgramTexelOffset(0),
      maxProgramTexelOffset(0),

      // Table 20.45
      maxComputeWorkGroupInvocations(0),
      maxComputeSharedMemorySize(0),

      // Table 20.46
      maxUniformBufferBindings(0),
      maxUniformBlockSize(0),
      uniformBufferOffsetAlignment(0),
      maxCombinedUniformBlocks(0),
      maxVaryingComponents(0),
      maxVaryingVectors(0),
      maxCombinedTextureImageUnits(0),
      maxCombinedShaderOutputResources(0),

      // Table 20.47
      maxUniformLocations(0),
      maxAtomicCounterBufferBindings(0),
      maxAtomicCounterBufferSize(0),
      maxCombinedAtomicCounterBuffers(0),
      maxCombinedAtomicCounters(0),
      maxImageUnits(0),
      maxCombinedImageUniforms(0),
      maxShaderStorageBufferBindings(0),
      maxShaderStorageBlockSize(0),
      maxCombinedShaderStorageBlocks(0),
      shaderStorageBufferOffsetAlignment(0),

      // Table 20.48
      maxTransformFeedbackInterleavedComponents(0),
      maxTransformFeedbackSeparateAttributes(0),
      maxTransformFeedbackSeparateComponents(0),

      // Table 20.49
      maxSamples(0),

      // Table 20.40 (cont.) (GL_EXT_geometry_shader)
      maxFramebufferLayers(0),
      layerProvokingVertex(0),

      // Table 20.43gs (GL_EXT_geometry_shader)
      maxGeometryInputComponents(0),
      maxGeometryOutputComponents(0),
      maxGeometryOutputVertices(0),
      maxGeometryTotalOutputComponents(0),
      maxGeometryShaderInvocations(0),

      // GLES1 emulation: Table 6.20 / 6.22 (ES 1.1 spec)
      maxMultitextureUnits(0),
      maxClipPlanes(0),
      maxLights(0),
      maxModelviewMatrixStackDepth(0),
      maxProjectionMatrixStackDepth(0),
      maxTextureMatrixStackDepth(0),
      minSmoothPointSize(0),
      maxSmoothPointSize(0),
      minSmoothLineWidth(0),
      maxSmoothLineWidth(0)

{
    for (size_t i = 0; i < 3; ++i)
    {
        maxComputeWorkGroupCount[i] = 0;
        maxComputeWorkGroupSize[i]  = 0;
    }
}

Caps::Caps(const Caps &other) = default;
Caps::~Caps()                 = default;

Caps GenerateMinimumCaps(const Version &clientVersion, const Extensions &extensions)
{
    Caps caps;

    // GLES1 emulation (Minimums taken from Table 6.20 / 6.22 (ES 1.1 spec))
    if (clientVersion < Version(2, 0))
    {
        caps.maxMultitextureUnits = 2;
        caps.maxLights            = 8;
        caps.maxClipPlanes        = 1;

        caps.maxModelviewMatrixStackDepth  = 16;
        caps.maxProjectionMatrixStackDepth = 2;
        caps.maxTextureMatrixStackDepth    = 2;

        caps.minSmoothPointSize = 1.0f;
        caps.maxSmoothPointSize = 1.0f;
    }

    if (clientVersion >= Version(2, 0))
    {
        // Table 6.18
        caps.max2DTextureSize      = 64;
        caps.maxCubeMapTextureSize = 16;
        caps.maxViewportWidth      = caps.max2DTextureSize;
        caps.maxViewportHeight     = caps.max2DTextureSize;
        caps.minAliasedPointSize   = 1;
        caps.maxAliasedPointSize   = 1;
        caps.minAliasedLineWidth   = 1;
        caps.maxAliasedLineWidth   = 1;

        // Table 6.19
        caps.vertexHighpFloat.setSimulatedFloat(62, 16);
        caps.vertexMediumpFloat.setSimulatedFloat(14, 10);
        caps.vertexLowpFloat.setSimulatedFloat(1, 8);
        caps.vertexHighpInt.setSimulatedInt(16);
        caps.vertexMediumpInt.setSimulatedInt(10);
        caps.vertexLowpInt.setSimulatedInt(8);
        caps.fragmentHighpFloat.setSimulatedFloat(62, 16);
        caps.fragmentMediumpFloat.setSimulatedFloat(14, 10);
        caps.fragmentLowpFloat.setSimulatedFloat(1, 8);
        caps.fragmentHighpInt.setSimulatedInt(16);
        caps.fragmentMediumpInt.setSimulatedInt(10);
        caps.fragmentLowpInt.setSimulatedInt(8);

        // Table 6.20
        caps.maxVertexAttributes                              = 8;
        caps.maxVertexUniformVectors                          = 128;
        caps.maxVaryingVectors                                = 8;
        caps.maxCombinedTextureImageUnits                     = 8;
        caps.maxShaderTextureImageUnits[ShaderType::Fragment] = 8;
        caps.maxFragmentUniformVectors                        = 16;
        caps.maxRenderbufferSize                              = 1;
    }

    if (clientVersion >= Version(3, 0))
    {
        // Table 6.28
        caps.maxElementIndex       = (1 << 24) - 1;
        caps.max3DTextureSize      = 256;
        caps.max2DTextureSize      = 2048;
        caps.maxArrayTextureLayers = 256;
        caps.maxLODBias            = 2.0f;
        caps.maxCubeMapTextureSize = 2048;
        caps.maxRenderbufferSize   = 2048;
        caps.maxDrawBuffers        = 4;
        caps.maxColorAttachments   = 4;
        caps.maxViewportWidth      = caps.max2DTextureSize;
        caps.maxViewportHeight     = caps.max2DTextureSize;

        // Table 6.29
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_R11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SIGNED_R11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RG11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SIGNED_RG11_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGB8_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_RGBA8_ETC2_EAC);
        caps.compressedTextureFormats.push_back(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
        caps.vertexHighpFloat.setIEEEFloat();
        caps.vertexHighpInt.setTwosComplementInt(32);
        caps.vertexMediumpInt.setTwosComplementInt(16);
        caps.vertexLowpInt.setTwosComplementInt(8);
        caps.fragmentHighpFloat.setIEEEFloat();
        caps.fragmentHighpInt.setSimulatedInt(32);
        caps.fragmentMediumpInt.setTwosComplementInt(16);
        caps.fragmentLowpInt.setTwosComplementInt(8);
        caps.maxServerWaitTimeout = 0;

        // Table 6.31
        caps.maxVertexAttributes                            = 16;
        caps.maxShaderUniformComponents[ShaderType::Vertex] = 1024;
        caps.maxVertexUniformVectors                        = 256;
        caps.maxShaderUniformBlocks[ShaderType::Vertex]     = 12;
        caps.maxVertexOutputComponents                      = 64;
        caps.maxShaderTextureImageUnits[ShaderType::Vertex] = 16;

        // Table 6.32
        caps.maxShaderUniformComponents[ShaderType::Fragment] = 896;
        caps.maxFragmentUniformVectors                        = 224;
        caps.maxShaderUniformBlocks[ShaderType::Fragment]     = 12;
        caps.maxFragmentInputComponents                       = 60;
        caps.maxShaderTextureImageUnits[ShaderType::Fragment] = 16;
        caps.minProgramTexelOffset                            = -8;
        caps.maxProgramTexelOffset                            = 7;

        // Table 6.33
        caps.maxUniformBufferBindings     = 24;
        caps.maxUniformBlockSize          = 16384;
        caps.uniformBufferOffsetAlignment = 256;
        caps.maxCombinedUniformBlocks     = 24;
        caps.maxVaryingComponents         = 60;
        caps.maxVaryingVectors            = 15;
        caps.maxCombinedTextureImageUnits = 32;

        // Table 6.34
        caps.maxTransformFeedbackInterleavedComponents = 64;
        caps.maxTransformFeedbackSeparateAttributes    = 4;
        caps.maxTransformFeedbackSeparateComponents    = 4;

        // Table 3.35
        caps.maxSamples = 4;
    }

    if (clientVersion >= Version(3, 1))
    {
        // Table 20.40
        caps.maxFramebufferWidth    = 2048;
        caps.maxFramebufferHeight   = 2048;
        caps.maxFramebufferSamples  = 4;
        caps.maxSampleMaskWords     = 1;
        caps.maxColorTextureSamples = 1;
        caps.maxDepthTextureSamples = 1;
        caps.maxIntegerSamples      = 1;

        // Table 20.41
        caps.maxVertexAttribRelativeOffset = 2047;
        caps.maxVertexAttribBindings       = 16;
        caps.maxVertexAttribStride         = 2048;

        // Table 20.43
        caps.maxShaderAtomicCounterBuffers[ShaderType::Vertex] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Vertex]       = 0;
        caps.maxShaderImageUniforms[ShaderType::Vertex]        = 0;
        caps.maxShaderStorageBlocks[ShaderType::Vertex]        = 0;

        // Table 20.44
        caps.maxShaderUniformComponents[ShaderType::Fragment]    = 1024;
        caps.maxFragmentUniformVectors                           = 256;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Fragment] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Fragment]       = 0;
        caps.maxShaderImageUniforms[ShaderType::Fragment]        = 0;
        caps.maxShaderStorageBlocks[ShaderType::Fragment]        = 0;
        caps.minProgramTextureGatherOffset                       = 0;
        caps.maxProgramTextureGatherOffset                       = 0;

        // Table 20.45
        caps.maxComputeWorkGroupCount                           = {{65535, 65535, 65535}};
        caps.maxComputeWorkGroupSize                            = {{128, 128, 64}};
        caps.maxComputeWorkGroupInvocations                     = 12;
        caps.maxShaderUniformBlocks[ShaderType::Compute]        = 12;
        caps.maxShaderTextureImageUnits[ShaderType::Compute]    = 16;
        caps.maxComputeSharedMemorySize                         = 16384;
        caps.maxShaderUniformComponents[ShaderType::Compute]    = 1024;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Compute] = 1;
        caps.maxShaderAtomicCounters[ShaderType::Compute]       = 8;
        caps.maxShaderImageUniforms[ShaderType::Compute]        = 4;
        caps.maxShaderStorageBlocks[ShaderType::Compute]        = 4;

        // Table 20.46
        caps.maxUniformBufferBindings         = 36;
        caps.maxCombinedTextureImageUnits     = 48;
        caps.maxCombinedShaderOutputResources = 4;

        // Table 20.47
        caps.maxUniformLocations                = 1024;
        caps.maxAtomicCounterBufferBindings     = 1;
        caps.maxAtomicCounterBufferSize         = 32;
        caps.maxCombinedAtomicCounterBuffers    = 1;
        caps.maxCombinedAtomicCounters          = 8;
        caps.maxImageUnits                      = 4;
        caps.maxCombinedImageUniforms           = 4;
        caps.maxShaderStorageBufferBindings     = 4;
        caps.maxShaderStorageBlockSize          = 1 << 27;
        caps.maxCombinedShaderStorageBlocks     = 4;
        caps.shaderStorageBufferOffsetAlignment = 256;
    }

    if (extensions.textureRectangle)
    {
        caps.maxRectangleTextureSize = 64;
    }

    if (extensions.geometryShader)
    {
        // Table 20.40 (GL_EXT_geometry_shader)
        caps.maxFramebufferLayers = 256;
        caps.layerProvokingVertex = GL_LAST_VERTEX_CONVENTION_EXT;

        // Table 20.43gs (GL_EXT_geometry_shader)
        caps.maxShaderUniformComponents[ShaderType::Geometry]    = 1024;
        caps.maxShaderUniformBlocks[ShaderType::Geometry]        = 12;
        caps.maxGeometryInputComponents                          = 64;
        caps.maxGeometryOutputComponents                         = 64;
        caps.maxGeometryOutputVertices                           = 256;
        caps.maxGeometryTotalOutputComponents                    = 1024;
        caps.maxShaderTextureImageUnits[ShaderType::Geometry]    = 16;
        caps.maxShaderAtomicCounterBuffers[ShaderType::Geometry] = 0;
        caps.maxShaderAtomicCounters[ShaderType::Geometry]       = 0;
        caps.maxShaderStorageBlocks[ShaderType::Geometry]        = 0;
        caps.maxGeometryShaderInvocations                        = 32;

        // Table 20.46 (GL_EXT_geometry_shader)
        caps.maxShaderImageUniforms[ShaderType::Geometry] = 0;

        // Table 20.46 (GL_EXT_geometry_shader)
        caps.maxUniformBufferBindings     = 48;
        caps.maxCombinedUniformBlocks     = 36;
        caps.maxCombinedTextureImageUnits = 64;
    }

    for (ShaderType shaderType : AllShaderTypes())
    {
        caps.maxCombinedShaderUniformComponents[shaderType] =
            caps.maxShaderUniformBlocks[shaderType] *
                static_cast<GLuint>(caps.maxUniformBlockSize / 4) +
            caps.maxShaderUniformComponents[shaderType];
    }

    return caps;
}
}  // namespace gl

namespace egl
{

Caps::Caps() : textureNPOT(false) {}

DisplayExtensions::DisplayExtensions()
    : createContextRobustness(false),
      d3dShareHandleClientBuffer(false),
      d3dTextureClientBuffer(false),
      surfaceD3DTexture2DShareHandle(false),
      querySurfacePointer(false),
      windowFixedSize(false),
      keyedMutex(false),
      surfaceOrientation(false),
      postSubBuffer(false),
      createContext(false),
      deviceQuery(false),
      image(false),
      imageBase(false),
      imagePixmap(false),
      glTexture2DImage(false),
      glTextureCubemapImage(false),
      glTexture3DImage(false),
      glRenderbufferImage(false),
      getAllProcAddresses(false),
      flexibleSurfaceCompatibility(false),
      directComposition(false),
      windowsUIComposition(false),
      createContextNoError(false),
      stream(false),
      streamConsumerGLTexture(false),
      streamConsumerGLTextureYUV(false),
      streamProducerD3DTexture(false),
      fenceSync(false),
      waitSync(false),
      createContextWebGLCompatibility(false),
      createContextBindGeneratesResource(false),
      getSyncValues(false),
      swapBuffersWithDamage(false),
      pixelFormatFloat(false),
      surfacelessContext(false),
      displayTextureShareGroup(false),
      createContextClientArrays(false),
      programCacheControl(false),
      robustResourceInitialization(false),
      iosurfaceClientBuffer(false),
      createContextExtensionsEnabled(false),
      presentationTime(false),
      blobCache(false),
      imageNativeBuffer(false),
      getFrameTimestamps(false),
      recordable(false)
{}

std::vector<std::string> DisplayExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                       | Supported flag                    | Output vector   |
    InsertExtensionString("EGL_EXT_create_context_robustness",                   createContextRobustness,            &extensionStrings);
    InsertExtensionString("EGL_ANGLE_d3d_share_handle_client_buffer",            d3dShareHandleClientBuffer,         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_d3d_texture_client_buffer",                 d3dTextureClientBuffer,             &extensionStrings);
    InsertExtensionString("EGL_ANGLE_surface_d3d_texture_2d_share_handle",       surfaceD3DTexture2DShareHandle,     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_query_surface_pointer",                     querySurfacePointer,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_window_fixed_size",                         windowFixedSize,                    &extensionStrings);
    InsertExtensionString("EGL_ANGLE_keyed_mutex",                               keyedMutex,                         &extensionStrings);
    InsertExtensionString("EGL_ANGLE_surface_orientation",                       surfaceOrientation,                 &extensionStrings);
    InsertExtensionString("EGL_ANGLE_direct_composition",                        directComposition,                  &extensionStrings);
    InsertExtensionString("EGL_ANGLE_windows_ui_composition",                    windowsUIComposition,               &extensionStrings);
    InsertExtensionString("EGL_NV_post_sub_buffer",                              postSubBuffer,                      &extensionStrings);
    InsertExtensionString("EGL_KHR_create_context",                              createContext,                      &extensionStrings);
    InsertExtensionString("EGL_EXT_device_query",                                deviceQuery,                        &extensionStrings);
    InsertExtensionString("EGL_KHR_image",                                       image,                              &extensionStrings);
    InsertExtensionString("EGL_KHR_image_base",                                  imageBase,                          &extensionStrings);
    InsertExtensionString("EGL_KHR_image_pixmap",                                imagePixmap,                        &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_2D_image",                         glTexture2DImage,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_cubemap_image",                    glTextureCubemapImage,              &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_texture_3D_image",                         glTexture3DImage,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_gl_renderbuffer_image",                       glRenderbufferImage,                &extensionStrings);
    InsertExtensionString("EGL_KHR_get_all_proc_addresses",                      getAllProcAddresses,                &extensionStrings);
    InsertExtensionString("EGL_KHR_stream",                                      stream,                             &extensionStrings);
    InsertExtensionString("EGL_KHR_stream_consumer_gltexture",                   streamConsumerGLTexture,            &extensionStrings);
    InsertExtensionString("EGL_NV_stream_consumer_gltexture_yuv",                streamConsumerGLTextureYUV,         &extensionStrings);
    InsertExtensionString("EGL_KHR_fence_sync",                                  fenceSync,                          &extensionStrings);
    InsertExtensionString("EGL_KHR_wait_sync",                                   waitSync,                           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_flexible_surface_compatibility",            flexibleSurfaceCompatibility,       &extensionStrings);
    InsertExtensionString("EGL_ANGLE_stream_producer_d3d_texture",               streamProducerD3DTexture,           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_webgl_compatibility",        createContextWebGLCompatibility,    &extensionStrings);
    InsertExtensionString("EGL_CHROMIUM_create_context_bind_generates_resource", createContextBindGeneratesResource, &extensionStrings);
    InsertExtensionString("EGL_CHROMIUM_sync_control",                           getSyncValues,                      &extensionStrings);
    InsertExtensionString("EGL_KHR_swap_buffers_with_damage",                    swapBuffersWithDamage,              &extensionStrings);
    InsertExtensionString("EGL_EXT_pixel_format_float",                          pixelFormatFloat,                   &extensionStrings);
    InsertExtensionString("EGL_KHR_surfaceless_context",                         surfacelessContext,                 &extensionStrings);
    InsertExtensionString("EGL_ANGLE_display_texture_share_group",               displayTextureShareGroup,           &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_client_arrays",              createContextClientArrays,          &extensionStrings);
    InsertExtensionString("EGL_ANGLE_program_cache_control",                     programCacheControl,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_robust_resource_initialization",            robustResourceInitialization,       &extensionStrings);
    InsertExtensionString("EGL_ANGLE_iosurface_client_buffer",                   iosurfaceClientBuffer,              &extensionStrings);
    InsertExtensionString("EGL_ANGLE_create_context_extensions_enabled",         createContextExtensionsEnabled,     &extensionStrings);
    InsertExtensionString("EGL_ANDROID_presentation_time",                       presentationTime,                   &extensionStrings);
    InsertExtensionString("EGL_ANDROID_blob_cache",                              blobCache,                          &extensionStrings);
    InsertExtensionString("EGL_ANDROID_image_native_buffer",                     imageNativeBuffer,                  &extensionStrings);
    InsertExtensionString("EGL_ANDROID_get_frame_timestamps",                    getFrameTimestamps,                 &extensionStrings);
    InsertExtensionString("EGL_ANDROID_recordable",                              recordable,                 &extensionStrings);
    // TODO(jmadill): Enable this when complete.
    //InsertExtensionString("KHR_create_context_no_error",                       createContextNoError,               &extensionStrings);
    // clang-format on

    return extensionStrings;
}

DeviceExtensions::DeviceExtensions() : deviceD3D(false) {}

std::vector<std::string> DeviceExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                 | Supported flag                | Output vector   |
    InsertExtensionString("EGL_ANGLE_device_d3d",                          deviceD3D,                      &extensionStrings);
    // clang-format on

    return extensionStrings;
}

ClientExtensions::ClientExtensions()
    : clientExtensions(false),
      platformBase(false),
      platformDevice(false),
      platformANGLE(false),
      platformANGLED3D(false),
      platformANGLEOpenGL(false),
      platformANGLEVulkan(false),
      platformANGLEContextVirtualization(false),
      deviceCreation(false),
      deviceCreationD3D11(false),
      x11Visual(false),
      experimentalPresentPath(false),
      clientGetAllProcAddresses(false),
      debug(false),
      explicitContext(false)
{}

ClientExtensions::ClientExtensions(const ClientExtensions &other) = default;

std::vector<std::string> ClientExtensions::getStrings() const
{
    std::vector<std::string> extensionStrings;

    // clang-format off
    //                   | Extension name                                   | Supported flag                   | Output vector   |
    InsertExtensionString("EGL_EXT_client_extensions",                       clientExtensions,                   &extensionStrings);
    InsertExtensionString("EGL_EXT_platform_base",                           platformBase,                       &extensionStrings);
    InsertExtensionString("EGL_EXT_platform_device",                         platformDevice,                     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle",                        platformANGLE,                      &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_d3d",                    platformANGLED3D,                   &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_opengl",                 platformANGLEOpenGL,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_null",                   platformANGLENULL,                  &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_vulkan",                 platformANGLEVulkan,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_platform_angle_context_virtualization", platformANGLEContextVirtualization, &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_creation",                       deviceCreation,                     &extensionStrings);
    InsertExtensionString("EGL_ANGLE_device_creation_d3d11",                 deviceCreationD3D11,                &extensionStrings);
    InsertExtensionString("EGL_ANGLE_x11_visual",                            x11Visual,                          &extensionStrings);
    InsertExtensionString("EGL_ANGLE_experimental_present_path",             experimentalPresentPath,            &extensionStrings);
    InsertExtensionString("EGL_KHR_client_get_all_proc_addresses",           clientGetAllProcAddresses,          &extensionStrings);
    InsertExtensionString("EGL_KHR_debug",                                   debug,                              &extensionStrings);
    InsertExtensionString("EGL_ANGLE_explicit_context",                      explicitContext,                    &extensionStrings);
    // clang-format on

    return extensionStrings;
}

}  // namespace egl
