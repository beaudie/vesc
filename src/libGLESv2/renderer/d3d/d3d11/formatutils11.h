//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils11.h: Queries for GL image formats and their translations to D3D11
// formats.

#ifndef LIBGLESV2_RENDERER_FORMATUTILS11_H_
#define LIBGLESV2_RENDERER_FORMATUTILS11_H_

#include "libGLESv2/formatutils.h"

namespace rx
{

namespace d3d11
{

typedef std::map<std::pair<GLenum, GLenum>, ColorCopyFunction> FastCopyFunctionMap;

struct DXGIFormat
{
    DXGIFormat();

    GLuint pixelBytes;
    GLuint blockWidth;
    GLuint blockHeight;

    GLuint depthBits;
    GLuint depthOffset;
    GLuint stencilBits;
    GLuint stencilOffset;

    GLenum internalFormat;
    GLenum componentType;

    MipGenerationFunction mipGenerationFunction;
    ColorReadFunction colorReadFunction;

    FastCopyFunctionMap fastCopyFunctions;
};
const DXGIFormat &GetDXGIFormatInfo(DXGI_FORMAT format);

}

namespace gl_d3d11
{

struct D3D11Format
{
    D3D11Format();

    DXGI_FORMAT texFormat;
    DXGI_FORMAT srvFormat;
    DXGI_FORMAT rtvFormat;
    DXGI_FORMAT dsvFormat;
    DXGI_FORMAT renderFormat;

    DXGI_FORMAT swizzleTexFormat;
    DXGI_FORMAT swizzleSRVFormat;
    DXGI_FORMAT swizzleRTVFormat;

    InitializeTextureDataFunction dataInitializerFunction;

    typedef std::map<GLenum, LoadImageFunction> LoadFunctionMap;
    LoadFunctionMap loadFunctions;
};
const D3D11Format &GetD3D11FormatInfo(GLenum internalFormat);

struct D3D11VertexFormat
{
    D3D11VertexFormat();

    VertexConversionType conversionType;
    DXGI_FORMAT nativeFormat;
    VertexCopyFunction copyFunction;
};
const D3D11VertexFormat &GetD3D11VertexInfo(const gl::VertexFormat &vertexFormat);

}

}

#endif // LIBGLESV2_RENDERER_FORMATUTILS11_H_
