//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// load_functions_table:
//   Contains load functions table depending on internal format and dxgi format
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_LOADFUNCTIONSTABLE_H_
#define LIBANGLE_RENDERER_D3D_D3D11_LOADFUNCTIONSTABLE_H_

#include "libANGLE/renderer/d3d/d3d11/renderer11.h"

#include <map>

namespace rx
{

namespace d3d11
{

struct LoadImageFunctionInfo;

typedef std::pair<DXGI_FORMAT, LoadImageFunctionInfo> DxgiFormatLoadFunctionPair;
typedef std::pair<GLenum, DxgiFormatLoadFunctionPair> GLTypeDXGIFunctionPair;
typedef std::map<GLenum, std::vector<GLTypeDXGIFunctionPair>> D3D11LoadFunctionMap;

const std::map<GLenum, LoadImageFunctionInfo> &GetLoadFunctionsMap(GLenum internalFormat,
                                                                   DXGI_FORMAT dxgiFormat);

}  // namespace d3d11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_LOADFUNCTIONSTABLE_H_
