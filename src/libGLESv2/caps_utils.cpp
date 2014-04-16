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

static bool GetFormatSupport(const TextureFormatCapsMap &textureCaps, const std::vector<GLenum> &requiredFormats,
                             bool requiresFiltering, bool requiresColorBuffer, bool requiresDepthStencil)
{
    for (size_t i = 0; i < requiredFormats.size(); i++)
    {
        TextureFormatCapsMap::const_iterator iter = textureCaps.find(requiredFormats[i]);
        if (iter == textureCaps.end())
        {
            return false;
        }

        const TextureCaps &cap = iter->second;
        if (requiresFiltering && !cap.getTextureFilteringSupport())
        {
            return false;
        }

        if (requiresColorBuffer && !cap.getColorRenderingSupport())
        {
            return false;
        }

        if (requiresDepthStencil && !cap.getDepthRenderingSupport())
        {
            return false;
        }
    }

    return true;
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

GLuint GetMaximumSupportedSamples(const TextureFormatCapsMap &textureCaps)
{
    GLuint maxSamples = 0;
    for (TextureFormatCapsMap::const_iterator i = textureCaps.begin(); i != textureCaps.end(); i++)
    {
        maxSamples = std::max(maxSamples, i->second.getMaxSupportedSamples());
    }
    return maxSamples;
}

}
