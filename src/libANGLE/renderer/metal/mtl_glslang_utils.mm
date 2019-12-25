//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangUtils: Wrapper for Khronos's glslang compiler.
//

#include "libANGLE/renderer/metal/mtl_glslang_utils.h"

#include "libANGLE/renderer/glslang_wrapper_utils.h"

namespace rx
{
namespace mtl
{
namespace
{
angle::Result HandleError(ErrorHandler *context, GlslangError)
{
    ANGLE_MTL_TRY(context, false);
    return angle::Result::Stop;
}

GlslangSourceOptions CreateSourceOptions()
{
    GlslangSourceOptions options;
    // We don't actually use descriptor set for now, the actual binding will be done inside
    // ProgramMtl using spirv-cross.
    options.uniformsAndXfbDescriptorSetIndex = kDefaultUniformsBindingIndex;
    options.textureDescriptorSetIndex        = 0;
    options.driverUniformsDescriptorSetIndex = kDriverUniformsBindingIndex;
    // NOTE(hqle): Unused for now, until we support ES 3.0
    options.shaderResourceDescriptorSetIndex = -1;
    options.xfbBindingIndexStart             = -1;

    static_assert(kDefaultUniformsBindingIndex != 0, "kDefaultUniformsBindingIndex must not be 0");
    static_assert(kDriverUniformsBindingIndex != 0, "kDriverUniformsBindingIndex must not be 0");

    return options;
}
}  // namespace

angle::Result GlslangGetShaderSpirvCode(ErrorHandler *context,
                                        const gl::Caps &glCaps,
                                        const gl::ProgramState &programState,
                                        const gl::ProgramLinkedResources &resources,
                                        gl::ShaderMap<SpirvShader> *spirvShadersOut)
{
    auto errorHandler = [context](GlslangError error) { return HandleError(context, error); };
    return rx::GlslangGetShaderSpirvCode(CreateSourceOptions(), errorHandler, glCaps, programState,
                                         resources, spirvShadersOut);
}

void GlslangGetSpecializedShaderSpirvCode(
    bool enableLineRasterEmulation,
    const gl::ShaderMap<SpirvShader> &spirvShaders,
    gl::ShaderMap<std::vector<uint32_t>> *specializedSpirvShadersOut)
{
    rx::GlslangGetSpecializedShaderSpirvCode(enableLineRasterEmulation, spirvShaders,
                                             specializedSpirvShadersOut);
}
}  // namespace mtl
}  // namespace rx
