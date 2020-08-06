//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_utils.cpp : Utility functions for working with angle::Feature

#include "libANGLE/feature_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Surface.h"
#include "libANGLE/angletypes.h"
#include "platform/FrontendFeatures.h"

namespace angle
{
namespace
{

bool IsDownscaleBackbufferTextureEnabled(const FrontendFeatures &features, float *outScale)
{
    size_t highestDownscale = 0;
    for (size_t downscaleIdx = 0; downscaleIdx < features.downscaleBackbufferTextures.size();
         downscaleIdx++)
    {
        if (features.downscaleBackbufferTextures[downscaleIdx].enabled)
        {
            highestDownscale = downscaleIdx + 1;
        }
    }

    if (outScale)
    {
        // Each feature is 12.5% (1/8) more downscaling
        *outScale = 1.0f - (highestDownscale * 0.125f);
    }
    return highestDownscale > 0;
}
}  // namespace

float GetDownscaledBackbufferScale(const FrontendFeatures &features)
{
    float scale = 1.0f;
    if (IsDownscaleBackbufferTextureEnabled(features, &scale))
    {
        return scale;
    }
    else
    {
        return 1.0f;
    }
}

bool IsDownscaleBackbufferTextureEnabledForTexture(const gl::Context *context,
                                                   gl::TextureType textureType,
                                                   const gl::Format &format,
                                                   GLuint level,
                                                   gl::Extents *size)
{
    float scale = 1.0f;
    if (!context || !IsDownscaleBackbufferTextureEnabled(context->getFrontendFeatures(), &scale))
    {
        return false;
    }

    // Allow downscaling if:

    // - 2D texture
    if (textureType != gl::TextureType::_2D)
    {
        return false;
    }

    // - Level is 0 (glTexStorage passes level 0 always)
    if (level != 0)
    {
        return false;
    }

    // - Same size as the backbuffer of this context
    const egl::Surface *backbuffer = context->getCurrentDrawSurface();
    if (!backbuffer || backbuffer->getWidth() != size->width ||
        backbuffer->getHeight() != size->height)
    {
        return false;
    }

    // - Renderable texture format
    if (!format.info->textureAttachmentSupport(context->getClientVersion(),
                                               context->getExtensions()))
    {
        return false;
    }

    // Skipped options:
    // - No pixel data (trace replay uploads data during replay even if the user did not originally)
    // - Only 1 level (some applications render to non-zero mips of backbuffer-sized textures)

    ASSERT(size);
    size->scale(scale);
    return true;
}

bool IsDownscaleBackbufferTextureEnabledForRenderbuffer(const gl::Context *context,
                                                        const gl::Format &format,
                                                        gl::Extents *size)
{
    float scale = 1.0f;
    if (!context || !IsDownscaleBackbufferTextureEnabled(context->getFrontendFeatures(), &scale))
    {
        return false;
    }

    // Allow downscaling if:

    // - Same size as the backbuffer of this context
    const egl::Surface *backbuffer = context->getCurrentDrawSurface();
    if (!backbuffer || backbuffer->getWidth() != size->width ||
        backbuffer->getHeight() != size->height)
    {
        return false;
    }

    ASSERT(size);
    size->scale(scale);
    return true;
}
}  // namespace angle
