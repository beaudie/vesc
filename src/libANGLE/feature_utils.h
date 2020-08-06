//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_utils.h : Utility functions for working with angle::Feature

#include <common/PackedEnums.h>

namespace gl
{
class Context;
struct Extents;
struct Format;
}  // namespace gl

namespace angle
{

struct FrontendFeatures;

float GetDownscaledBackbufferScale(const FrontendFeatures &features);
bool IsDownscaleBackbufferTextureEnabledForTexture(const gl::Context *context,
                                                   gl::TextureType textureType,
                                                   const gl::Format &format,
                                                   GLuint level,
                                                   gl::Extents *size);
bool IsDownscaleBackbufferTextureEnabledForRenderbuffer(const gl::Context *context,
                                                        const gl::Format &format,
                                                        gl::Extents *size);

}  // namespace angle
