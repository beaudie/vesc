//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_utils.cpp: Helper functions for the CL back end

#include "libANGLE/renderer/cl/cl_util.h"

#include "libANGLE/Debug.h"

#include <cstdlib>

namespace rx
{

cl_version ExtractCLVersion(const std::string &version)
{
    const std::string::size_type spacePos = version.find(' ');
    const std::string::size_type dotPos   = version.find('.');
    if (spacePos == std::string::npos || dotPos == std::string::npos)
    {
        ERR() << "Failed to extract version from OpenCL version string: " << version;
        return 0u;
    }

    const cl_uint major = static_cast<cl_uint>(std::strtol(&version[spacePos + 1u], nullptr, 10));
    const cl_uint minor = static_cast<cl_uint>(std::strtol(&version[dotPos + 1u], nullptr, 10));
    if (major == 0u)
    {
        ERR() << "Failed to extract version from OpenCL version string: " << version;
        return 0u;
    }
    return CL_MAKE_VERSION(major, minor, 0);
}

void RemoveUnsupportedCLExtensions(std::string &extensions)
{
    if (extensions.empty())
    {
        return;
    }
    using SizeT    = std::string::size_type;
    SizeT extStart = 0u;
    do
    {
        const SizeT spacePos  = extensions.find(' ', extStart);
        const bool foundSpace = spacePos != std::string::npos;
        const SizeT length    = (foundSpace ? spacePos : extensions.length()) - extStart;
        if (IsCLExtensionSupported(extensions.substr(extStart, length)))
        {
            extStart = foundSpace && spacePos + 1u < extensions.length() ? spacePos + 1u
                                                                         : std::string::npos;
        }
        else
        {
            extensions.erase(extStart, length + (foundSpace ? 1u : 0u));
            if (extStart >= extensions.length())
            {
                extStart = std::string::npos;
            }
        }
    } while (extStart != std::string::npos);
    while (!extensions.empty() && extensions.back() == ' ')
    {
        extensions.pop_back();
    }
}

void RemoveUnsupportedCLExtensions(NameVersionVector &extensions)
{
    auto extIt = extensions.cbegin();
    while (extIt != extensions.cend())
    {
        if (IsCLExtensionSupported(extIt->name))
        {
            ++extIt;
        }
        else
        {
            extIt = extensions.erase(extIt);
        }
    }
}

}  // namespace rx
