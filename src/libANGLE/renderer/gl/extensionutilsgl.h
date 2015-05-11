//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// extensionutils.h: Queries for native GL extensions and their version availability

#include "angle_gl.h"
#include <string>

namespace rx
{

namespace nativegl
{

struct Extension
{
    Extension();

    // Version that the extension was written against
    GLuint minVersionMajor;
    GLuint minVersionMinor;

    // Version that the extension became core
    GLuint coreVersionMajor;
    GLuint coreVersionMinor;
};

const Extension &GetExtensionInfo(const std::string &name);

}

}
