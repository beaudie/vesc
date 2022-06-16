//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayoutmetal.h:
//   Methods and classes related to uniform layout and packing in GLSL and HLSL.
//

#ifndef COMMON_BLOCKLAYOUT_METAL__H_
#define COMMON_BLOCKLAYOUT_METAL__H_

#include <cstddef>
#include <map>
#include <vector>

#include <GLSLANG/ShaderLang.h>
#include "angle_gl.h"
#include "blocklayout.h"

namespace sh
{

namespace mtl
{
size_t GetMetalSizeForGLType(GLenum type);
size_t GetMetalAlignmentForGLType(GLenum type);
}  // namespace mtl

}  // namespace sh

#endif  // COMMON_BLOCKLAYOUT_METAL_H_
