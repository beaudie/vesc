//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangWrapperVk: Wrapper for Vulkan's glslang compiler.
//

#include "libANGLE/renderer/vulkan/GlslangWrapperVk.h"

#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"

namespace rx
{
namespace
{
angle::Result ErrorHandler(vk::Context *context, GlslangError)
{
    ANGLE_VK_CHECK(context, false, VK_ERROR_INVALID_SHADER_NV);
    return angle::Result::Stop;
}

GlslangSourceOptions CreateSourceOptions(const angle::FeaturesVk &features)
{
    GlslangSourceOptions options;
    options.uniformsAndXfbDescriptorSetIndex = kUniformsAndXfbDescriptorSetIndex;
    options.textureDescriptorSetIndex        = kTextureDescriptorSetIndex;
    options.shaderResourceDescriptorSetIndex = kShaderResourceDescriptorSetIndex;
    options.driverUniformsDescriptorSetIndex = kDriverUniformsDescriptorSetIndex;
    options.xfbBindingIndexStart             = kXfbBindingIndexStart;
    options.useOldRewriteStructSamplers      = features.forceOldRewriteStructSamplers.enabled;
    options.supportsTransformFeedbackExtension =
        features.supportsTransformFeedbackExtension.enabled;
    options.emulateTransformFeedback = features.emulateTransformFeedback.enabled;
    return options;
}
}  // namespace

// static
angle::Result GlslangWrapperVk::GetShaderSpirvCode(vk::Context *context,
                                                   const angle::FeaturesVk &features,
                                                   const gl::Caps &glCaps,
                                                   const gl::ProgramState &programState,
                                                   const gl::ProgramLinkedResources &resources,
                                                   gl::ShaderMap<SpirvShader> *spirvShadersOut)
{
    auto errorHandler = [context](GlslangError error) { return ErrorHandler(context, error); };
    return GlslangGetShaderSpirvCode(CreateSourceOptions(features), errorHandler, glCaps,
                                     programState, resources, spirvShadersOut);
}

// static
void GlslangWrapperVk::GetSpecializedShaderSpirvCode(
    bool enableLineRasterEmulation,
    const gl::ShaderMap<SpirvShader> &spirvShaders,
    gl::ShaderMap<std::vector<uint32_t>> *specializedSpirvShadersOut)
{
    GlslangGetSpecializedShaderSpirvCode(enableLineRasterEmulation, spirvShaders,
                                         specializedSpirvShadersOut);
}
}  // namespace rx
