//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// dxgi_support_table:
//   Queries for DXGI support of various texture formats. Depends on DXGI
//   version, D3D feature level, and is sometimes guaranteed or optional.
//

#include "libANGLE/renderer/d3d/d3d11/dxgi_support_table.h"

#include <map>

namespace rx
{

namespace d3d11
{

namespace
{

std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable10_0;
std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable10_1;
std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable11_0;

} // namespace

#define F_2D D3D11_FORMAT_SUPPORT_TEXTURE2D
#define F_3D D3D11_FORMAT_SUPPORT_TEXTURE3D
#define F_CUBE D3D11_FORMAT_SUPPORT_TEXTURECUBE
#define F_SAMPLE D3D11_FORMAT_SUPPORT_SHADER_SAMPLE
#define F_RT D3D11_FORMAT_SUPPORT_RENDER_TARGET
#define F_MS D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET
#define F_DS D3D11_FORMAT_SUPPORT_DEPTH_STENCIL

void InitializeDXGISupportTables()
{
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8A8_UNORM] =
        DXGISupport(0, 0, F_2D | F_CUBE | F_MS | F_SAMPLE | F_3D | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8A8_UNORM] =
        DXGISupport(0, 0, F_2D | F_CUBE | F_MS | F_SAMPLE | F_3D | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8A8_UNORM] =
        DXGISupport(F_RT | F_SAMPLE, 0, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_R24_UNORM_X8_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_3D | F_RT, F_MS | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_R24_UNORM_X8_TYPELESS] =
        DXGISupport(F_CUBE | F_2D | F_SAMPLE, F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R24_UNORM_X8_TYPELESS] =
        DXGISupport(F_CUBE | F_SAMPLE | F_2D, F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B4G4R4A4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_B4G4R4A4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_B4G4R4A4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC4_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC4_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC4_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC1_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC1_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC1_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_IA44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_IA44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_IA44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8A8_UNORM_SRGB] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8A8_UNORM_SRGB] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8A8_UNORM_SRGB] =
        DXGISupport(F_RT | F_SAMPLE, 0, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_Y216] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_Y216] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_Y216] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_X24_TYPELESS_G8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_X24_TYPELESS_G8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_X24_TYPELESS_G8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32_SINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32_SINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32_SINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_NV12] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_NV12] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_NV12] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_A8P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_A8P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_A8P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_NV11] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_NV11] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_NV11] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_P010] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_P010] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_P010] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_D24_UNORM_S8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_D24_UNORM_S8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_D24_UNORM_S8_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_D32_FLOAT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_D32_FLOAT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_D32_FLOAT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC6H_UF16] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC6H_UF16] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC6H_UF16] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_Y410] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_Y410] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_Y410] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_D16_UNORM] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_D16_UNORM] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_D16_UNORM] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC4_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, 0, F_MS | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32_FLOAT] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D | F_RT, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32_FLOAT] =
        DXGISupport(F_CUBE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32_FLOAT] =
        DXGISupport(F_CUBE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32_FLOAT] =
        DXGISupport(F_CUBE | F_3D | F_2D, 0, F_MS | F_RT | F_DS | F_SAMPLE);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32A32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32A32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32A32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB] =
        DXGISupport(F_RT | F_SAMPLE, 0, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC6H_SF16] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC6H_SF16] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC6H_SF16] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC3_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC3_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC3_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_Y416] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_Y416] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_Y416] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R9G9B9E5_SHAREDEXP] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R9G9B9E5_SHAREDEXP] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R9G9B9E5_SHAREDEXP] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC7_UNORM_SRGB] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC7_UNORM_SRGB] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC7_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC2_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC2_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC2_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R1_UNORM] =
        DXGISupport(F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R1_UNORM] =
        DXGISupport(F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R1_UNORM] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R10G10B10A2_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R10G10B10A2_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R10G10B10A2_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_B8G8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_B8G8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_B8G8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R11G11B10_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R11G11B10_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R11G11B10_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32A32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32A32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32A32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_P8] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC5_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC5_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC5_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R10G10B10A2_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R10G10B10A2_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R10G10B10A2_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32A32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32A32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32A32_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_UINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC3_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC3_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC3_UNORM_SRGB] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC7_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC7_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC7_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8B8A8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8B8A8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8B8A8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC5_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC5_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC5_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC6H_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC6H_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC6H_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC3_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC3_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC3_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] =
        DXGISupport(0, F_CUBE | F_SAMPLE | F_RT, F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_1[DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] =
        DXGISupport(0, F_CUBE | F_SAMPLE | F_RT, F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable11_0[DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] =
        DXGISupport(0, F_CUBE | F_SAMPLE | F_RT, F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_R24G8_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R24G8_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R24G8_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC4_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC4_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC4_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_Y210] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_Y210] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_Y210] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_AYUV] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_AYUV] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_AYUV] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, 0, F_MS | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32_FLOAT] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D | F_RT, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B5G6R5_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_B5G6R5_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_B5G6R5_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R10G10B10A2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R10G10B10A2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R10G10B10A2_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_D32_FLOAT_S8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_D32_FLOAT_S8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_D32_FLOAT_S8X24_UINT] =
        DXGISupport(F_CUBE | F_2D, F_SAMPLE | F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8X8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8X8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8X8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_YUY2] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_YUY2] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_YUY2] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_A8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_2D, F_3D | F_RT, F_MS | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_2D | F_SAMPLE, F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS] =
        DXGISupport(F_CUBE | F_SAMPLE | F_2D, F_3D | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32A32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, 0, F_MS | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32A32_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32A32_FLOAT] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D | F_RT, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8_SINT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_2D, F_SAMPLE, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8A8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8A8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8A8_TYPELESS] =
        DXGISupport(0, F_SAMPLE | F_RT, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_B5G5R5A1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_B5G5R5A1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_B5G5R5A1_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_0[DXGI_FORMAT_420_OPAQUE] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_420_OPAQUE] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_420_OPAQUE] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R8G8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R8G8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R8G8_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32_UINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32_UINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32_UINT] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE, F_MS | F_DS | F_RT);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC7_UNORM] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC7_UNORM] =
        DXGISupport(0, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC7_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_UNKNOWN] =
        DXGISupport(0, F_MS | F_RT | F_CUBE | F_SAMPLE | F_3D | F_DS | F_2D, 0);
    g_DXGISupportTable10_1[DXGI_FORMAT_UNKNOWN] =
        DXGISupport(0, F_MS | F_RT | F_CUBE | F_SAMPLE | F_3D | F_DS | F_2D, 0);
    g_DXGISupportTable11_0[DXGI_FORMAT_UNKNOWN] =
        DXGISupport(0, F_MS | F_RT | F_CUBE | F_SAMPLE | F_3D | F_DS | F_2D, 0);
    g_DXGISupportTable10_0[DXGI_FORMAT_P016] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_P016] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_P016] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32G32B32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32G32B32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32G32B32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_B8G8R8X8_UNORM] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable10_1[DXGI_FORMAT_B8G8R8X8_UNORM] =
        DXGISupport(0, 0, F_CUBE | F_MS | F_2D | F_3D | F_DS | F_SAMPLE);
    g_DXGISupportTable11_0[DXGI_FORMAT_B8G8R8X8_UNORM] =
        DXGISupport(F_RT | F_SAMPLE, 0, F_CUBE | F_MS | F_3D | F_DS | F_2D);
    g_DXGISupportTable10_0[DXGI_FORMAT_G8R8_G8B8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_G8R8_G8B8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_G8R8_G8B8_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16_SNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16G16B16A16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16G16B16A16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16G16B16A16_UNORM] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_2D | F_3D | F_SAMPLE, 0, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R16_FLOAT] =
        DXGISupport(F_CUBE | F_RT | F_SAMPLE | F_3D | F_2D, 0, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_AI44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_AI44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_AI44] =
        DXGISupport(0, F_CUBE | F_RT | F_3D | F_SAMPLE | F_2D, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_R32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_R32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_R32_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC5_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC5_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC5_SNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC1_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC1_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC1_TYPELESS] =
        DXGISupport(F_CUBE | F_3D | F_2D, F_SAMPLE | F_RT, F_MS | F_DS);
    g_DXGISupportTable10_0[DXGI_FORMAT_BC2_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable10_1[DXGI_FORMAT_BC2_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);
    g_DXGISupportTable11_0[DXGI_FORMAT_BC2_UNORM] =
        DXGISupport(F_CUBE | F_SAMPLE | F_3D | F_2D, F_RT, F_MS | F_DS);

}

