//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerVk.cpp:
//    Implements the class methods for CompilerVk.
//

#include "libANGLE/renderer/vulkan/CompilerVk.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{

CompilerVk::CompilerVk(const ContextVk *context) : CompilerImpl()
{
    if (context->getExtensions().shaderPixelLocalStorageCoherentANGLE)
    {
        ASSERT(context->getFeatures().supportsFragmentShaderPixelInterlock.enabled);
        // Set our fragment synchronization type to GL_ARB_fragment_shader_interlock.
        //
        // This is what we need for Vulkan GLSL, and VK_EXT_fragment_shader_interlock /
        // SPV_EXT_fragment_shader_interlock are both spec'd as providing support for
        // GL_ARB_fragment_shader_interlock.
        mBackendFeatures.fragmentSynchronizationType =
            ShFragmentSynchronizationType::FragmentShaderInterlock_ARB_GLSL;
    }
}

CompilerVk::~CompilerVk() {}

ShShaderOutput CompilerVk::getTranslatorOutputType() const
{
    return SH_SPIRV_VULKAN_OUTPUT;
}

CompilerBackendFeatures CompilerVk::getBackendFeatures() const
{
    return mBackendFeatures;
}

}  // namespace rx
