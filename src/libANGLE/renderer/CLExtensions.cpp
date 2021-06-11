//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLExtensions.cpp: Implements the struct methods for CLExtension.

#include "libANGLE/renderer/CLExtensions.h"

namespace rx
{

CLExtensions::CLExtensions() = default;

CLExtensions::~CLExtensions() = default;

CLExtensions::CLExtensions(CLExtensions &&) = default;

CLExtensions &CLExtensions::operator=(CLExtensions &&) = default;

void CLExtensions::initializeExtensions(std::string &&extensionStr)
{
    extensions.assign(std::move(extensionStr));
    if (extensions.empty())
    {
        return;
    }

    auto hasExtension = [&](const std::string &extension) {
        // Compare extension with all sub-strings terminated by space or end of string
        std::string::size_type start = 0u;
        do
        {
            std::string::size_type end = extensions.find(' ', start);
            if (end == std::string::npos)
            {
                end = extensions.length();
            }
            const std::string::size_type length = end - start;
            if (length == extension.length() && extensions.compare(start, length, extension) == 0)
            {
                return true;
            }
            start = end + 1u;
        } while (start < extensions.size());
        return false;
    };

    khrICD  = hasExtension("cl_khr_icd");
    khrFP64 = hasExtension("cl_khr_fp64");
}

}  // namespace rx
