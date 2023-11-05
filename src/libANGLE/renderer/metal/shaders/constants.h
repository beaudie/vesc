//
// Copyright 2020 The ANGLE Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// constants.h: Declare some constant values to be used by metal defaultshaders.

#ifndef LIBANGLE_RENDERER_METAL_SHADERS_ENUM_H_
#define LIBANGLE_RENDERER_METAL_SHADERS_ENUM_H_

namespace rx
{
namespace mtl_shader
{

enum
{
    kTextureType2D            = 0,
    kTextureType2DMultisample = 1,
    kTextureType2DArray       = 2,
    kTextureTypeCube          = 3,
    kTextureType3D            = 4,
    kTextureTypeCount         = 5,
};

enum
{
    kVertexTypeFloat           = 0,
    kVertexTypeFixed           = 1,
    kVertexTypeHalf            = 2,
    kVertexTypeXYZW1010102Int  = 3,
    kVertexTypeXYZW1010102UInt = 4,
    kVertexTypeByte            = 5,
    kVertexTypeUByte           = 6,
    kVertexTypeShort           = 7,
    kVertexTypeUShort          = 8,
    kVertexTypeInt             = 9,
    kVertexTypeUInt            = 10,
    kVertexTypeInvalid         = 11,
};

enum
{
    kVertexConvertModeNone      = 0,
    kVertexConvertModeNormalize = 1,
    kVertexConvertModeToFloat   = 2,
};

// Metal doesn't support constexpr to be used as array size, so we need to use macro here
#define kGenerateMipThreadGroupSizePerDim 8

}  // namespace mtl_shader
}  // namespace rx

#endif
