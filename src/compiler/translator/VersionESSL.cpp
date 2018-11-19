//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/VersionESSL.h"

#include "angle_gl.h"

namespace sh
{

int ShaderOutputTypeToESSLVersion(ShShaderOutput output)
{
    switch (output)
    {
        case SH_ESSL_100_CORE_OUTPUT:
            return ESSL_VERSION_100;
        case SH_ESSL_300_CORE_OUTPUT:
            return ESSL_VERSION_300;
        case SH_ESSL_310_CORE_OUTPUT:
            return ESSL_VERSION_310;
        case SH_ESSL_OUTPUT:
            return ESSL_VERSION_100;
        default:
            UNREACHABLE();
            return 0;
    }
}

TVersionESSL::TVersionESSL(sh::GLenum type, const TPragma &pragma, ShShaderOutput output)
    : TIntermTraverser(true, false, false)
{
    mVersion = ShaderOutputTypeToESSLVersion(output);
    if (pragma.stdgl.invariantAll)
    {
        ensureVersionIsAtLeast(ESSL_VERSION_100);
    }
    if (type == GL_COMPUTE_SHADER)
    {
        ensureVersionIsAtLeast(ESSL_VERSION_310);
    }
}

void TVersionESSL::ensureVersionIsAtLeast(int version)
{
    mVersion = std::max(version, mVersion);
}

}  // namespace sh
