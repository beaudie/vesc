//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// extensionutils.cpp: Queries for native GL extensions and their version availability

#include "libANGLE/renderer/gl/extensionutilsgl.h"

#include <map>

#include "common/debug.h"

namespace rx
{

namespace nativegl
{

const GLuint UnavailableVersion = std::numeric_limits<GLuint>::max();

Extension::Extension()
    : minVersionMajor(UnavailableVersion),
      minVersionMinor(UnavailableVersion),
      coreVersionMajor(UnavailableVersion),
      coreVersionMinor(UnavailableVersion)
{
}

typedef std::map<std::string, Extension> ExtensionMap;
static ExtensionMap BuildExtensionMap()
{
    ExtensionMap map;

    return map;
}

const Extension &GetExtensionInfo(const std::string &name)
{
    static const ExtensionMap map = BuildExtensionMap();
    auto iter = map.find(name);
    ASSERT(iter != map.end());
    if (iter != map.end())
    {
        return iter->second;
    }
    else
    {
        static const Extension defaultExtension;
        return defaultExtension;
    }
}


}

}
