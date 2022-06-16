//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayoutMetal.cpp:
//   Implementation for block layout classes and methods.
//

#include "compiler/translator/blocklayoutMetal.h"
#include "common/mathutil.h"
#include "compiler/translator/blocklayout.h"

namespace sh
{
namespace mtl
{
size_t GetMetalSizeForGLType(GLenum type)
{
    switch (type)
    {
        case GL_BOOL:
            return 1;
        case GL_BOOL_VEC2:
            return 2;
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_FLOAT_VEC2:
            return 8;
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            return 16;
        case GL_FLOAT_MAT2:  // 2x2
            return 16;
        case GL_FLOAT_MAT3:  // 3x4
            return 48;
        case GL_FLOAT_MAT4:  // 4x4
            return 64;
        case GL_FLOAT_MAT2x3:  // 2x4
            return 32;
        case GL_FLOAT_MAT3x2:  // 3x2
            return 24;
        case GL_FLOAT_MAT2x4:  // 2x4
            return 32;
        case GL_FLOAT_MAT4x2:  // 4x2
            return 32;
        case GL_FLOAT_MAT3x4:  // 3x4
            return 48;
        case GL_FLOAT_MAT4x3:  // 4x4
            return 64;
        case GL_INT:
            return 4;
        case GL_INT_VEC2:
            return 8;
        case GL_INT_VEC3:
            return 16;
        case GL_INT_VEC4:
            return 16;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_INT_VEC2:
            return 8;
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return 16;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT_ANGLE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_EXTERNAL_OES:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_IMAGE_2D:
        case GL_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_IMAGE_CUBE:
        case GL_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_IMAGE_BUFFER:
        case GL_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}

size_t GetMetalAlignmentForGLType(GLenum type)
{
    switch (type)
    {
        case GL_BOOL:
            return 1;
        case GL_BOOL_VEC2:
            return 2;
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_FLOAT_VEC2:
            return 8;
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            return 16;
        case GL_FLOAT_MAT2:
            return 8;
        case GL_FLOAT_MAT3:
            return 16;
        case GL_FLOAT_MAT4:
            return 16;
        case GL_FLOAT_MAT2x3:
            return 16;
        case GL_FLOAT_MAT3x2:
            return 8;
        case GL_FLOAT_MAT2x4:
            return 16;
        case GL_FLOAT_MAT4x2:
            return 8;
        case GL_FLOAT_MAT3x4:
            return 16;
        case GL_FLOAT_MAT4x3:
            return 16;
        case GL_INT:
            return 4;
        case GL_INT_VEC2:
            return 8;
        case GL_INT_VEC3:
            return 16;
        case GL_INT_VEC4:
            return 16;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_INT_VEC2:
            return 8;
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return 16;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT_ANGLE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_EXTERNAL_OES:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_IMAGE_2D:
        case GL_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_IMAGE_CUBE:
        case GL_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_IMAGE_BUFFER:
        case GL_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}
}  // namespace mtl
}  // namespace sh
