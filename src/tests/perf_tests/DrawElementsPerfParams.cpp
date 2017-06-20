//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawElementsPerfParams.cpp:
//   Parametrization for performance tests for ANGLE DrawElements call overhead.
//

#include "DrawElementsPerfParams.h"

#include <sstream>

std::ostream &operator<<(std::ostream &os, const DrawElementsPerfParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

std::string DrawElementsPerfParams::suffix() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::suffix();

    if (indexBufferChanged)
    {
        strstr << "_index_buffer_changed";
    }

    if (numTris == 0)
    {
        strstr << "_validation_only";
    }

    if (useFBO)
    {
        strstr << "_render_to_texture";
    }

    if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
    {
        strstr << "_null";
    }

    return strstr.str();
}

using namespace angle::egl_platform;

DrawElementsPerfParams DrawElementsPerfWithChangedIndexBufferD3D11Params(bool useNullDevice,
                                                                         bool renderToTexture)
{
    DrawElementsPerfParams params;
    params.eglParameters      = useNullDevice ? D3D11_NULL() : D3D11();
    params.useFBO             = renderToTexture;
    params.indexBufferChanged = true;
    return params;
}

DrawElementsPerfParams DrawElementsPerfD3D11Params(bool useNullDevice, bool renderToTexture)
{
    DrawElementsPerfParams params;
    params.eglParameters = useNullDevice ? D3D11_NULL() : D3D11();
    params.useFBO        = renderToTexture;
    return params;
}

DrawElementsPerfParams DrawElementsPerfD3D9Params(bool useNullDevice, bool renderToTexture)
{
    DrawElementsPerfParams params;
    params.eglParameters = useNullDevice ? D3D9_NULL() : D3D9();
    params.useFBO        = renderToTexture;
    return params;
}

DrawElementsPerfParams DrawElementsPerfOpenGLParams(bool useNullDevice, bool renderToTexture)
{
    DrawElementsPerfParams params;
    params.eglParameters = useNullDevice ? OPENGL_NULL() : OPENGL();
    params.useFBO        = renderToTexture;
    return params;
}

DrawElementsPerfParams DrawElementsPerfValidationOnly()
{
    DrawElementsPerfParams params;
    params.eglParameters = DEFAULT();
    params.iterations    = 10000;
    params.numTris       = 0;
    return params;
}
