//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicImage2DHLSL.h: Interface for link and run-time HLSL generation
//

#ifndef LIBANGLE_RENDERER_D3D_DYNAMICIMAGE2DHLSL_H_
#define LIBANGLE_RENDERER_D3D_DYNAMICIMAGE2DHLSL_H_

#include "common/angleutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{
std::string generateShaderForImage2DBindSignature(
    const d3d::Context *context,
    ProgramD3D &programD3D,
    const gl::ProgramState &programData,
    gl::ShaderType shaderType,
    const std::string &shaderHLSL,
    std::vector<sh::ShaderVariable> &image2DUniforms,
    const gl::ImageUnitTextureTypeMap &image2DBindLayout,
    unsigned int baseUAVRegister);

inline std::string GenerateShaderForImage2DBindSignature(
    const d3d::Context *context,
    ProgramD3D &programD3D,
    const gl::ProgramState &programData,
    gl::ShaderType shaderType,
    const std::string shaderHLSL,
    std::vector<sh::ShaderVariable> &image2DUniforms,
    const gl::ImageUnitTextureTypeMap &image2DBindLayout,
    unsigned int baseUAVRegister)
{
    return generateShaderForImage2DBindSignature(context, programD3D, programData, shaderType,
                                                 shaderHLSL, image2DUniforms, image2DBindLayout,
                                                 baseUAVRegister);
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_DYNAMICHLSL_H_
