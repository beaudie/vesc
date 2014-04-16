//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libGLESv2/caps_utils.h"
#include "libGLESv2/formatutils.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <algorithm>

namespace gl
{

void InsertExtensionString(const std::string &extension, GLuint minClientVersion, bool supported, GLuint curClientVersion,
                           std::vector<std::string> *extensionVector)
{
    if (supported && minClientVersion >= curClientVersion)
    {
        extensionVector->push_back(extension);
    }
}

std::vector<std::string> GetExtensionStrings(const Extensions &extensions, const GLuint clientVersion)
{
    std::vector<std::string> extensionStrings;

    InsertExtensionString("GL_OES_element_index_uint",         2, extensions.elementIndexUint,         clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_packed_depth_stencil",       2, extensions.packedDepthStencil,       clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_get_program_binary",         2, extensions.getProgramBinary,         clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_rgb8_rgba8",                 2, extensions.rgb8rgba8,                clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_texture_format_BGRA8888",    2, extensions.textureFormatBGRA8888,    clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_read_format_bgra",           2, extensions.readFormatBGRA,           clientVersion, &extensionStrings);
    InsertExtensionString("GL_NV_pixel_buffer_object",         2, extensions.pixelBufferObject,        clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_mapbuffer",                  2, extensions.mapBuffer,                clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_map_buffer_range",           2, extensions.mapBufferRange,           clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_texture_half_float",         2, extensions.textureHalfFloat,         clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_texture_half_float_linear",  2, extensions.textureHalfFloatLinear,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_texture_float",              2, extensions.textureFloat,             clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_texture_float_linear",       2, extensions.textureFloatLinear,       clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_texture_rg",                 2, extensions.textureRG,                clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_texture_compression_dxt1",   2, extensions.textureCompressionDXT1,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_texture_compression_dxt3", 2, extensions.textureCompressionDXT3,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_texture_compression_dxt5", 2, extensions.textureCompressionDXT5,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_depth_texture",            2, extensions.depthTextures,            clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_texture_storage",            2, extensions.textureStorage,           clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_texture_npot",               2, extensions.textureNPOT,              clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_draw_buffers",               2, extensions.drawBuffers,              clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_texture_filter_anisotropic", 2, extensions.textureFilterAnisotropic, clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_occlusion_query_boolean",    2, extensions.occlusionQueryBoolean,    clientVersion, &extensionStrings);
    InsertExtensionString("GL_NV_fence",                       2, extensions.fence,                    clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_timer_query",              2, extensions.timerQuery,               clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_robustness",                 2, extensions.robustness,               clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_blend_minmax",               2, extensions.blendMinMax,              clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_framebuffer_blit",         2, extensions.framebufferBlit,          clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_framebuffer_multisample",  2, extensions.framebufferMultisample,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_instanced_arrays",         2, extensions.instancedArrays,          clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_pack_reverse_row_order",   2, extensions.packReverseRowOrder,      clientVersion, &extensionStrings);
    InsertExtensionString("GL_OES_standard_derivatives",       2, extensions.standardDerivatives,      clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_shader_texture_lod",         2, extensions.shaderTextureLOD,         clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_frag_depth",                 2, extensions.fragDepth,                clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_texture_usage",            2, extensions.textureUsage,             clientVersion, &extensionStrings);
    InsertExtensionString("GL_ANGLE_translated_shader_source", 2, extensions.translatedShaderSource,   clientVersion, &extensionStrings);
    InsertExtensionString("GL_EXT_color_buffer_float",         3, extensions.colorBufferFloat,         clientVersion, &extensionStrings);

    return extensionStrings;
}

static bool GetFormatSupport(const TextureFormatCapsMap &textureCaps, const std::vector<GLenum> &requiredFormats,
                             bool requiresFiltering, bool requiresColorBuffer, bool requiresDepthStencil)
{
    for (size_t i = 0; i < requiredFormats.size(); i++)
    {
        const TextureCaps &cap = GetTextureCaps(textureCaps, requiredFormats[i]);

        if (requiresFiltering && !cap.filtering)
        {
            return false;
        }

        if (requiresColorBuffer && !cap.colorRendering)
        {
            return false;
        }

        if (requiresDepthStencil && !cap.depthRendering)
        {
            return false;
        }
    }

    return true;
}

const TextureCaps &GetTextureCaps(const TextureFormatCapsMap &textureCaps, GLenum internalFormat)
{
    static TextureCaps defaultUnsupportedTexture;
    TextureFormatCapsMap::const_iterator iter = textureCaps.find(internalFormat);
    return (iter != textureCaps.end()) ? iter->second : defaultUnsupportedTexture;
}

bool GetRGB8AndRGBA8TextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_RGB8);
    requiredFormats.push_back(GL_RGBA8);

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false);
}

bool GetBGRA8TextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_BGRA8_EXT);

    return GetFormatSupport(textureCaps, requiredFormats, true, true, false);
}

bool GetHalfFloatTextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_RGB16F);
    requiredFormats.push_back(GL_RGBA16F);

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false);
}

bool GetHalfFloatTextureFilteringSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_RGB16F);
    requiredFormats.push_back(GL_RGBA16F);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetFloatTextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_RGB32F);
    requiredFormats.push_back(GL_RGBA32F);

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false);
}

bool GetFloatTextureFilteringSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_RGB32F);
    requiredFormats.push_back(GL_RGBA32F);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetRGTextureSupport(const TextureFormatCapsMap &textureCaps, bool checkHalfFloatFormats, bool checkFloatFormats)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_R8);
    requiredFormats.push_back(GL_RG8);
    if (checkHalfFloatFormats)
    {
        requiredFormats.push_back(GL_R16F);
        requiredFormats.push_back(GL_RG16F);
    }
    if (checkFloatFormats)
    {
        requiredFormats.push_back(GL_R32F);
        requiredFormats.push_back(GL_RG32F);
    }

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetDXT1TextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    requiredFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetDXT3TextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetDXT5TextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, false);
}

bool GetDepthTextureSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_DEPTH_COMPONENT16);
    requiredFormats.push_back(GL_DEPTH_COMPONENT32_OES);
    requiredFormats.push_back(GL_DEPTH24_STENCIL8_OES);

    return GetFormatSupport(textureCaps, requiredFormats, true, false, true);
}

bool GetColorBufferFloatSupport(const TextureFormatCapsMap &textureCaps)
{
    std::vector<GLenum> requiredFormats;
    requiredFormats.push_back(GL_R16F);
    requiredFormats.push_back(GL_RG16F);
    requiredFormats.push_back(GL_RGBA16F);
    requiredFormats.push_back(GL_R32F);
    requiredFormats.push_back(GL_RG32F);
    requiredFormats.push_back(GL_RGBA32F);
    requiredFormats.push_back(GL_R11F_G11F_B10F);

    return GetFormatSupport(textureCaps, requiredFormats, false, true, false);
}

}
