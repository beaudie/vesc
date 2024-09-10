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
}

namespace impl
{
std::string WgslReplaceLocationMarkers(const std::string &shaderSource,
                                       std::map<std::string, int> varNameToLocation)
{
    const char *marker    = "@location(@@@@@@) ";
    const char *endOfName = " : ";

    std::string newSource;
    newSource.reserve(shaderSource.size());

    size_t currPos = 0;
    while (true)
    {
        size_t nextMarker = shaderSource.find(marker, currPos, strlen(marker));
        if (nextMarker == std::string::npos)
        {
            // Copy the rest of the shader and end the loop.
            newSource.append(shaderSource, currPos);
            break;
        }
        else
        {
            // Copy up to the next marker
            newSource.append(shaderSource, currPos, nextMarker - currPos);

            // Extract name from something like `@location(@@@@@@) NAME : TYPE`.
            size_t startOfNamePos = nextMarker + strlen(marker);
            size_t endOfNamePos   = shaderSource.find(endOfName, startOfNamePos, strlen(endOfName));
            std::string name(shaderSource.c_str() + startOfNamePos, endOfNamePos - startOfNamePos);

            // Use the shader variable's name to get the assigned location
            auto locationIter = varNameToLocation.find(name);
            if (locationIter == varNameToLocation.end())
            {
                ASSERT(false);
                return "";
            }

            // TODO(anglebug.com/42267100): if the GLSL input is a matrix there should be multiple
            // WGSL input variables (multiple vectors representing the columns of the matrix).
            int location = locationIter->second;
            std::ostringstream locationReplacementStream;
            locationReplacementStream << "@location(" << location << ") " << name;

            if (kOutputReplacements)
            {
                std::cout << "Replace \"" << marker << name << "\" with \""
                          << locationReplacementStream.str() << "\"" << std::endl;
            }

            // Append the new `@location(N) name` and then continue from the ` : type`.
            newSource.append(locationReplacementStream.str());
            currPos = endOfNamePos;
        }
    }
    return newSource;
}

}  // namespace impl

}  // namespace webgpu
}  // namespace rx
