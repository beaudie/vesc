// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackedGLEnums_autogen.cpp:
//   Declares ANGLE-specific enums classes for GLEnum and functions operating
//   on them.

#include "libANGLE/PackedGLEnums.h"

namespace gl
{

    TextureType TextureTargetToType(TextureTarget target)
    {
        switch (target) {
            case TextureTarget::CubeMapNegativeX:
            case TextureTarget::CubeMapNegativeY:
            case TextureTarget::CubeMapNegativeZ:
            case TextureTarget::CubeMapPositiveX:
            case TextureTarget::CubeMapPositiveY:
            case TextureTarget::CubeMapPositiveZ:
                return TextureType::CubeMap;
            case TextureTarget::External:
                return TextureType::External;
            case TextureTarget::Rectangle:
                return TextureType::Rectangle;
            case TextureTarget::_2D:
                return TextureType::_2D;
            case TextureTarget::_2DArray:
                return TextureType::_2DArray;
            case TextureTarget::_2DMultisample:
                return TextureType::_2DMultisample;
            case TextureTarget::_3D:
                return TextureType::_3D;
            default:
                UNREACHABLE();
                return TextureType::InvalidEnum;
        }
    }

    TextureTarget NonCubeTextureTypeToTarget(TextureType type) {
        switch (type) {
            case TextureType::External:
                return TextureTarget::External;
            case TextureType::Rectangle:
                return TextureTarget::Rectangle;
            case TextureType::_2D:
                return TextureTarget::_2D;
            case TextureType::_2DArray:
                return TextureTarget::_2DArray;
            case TextureType::_2DMultisample:
                return TextureTarget::_2DMultisample;
            case TextureType::_3D:
                return TextureTarget::_3D;
            default:
                UNREACHABLE();
                return TextureTarget::InvalidEnum;
        }
    }

}