#undef F_2D
#undef F_3D
#undef F_CUBE
#undef F_SAMPLE
#undef F_RT
#undef F_MS
#undef F_DS

void DeinitializeDXGISupportTables()
{
    g_DXGISupportTable10_0.clear();
    g_DXGISupportTable10_1.clear();
    g_DXGISupportTable11_0.clear();
}

const DXGISupport &GetDXGISupport(DXGI_FORMAT dxgiFormat, D3D_FEATURE_LEVEL featureLevel)
{
    static UINT AllSupportFlags = D3D11_FORMAT_SUPPORT_TEXTURE2D |
                                  D3D11_FORMAT_SUPPORT_TEXTURE3D |
                                  D3D11_FORMAT_SUPPORT_TEXTURECUBE |
                                  D3D11_FORMAT_SUPPORT_SHADER_SAMPLE |
                                  D3D11_FORMAT_SUPPORT_RENDER_TARGET |
                                  D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
                                  D3D11_FORMAT_SUPPORT_DEPTH_STENCIL;
    static const DXGISupport defaultSupport(0, 0, AllSupportFlags);

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_0:
            return g_DXGISupportTable11_0[dxgiFormat];
        case D3D_FEATURE_LEVEL_10_1:
            return g_DXGISupportTable10_1[dxgiFormat];
        case D3D_FEATURE_LEVEL_10_0:
            return g_DXGISupportTable10_0[dxgiFormat];
        default:
            return defaultSupport;
    }
}

} // namespace d3d11

} // namespace rx
