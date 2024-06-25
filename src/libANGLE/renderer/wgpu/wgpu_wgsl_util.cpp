//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_wgsl_util.h"

#include <sstream>

#include "common/PackedEnums.h"
#include "common/PackedGLEnums_autogen.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"

namespace rx
{
namespace webgpu
{

namespace
{
const bool kOutputReplacements = false;

// Replaces location markers in the WGSL source with actual locations.
void ReplaceLocationMarker(std::string &shaderSource, int location, const std::string &name)
{
    ASSERT(location != -1);
    const char *marker = "@location(@@@@@@) ";
    // TODO(anglebug.com/42267100): if the GLSL input is a matrix there should be multiple WGSL
    // input variables (multiple vectors representing the columns of the matrix).
    std::ostringstream locationMarkerStream;
    locationMarkerStream << marker << name;
    std::size_t varFoundInSource = shaderSource.find(locationMarkerStream.str());
    if (varFoundInSource == std::string::npos)
    {
        // This substring should always be found, but fail gracefully in production.
        ASSERT(false);
        return;
    }

    std::ostringstream locationReplacementStream;
    locationReplacementStream << "@location(" << location << ") ";

    // We can avoid string copies by replacing with a string of the exact same size, so pad
    // location marker with spaces.
    for (size_t newSize = locationReplacementStream.str().size(); newSize < strlen(marker);
         newSize++)
    {
        locationReplacementStream << ' ';
    }
    locationReplacementStream << name;
    if (kOutputReplacements)
    {
        std::cout << "Replace \"" << locationMarkerStream.str() << "\" with \""
                  << locationReplacementStream.str() << "\"" << std::endl;
    }
    ASSERT(locationReplacementStream.str().size() == locationMarkerStream.str().size());

    shaderSource.replace(varFoundInSource, locationReplacementStream.str().size(),
                         locationReplacementStream.str());
}

}  // namespace

void WgslAssignLocations(std::string &shaderSource, const std::vector<gl::ProgramInput> &shaderVars)
{
    for (const gl::ProgramInput &shaderVar : shaderVars)
    {
        if (shaderVar.isBuiltIn())
        {
            continue;
        }
        ReplaceLocationMarker(shaderSource, shaderVar.getLocation(), shaderVar.name);
    }
}
void WgslAssignLocations(std::string &shaderSource,
                         const std::vector<gl::ProgramOutput> &shaderVars)
{
    for (const gl::ProgramOutput &shaderVar : shaderVars)
    {
        if (shaderVar.isBuiltIn())
        {
            continue;
        }
        ReplaceLocationMarker(shaderSource, shaderVar.getLocation(), shaderVar.name);
    }
}

void WgslAssignLocations(std::string &shaderSource, const gl::ProgramMergedVaryings &mergedVaryings)
{
    int currLocMarker = 0;
    for (const gl::ProgramVaryingRef &linkedVarying : mergedVaryings)
    {
        gl::ShaderBitSet supportedShaderStages =
            gl::ShaderBitSet({gl::ShaderType::Vertex, gl::ShaderType::Fragment});
        ASSERT(linkedVarying.frontShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.frontShaderStage));
        ASSERT(linkedVarying.backShaderStage == gl::ShaderType::InvalidEnum ||
               supportedShaderStages.test(linkedVarying.backShaderStage));
        if (!linkedVarying.frontShader || !linkedVarying.backShader)
        {
            continue;
        }
        if (linkedVarying.backShader->isBuiltIn())
        {
            continue;
        }
        ReplaceLocationMarker(shaderSource, currLocMarker++, linkedVarying.backShader->name);
    }
}

}  // namespace webgpu
}  // namespace rx
