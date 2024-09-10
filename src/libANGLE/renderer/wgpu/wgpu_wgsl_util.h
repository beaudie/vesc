//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// wgpu_wgsl_util.h: Utilities to manipulate previously translated WGSL.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_

#include "common/PackedGLEnums_autogen.h"
#include "libANGLE/Program.h"

namespace rx
{
namespace webgpu
{

namespace impl
{
std::string WgslReplaceLocationMarkers(const std::string &shaderSource,
                                       std::map<std::string, int> varNameToLocation);
}

// Replaces location markers in the WGSL source with actual locations, for
// `shaderVars` which is a vector of either gl::ProgramInputs or gl::ProgramOutputs, and for
// `mergedVaryings` which get assigned sequentially increasing locations. There should be at most
// vertex and fragment shader stages or this function will not assign locations correctly.
template <typename T>
std::string WgslAssignLocations(const std::string &shaderSource,
                                const std::vector<T> shaderVars,
                                const gl::ProgramMergedVaryings &mergedVaryings,
                                gl::ShaderType shaderType)
{
    std::map<std::string, int> varNameToLocation;
    for (const T &shaderVar : shaderVars)
    {
        if (shaderVar.isBuiltIn())
        {
            continue;
        }
        varNameToLocation[shaderVar.name] = shaderVar.getLocation();
    }

    int currLocMarker = 0;
    for (const gl::ProgramVaryingRef &linkedVarying : mergedVaryings)
    {
        gl::ShaderBitSet supportedShaderStages =
            gl::ShaderBitSet({gl::ShaderType::Vertex, gl::ShaderType::Fragment});
        ASSERT(linkedVarying.frontShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.frontShaderStage));
        ASSERT(linkedVarying.backShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.backShaderStage));
        if (!linkedVarying.frontShader && !linkedVarying.backShader)
        {
            continue;
        }
        const sh::ShaderVariable *shaderVar = shaderType == gl::ShaderType::Vertex
                                                  ? linkedVarying.frontShader
                                                  : linkedVarying.backShader;
        if (shaderVar)
        {
            if (shaderVar->isBuiltIn())
            {
                continue;
            }
            ASSERT(varNameToLocation.find(shaderVar->name) == varNameToLocation.end());
            varNameToLocation[shaderVar->name] = currLocMarker++;
        }
        else
        {
            const sh::ShaderVariable *otherShaderVar = shaderType == gl::ShaderType::Vertex
                                                           ? linkedVarying.backShader
                                                           : linkedVarying.frontShader;
            if (!otherShaderVar->isBuiltIn())
            {
                // Increment `currLockMarker` to keep locations in sync with the WGSL source
                // generated for the other shader stage, which will also have incremented
                // `currLocMarker` when seeing this variable.
                currLocMarker++;
            }
        }
    }

    return impl::WgslReplaceLocationMarkers(shaderSource, varNameToLocation);
}

}  // namespace webgpu
}  // namespace rx

#endif /* LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_ */
