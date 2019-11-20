//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangWrapperVk: Wrapper for Vulkan's glslang compiler.
//

#include "libANGLE/renderer/vulkan/GlslangWrapperVk.h"

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
}  // namespace

// static
void GlslangWrapperVk::GetShaderSource(
    const gl::ProgramState &programState,
    const gl::ProgramLinkedResources &resources,
    GlslangSourceOptions &glslangSourceOptions,
    gl::ShaderMap<std::string> *shaderSourcesOut,
    gl::ShaderMap<ShaderInterfaceVariableInfoMap> *variableInfoMapOut)
{
    GlslangGetShaderSource(glslangSourceOptions, programState, resources, shaderSourcesOut,
                           variableInfoMapOut);
}

// static
angle::Result GlslangWrapperVk::GetShaderCode(
    vk::Context *context,
    const gl::Caps &glCaps,
    const gl::ShaderMap<std::string> &shaderSources,
    const gl::ShaderMap<ShaderInterfaceVariableInfoMap> &variableInfoMap,
    gl::ShaderMap<std::vector<uint32_t>> *shaderCodeOut)
{
    return GlslangGetShaderSpirvCode(
        [context](GlslangError error) { return ErrorHandler(context, error); }, glCaps,
        shaderSources, variableInfoMap, shaderCodeOut);
}

// static
angle::Result GlslangWrapperVk::TransformSpirV(
    vk::Context *context,
    const gl::ShaderType shaderType,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    std::vector<uint32_t> &initialSpirvBlob,
    std::vector<uint32_t> *shaderCodeOut)
{
    return TransformSpirvCode(
        [context](GlslangError error) { return ErrorHandler(context, error); }, shaderType,
        variableInfoMap, initialSpirvBlob, shaderCodeOut);
}
}  // namespace rx
