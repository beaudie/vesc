//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Stream11.cpp: Defines the rx::Stream11 class which implements rx::StreamImpl.

#include "libANGLE/renderer/d3d/d3d11/Stream11.h"

#include "common/utilities.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

Stream11::Stream11(Renderer11 *renderer) : mRenderer(renderer)
{
    UNUSED_VARIABLE(mRenderer);
}

Stream11::~Stream11()
{
}

bool Stream11::validateD3D11NV12Texture(void *texture)
{
    ID3D11Texture2D *textureD3D = static_cast<ID3D11Texture2D *>(texture);

    // Check that the texture originated from our device
    ID3D11Device *device;
    textureD3D->GetDevice(&device);
    if (device != mRenderer->getDevice())
    {
        return false;
    }

    // Get the desc
    D3D11_TEXTURE2D_DESC desc;
    textureD3D->GetDesc(&desc);
    if (desc.Format != DXGI_FORMAT_NV12)
    {
        return false;
    }

    return true;
}

void Stream11::referenceD3D11NV12Texture(void *texture)
{
    ID3D11Texture2D *textureD3D = static_cast<ID3D11Texture2D *>(texture);
    textureD3D->AddRef();
}

void Stream11::releaseD3D11NV12Texture(void *texture)
{
    ID3D11Texture2D *textureD3D = static_cast<ID3D11Texture2D *>(texture);
    textureD3D->Release();
}
}  // namespace rx
