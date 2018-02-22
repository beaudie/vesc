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

    int TextureTargetToCubeMapFaceIndex(TextureTarget target)
    {
        switch (target) {
            case TextureTarget::CubeMapNegativeX:
                return 0;
            case TextureTarget::CubeMapPositiveX:
                return 1;
            case TextureTarget::CubeMapNegativeY:
                return 2;
            case TextureTarget::CubeMapPositiveY:
                return 3;
            case TextureTarget::CubeMapNegativeZ:
                return 4;
            case TextureTarget::CubeMapPositiveZ:
                return 5;
            default:
                UNREACHABLE();
                return -1;
        }
    }

    TextureTarget CubeMapFaceIndexToTextureTarget(int face)
    {
        ASSERT(face >= 0 && face <= 5);
        switch (face) {
            case 0:
                return TextureTarget::CubeMapNegativeX;
            case 1:
                return TextureTarget::CubeMapPositiveX;
            case 2:
                return TextureTarget::CubeMapNegativeY;
            case 3:
                return TextureTarget::CubeMapPositiveY;
            case 4:
                return TextureTarget::CubeMapNegativeZ;
            case 5:
                return TextureTarget::CubeMapPositiveZ;
        }
        return TextureTarget::InvalidEnum;
    }
}
