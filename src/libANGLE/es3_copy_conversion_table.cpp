//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// format_map:
//   Determining the sized internal format from a (format,type) pair.
//   Also check es3 format combinations for validity.

#include "angle_gl.h"
#include "common/constexpr_array.h"
#include "common/debug.h"

namespace gl
{

namespace
{

// TODO(jmadill): Replace this with a constexpr std::pair when supported in Chromium.
struct CopyDesc final
{
    constexpr CopyDesc(GLenum textureFormat, GLenum framebufferFormat)
        : texFormat(textureFormat), fboFormat(framebufferFormat)
    {
    }

    constexpr bool operator<(const CopyDesc &other) const
    {
        if (texFormat < other.texFormat)
            return true;
        return fboFormat < other.fboFormat;
    }

    constexpr bool operator==(const CopyDesc &other) const
    {
        return ((texFormat == other.texFormat) && (fboFormat == other.fboFormat));
    }

    GLenum texFormat;
    GLenum fboFormat;
};

// Since the unsorted array is only referenced in construction, it should be optimized out.

constexpr angle::constexpr_array<CopyDesc, 34> kUnsorted{{// From ES 3.0.1 spec, table 3.15
                                                          {GL_ALPHA, GL_RGBA},
                                                          {GL_LUMINANCE, GL_RED},
                                                          {GL_LUMINANCE, GL_RG},
                                                          {GL_LUMINANCE, GL_RGB},
                                                          {GL_LUMINANCE, GL_RGBA},
                                                          {GL_LUMINANCE_ALPHA, GL_RGBA},
                                                          {GL_RED, GL_RED},
                                                          {GL_RED, GL_RG},
                                                          {GL_RED, GL_RGB},
                                                          {GL_RED, GL_RGBA},
                                                          {GL_RG, GL_RG},
                                                          {GL_RG, GL_RGB},
                                                          {GL_RG, GL_RGBA},
                                                          {GL_RGB, GL_RGB},
                                                          {GL_RGB, GL_RGBA},
                                                          {GL_RGBA, GL_RGBA},

                                                          // Necessary for ANGLE back-buffers
                                                          {GL_ALPHA, GL_BGRA_EXT},
                                                          {GL_LUMINANCE, GL_BGRA_EXT},
                                                          {GL_LUMINANCE_ALPHA, GL_BGRA_EXT},
                                                          {GL_RED, GL_BGRA_EXT},
                                                          {GL_RG, GL_BGRA_EXT},
                                                          {GL_RGB, GL_BGRA_EXT},
                                                          {GL_RGBA, GL_BGRA_EXT},
                                                          {GL_BGRA_EXT, GL_BGRA_EXT},

                                                          {GL_RED_INTEGER, GL_RED_INTEGER},
                                                          {GL_RED_INTEGER, GL_RG_INTEGER},
                                                          {GL_RED_INTEGER, GL_RGB_INTEGER},
                                                          {GL_RED_INTEGER, GL_RGBA_INTEGER},
                                                          {GL_RG_INTEGER, GL_RG_INTEGER},
                                                          {GL_RG_INTEGER, GL_RGB_INTEGER},
                                                          {GL_RG_INTEGER, GL_RGBA_INTEGER},
                                                          {GL_RGB_INTEGER, GL_RGB_INTEGER},
                                                          {GL_RGB_INTEGER, GL_RGBA_INTEGER},
                                                          {GL_RGBA_INTEGER, GL_RGBA_INTEGER}}};

constexpr auto kSorted = angle::constexpr_sort(kUnsorted);

}  // anonymous namespace

bool ValidES3CopyConversion(GLenum textureFormat, GLenum framebufferFormat)
{
    const CopyDesc searchDesc(textureFormat, framebufferFormat);
    const CopyDesc *foundDesc = std::lower_bound(kSorted.begin(), kSorted.end(), searchDesc);
    return (foundDesc && *foundDesc == searchDesc);
}

}  // namespace gl
