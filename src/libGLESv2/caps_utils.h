#ifndef LIBGLESV2_CAPS_UTILS_H
#define LIBGLESV2_CAPS_UTILS_H

//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libGLESv2/Caps.h"

namespace gl
{

// Checks for GL_OES_rgb8_rgba8 support
bool GetRGB8AndRGBA8TextureSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_EXT_texture_format_BGRA8888 support
bool GetBGRA8TextureSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_OES_texture_half_float support
bool GetHalfFloatTextureSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_OES_texture_half_float_linear support
bool GetHalfFloatTextureFilteringSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_OES_texture_float support
bool GetFloatTextureSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_OES_texture_float_linear support
bool GetFloatTextureFilteringSupport(const TextureFormatCapsMap &textureCaps);

// Checks for GL_EXT_texture_rg support
bool GetRGTextureSupport(const TextureFormatCapsMap &textureCaps, bool checkHalfFloatFormats, bool checkFloatFormats);

// Check for GL_EXT_texture_compression_dxt1
bool GetDXT1TextureSupport(const TextureFormatCapsMap &textureCaps);

// Check for GL_ANGLE_texture_compression_dxt3
bool GetDXT3TextureSupport(const TextureFormatCapsMap &textureCaps);

// Check for GL_ANGLE_texture_compression_dxt5
bool GetDXT5TextureSupport(const TextureFormatCapsMap &textureCaps);

// Check for GL_ANGLE_depth_texture
bool GetDepthTextureSupport(const TextureFormatCapsMap &textureCaps);

// Check for GL_EXT_color_buffer_float
bool GetColorBufferFloatSupport(const TextureFormatCapsMap &textureCaps);

// Determine the maximum number of samples supported, across all texture formats supported
GLuint GetMaximumSupportedSamples(const TextureFormatCapsMap &textureCaps);

}

#endif // LIBGLESV2_CAPS_UTILS_H
