//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorVulkan:
//   A GLSL-based translator that outputs shaders that fit GL_KHR_vulkan_glsl.
//   The shaders are then fed into glslang to spit out SPIR-V (libANGLE-side).
//   See: https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
//

#include "compiler/translator/TranslatorVulkan.h"

namespace sh
{

TranslatorVulkan::TranslatorVulkan(sh::GLenum type, ShShaderSpec spec) : TranslatorESSL(type, spec)
{
}

void TranslatorVulkan::translate(TIntermNode *root, ShCompileOptions compileOptions)
{
    // TODO(jmadill): Vulkan-specific translation.
    TranslatorESSL::translate(root, compileOptions);
}

}  // namespace sh
