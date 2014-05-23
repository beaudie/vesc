#include "precompiled.h"
//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer9_utils.cpp: Conversion functions and other utility routines
// specific to the D3D9 renderer.

#include "libGLESv2/renderer/d3d9/renderer9_utils.h"
#include "libGLESv2/renderer/d3d9/formatutils9.h"
#include "libGLESv2/formatutils.h"
#include "common/mathutil.h"
#include "libGLESv2/Context.h"
#include "libGLESv2/caps_utils.h"

#include "common/debug.h"

#include "third_party/systeminfo/SystemInfo.h"

namespace rx
{

namespace gl_d3d9
{

D3DCMPFUNC ConvertComparison(GLenum comparison)
{
    D3DCMPFUNC d3dComp = D3DCMP_ALWAYS;
    switch (comparison)
    {
      case GL_NEVER:    d3dComp = D3DCMP_NEVER;        break;
      case GL_ALWAYS:   d3dComp = D3DCMP_ALWAYS;       break;
      case GL_LESS:     d3dComp = D3DCMP_LESS;         break;
      case GL_LEQUAL:   d3dComp = D3DCMP_LESSEQUAL;    break;
      case GL_EQUAL:    d3dComp = D3DCMP_EQUAL;        break;
      case GL_GREATER:  d3dComp = D3DCMP_GREATER;      break;
      case GL_GEQUAL:   d3dComp = D3DCMP_GREATEREQUAL; break;
      case GL_NOTEQUAL: d3dComp = D3DCMP_NOTEQUAL;     break;
      default: UNREACHABLE();
    }

    return d3dComp;
}

D3DCOLOR ConvertColor(gl::ColorF color)
{
    return D3DCOLOR_RGBA(gl::unorm<8>(color.red),
                         gl::unorm<8>(color.green),
                         gl::unorm<8>(color.blue),
                         gl::unorm<8>(color.alpha));
}

D3DBLEND ConvertBlendFunc(GLenum blend)
{
    D3DBLEND d3dBlend = D3DBLEND_ZERO;

    switch (blend)
    {
      case GL_ZERO:                     d3dBlend = D3DBLEND_ZERO;           break;
      case GL_ONE:                      d3dBlend = D3DBLEND_ONE;            break;
      case GL_SRC_COLOR:                d3dBlend = D3DBLEND_SRCCOLOR;       break;
      case GL_ONE_MINUS_SRC_COLOR:      d3dBlend = D3DBLEND_INVSRCCOLOR;    break;
      case GL_DST_COLOR:                d3dBlend = D3DBLEND_DESTCOLOR;      break;
      case GL_ONE_MINUS_DST_COLOR:      d3dBlend = D3DBLEND_INVDESTCOLOR;   break;
      case GL_SRC_ALPHA:                d3dBlend = D3DBLEND_SRCALPHA;       break;
      case GL_ONE_MINUS_SRC_ALPHA:      d3dBlend = D3DBLEND_INVSRCALPHA;    break;
      case GL_DST_ALPHA:                d3dBlend = D3DBLEND_DESTALPHA;      break;
      case GL_ONE_MINUS_DST_ALPHA:      d3dBlend = D3DBLEND_INVDESTALPHA;   break;
      case GL_CONSTANT_COLOR:           d3dBlend = D3DBLEND_BLENDFACTOR;    break;
      case GL_ONE_MINUS_CONSTANT_COLOR: d3dBlend = D3DBLEND_INVBLENDFACTOR; break;
      case GL_CONSTANT_ALPHA:           d3dBlend = D3DBLEND_BLENDFACTOR;    break;
      case GL_ONE_MINUS_CONSTANT_ALPHA: d3dBlend = D3DBLEND_INVBLENDFACTOR; break;
      case GL_SRC_ALPHA_SATURATE:       d3dBlend = D3DBLEND_SRCALPHASAT;    break;
      default: UNREACHABLE();
    }

    return d3dBlend;
}

D3DBLENDOP ConvertBlendOp(GLenum blendOp)
{
    D3DBLENDOP d3dBlendOp = D3DBLENDOP_ADD;

    switch (blendOp)
    {
      case GL_FUNC_ADD:              d3dBlendOp = D3DBLENDOP_ADD;         break;
      case GL_FUNC_SUBTRACT:         d3dBlendOp = D3DBLENDOP_SUBTRACT;    break;
      case GL_FUNC_REVERSE_SUBTRACT: d3dBlendOp = D3DBLENDOP_REVSUBTRACT; break;
      case GL_MIN_EXT:               d3dBlendOp = D3DBLENDOP_MIN;         break;
      case GL_MAX_EXT:               d3dBlendOp = D3DBLENDOP_MAX;         break;
      default: UNREACHABLE();
    }

    return d3dBlendOp;
}

D3DSTENCILOP ConvertStencilOp(GLenum stencilOp)
{
    D3DSTENCILOP d3dStencilOp = D3DSTENCILOP_KEEP;

    switch (stencilOp)
    {
      case GL_ZERO:      d3dStencilOp = D3DSTENCILOP_ZERO;    break;
      case GL_KEEP:      d3dStencilOp = D3DSTENCILOP_KEEP;    break;
      case GL_REPLACE:   d3dStencilOp = D3DSTENCILOP_REPLACE; break;
      case GL_INCR:      d3dStencilOp = D3DSTENCILOP_INCRSAT; break;
      case GL_DECR:      d3dStencilOp = D3DSTENCILOP_DECRSAT; break;
      case GL_INVERT:    d3dStencilOp = D3DSTENCILOP_INVERT;  break;
      case GL_INCR_WRAP: d3dStencilOp = D3DSTENCILOP_INCR;    break;
      case GL_DECR_WRAP: d3dStencilOp = D3DSTENCILOP_DECR;    break;
      default: UNREACHABLE();
    }

    return d3dStencilOp;
}

D3DTEXTUREADDRESS ConvertTextureWrap(GLenum wrap)
{
    D3DTEXTUREADDRESS d3dWrap = D3DTADDRESS_WRAP;

    switch (wrap)
    {
      case GL_REPEAT:            d3dWrap = D3DTADDRESS_WRAP;   break;
      case GL_CLAMP_TO_EDGE:     d3dWrap = D3DTADDRESS_CLAMP;  break;
      case GL_MIRRORED_REPEAT:   d3dWrap = D3DTADDRESS_MIRROR; break;
      default: UNREACHABLE();
    }

    return d3dWrap;
}

D3DCULL ConvertCullMode(GLenum cullFace, GLenum frontFace)
{
    D3DCULL cull = D3DCULL_CCW;
    switch (cullFace)
    {
      case GL_FRONT:
        cull = (frontFace == GL_CCW ? D3DCULL_CW : D3DCULL_CCW);
        break;
      case GL_BACK:
        cull = (frontFace == GL_CCW ? D3DCULL_CCW : D3DCULL_CW);
        break;
      case GL_FRONT_AND_BACK:
        cull = D3DCULL_NONE; // culling will be handled during draw
        break;
      default: UNREACHABLE();
    }

    return cull;
}

D3DCUBEMAP_FACES ConvertCubeFace(GLenum cubeFace)
{
    D3DCUBEMAP_FACES face = D3DCUBEMAP_FACE_POSITIVE_X;

    switch (cubeFace)
    {
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        face = D3DCUBEMAP_FACE_POSITIVE_X;
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        face = D3DCUBEMAP_FACE_NEGATIVE_X;
        break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        face = D3DCUBEMAP_FACE_POSITIVE_Y;
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        face = D3DCUBEMAP_FACE_NEGATIVE_Y;
        break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        face = D3DCUBEMAP_FACE_POSITIVE_Z;
        break;
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        face = D3DCUBEMAP_FACE_NEGATIVE_Z;
        break;
      default: UNREACHABLE();
    }

    return face;
}

DWORD ConvertColorMask(bool red, bool green, bool blue, bool alpha)
{
    return (red   ? D3DCOLORWRITEENABLE_RED   : 0) |
           (green ? D3DCOLORWRITEENABLE_GREEN : 0) |
           (blue  ? D3DCOLORWRITEENABLE_BLUE  : 0) |
           (alpha ? D3DCOLORWRITEENABLE_ALPHA : 0);
}

D3DTEXTUREFILTERTYPE ConvertMagFilter(GLenum magFilter, float maxAnisotropy)
{
    if (maxAnisotropy > 1.0f)
    {
        return D3DTEXF_ANISOTROPIC;
    }

    D3DTEXTUREFILTERTYPE d3dMagFilter = D3DTEXF_POINT;
    switch (magFilter)
    {
      case GL_NEAREST: d3dMagFilter = D3DTEXF_POINT;  break;
      case GL_LINEAR:  d3dMagFilter = D3DTEXF_LINEAR; break;
      default: UNREACHABLE();
    }

    return d3dMagFilter;
}

void ConvertMinFilter(GLenum minFilter, D3DTEXTUREFILTERTYPE *d3dMinFilter, D3DTEXTUREFILTERTYPE *d3dMipFilter, float maxAnisotropy)
{
    switch (minFilter)
    {
      case GL_NEAREST:
        *d3dMinFilter = D3DTEXF_POINT;
        *d3dMipFilter = D3DTEXF_NONE;
        break;
      case GL_LINEAR:
        *d3dMinFilter = D3DTEXF_LINEAR;
        *d3dMipFilter = D3DTEXF_NONE;
        break;
      case GL_NEAREST_MIPMAP_NEAREST:
        *d3dMinFilter = D3DTEXF_POINT;
        *d3dMipFilter = D3DTEXF_POINT;
        break;
      case GL_LINEAR_MIPMAP_NEAREST:
        *d3dMinFilter = D3DTEXF_LINEAR;
        *d3dMipFilter = D3DTEXF_POINT;
        break;
      case GL_NEAREST_MIPMAP_LINEAR:
        *d3dMinFilter = D3DTEXF_POINT;
        *d3dMipFilter = D3DTEXF_LINEAR;
        break;
      case GL_LINEAR_MIPMAP_LINEAR:
        *d3dMinFilter = D3DTEXF_LINEAR;
        *d3dMipFilter = D3DTEXF_LINEAR;
        break;
      default:
        *d3dMinFilter = D3DTEXF_POINT;
        *d3dMipFilter = D3DTEXF_NONE;
        UNREACHABLE();
    }

    if (maxAnisotropy > 1.0f)
    {
        *d3dMinFilter = D3DTEXF_ANISOTROPIC;
    }
}

}

namespace d3d9_gl
{

gl::Caps GenerateCaps(IDirect3D9 *d3d9, IDirect3DDevice9 *device, D3DDEVTYPE deviceType, UINT adapter)
{
    gl::Caps caps;

    D3DCAPS9 deviceCaps;
    if (FAILED(d3d9->GetDeviceCaps(adapter, deviceType, &deviceCaps)))
    {
        // Can't continue with out device caps
        return caps;
    }

    D3DDISPLAYMODE currentDisplayMode;
    d3d9->GetAdapterDisplayMode(adapter, &currentDisplayMode);

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (gl::FormatSet::const_iterator i = allFormats.begin(); i != allFormats.end(); ++i)
    {
        GLenum internalFormat = *i;

        gl::TextureCaps textureCaps;

        D3DFORMAT textureFormat = gl_d3d9::GetTextureFormat(internalFormat);
        D3DFORMAT renderFormat = gl_d3d9::GetRenderFormat(internalFormat);

        textureCaps.setTexture2DSupport(SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                          0, D3DRTYPE_TEXTURE, textureFormat)));

        textureCaps.setTextureCubeMapSupport(SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                               0, D3DRTYPE_CUBETEXTURE, textureFormat)));

        // D3D9 Renderer doesn't support 3D textures
        //textureCaps.setTexture3DSupport(SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
        //                                                                  0, D3DRTYPE_VOLUMETEXTURE, textureFormat)));
        textureCaps.setTexture3DSupport(false);

        // D3D9 doesn't support 2D array textures
        textureCaps.setTexture2DArraySupport(false);

        textureCaps.setTextureFilteringSupport(SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                 D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, textureFormat)));

        textureCaps.setColorRenderingSupport((SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, textureFormat)) ||
                                              SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, renderFormat))));

        textureCaps.setDepthRenderingSupport(gl::GetDepthBits(internalFormat) > 0 &&
                                             (SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, textureFormat)) ||
                                              SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, renderFormat))));

        textureCaps.setStencilRenderingSupport(gl::GetStencilBits(internalFormat) > 0 &&
                                               (SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                  D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, textureFormat)) ||
                                                SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                                                                  D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, renderFormat))));

        textureCaps.addSampleSupport(1);
        for (size_t i = D3DMULTISAMPLE_2_SAMPLES; i <= D3DMULTISAMPLE_16_SAMPLES; i++)
        {
            D3DMULTISAMPLE_TYPE multisampleType = D3DMULTISAMPLE_TYPE(i);

            HRESULT result = d3d9->CheckDeviceMultiSampleType(adapter, deviceType, textureFormat, TRUE, multisampleType, NULL);
            if (SUCCEEDED(result))
            {
                textureCaps.addSampleSupport(i);
            }
        }

        caps.setTextureFormatCaps(internalFormat, textureCaps);
    }

    const gl::TextureFormatCapsMap &formatSupport = caps.getTextureFormatCapsMap();

    caps.set32BitIndexSupport(deviceCaps.MaxVertexIndex >= (1 << 16));
    caps.setPackedDepthStencilSupport(true);
    caps.setProgramBinarySupport(true);
    caps.setRGB8AndRGBA8TextureSupport(true);
    caps.setBGRA8TextureSupport(gl::GetBGRA8TextureSupport(formatSupport));
    caps.setBGRAReadSupport(true);
    caps.setPixelBufferObjectSupport(false);
    caps.setMapBufferSupport(false);
    caps.setMapBufferRangeSupport(false);
    caps.setHalfFloatTextureSupport(gl::GetHalfFloatTextureSupport(formatSupport));
    caps.setHalfFloatTextureFilteringSupport(gl::GetHalfFloatTextureFilteringSupport(formatSupport));
    caps.setFloatTextureSupport(gl::GetFloatTextureSupport(formatSupport));
    caps.setFloatTextureFilteringSupport(gl::GetFloatTextureFilteringSupport(formatSupport));
    caps.setRGTextureSupport(gl::GetRGTextureSupport(formatSupport, caps.getHalfFloatTextureSupport(), caps.getFloatTextureSupport()));
    caps.setDXT1TextureSupport(gl::GetDXT1TextureSupport(formatSupport));
    caps.setDXT3TextureSupport(gl::GetDXT3TextureSupport(formatSupport));
    caps.setDXT5TextureSupport(gl::GetDXT5TextureSupport(formatSupport));
    caps.setDepthTextureSupport(gl::GetDepthTextureSupport(formatSupport));

    // ATI cards on XP have problems with non-power-of-two textures.
    D3DADAPTER_IDENTIFIER9 adapterId = { 0 };
    if (SUCCEEDED(d3d9->GetAdapterIdentifier(adapter, 0, &adapterId)))
    {
        caps.setNPOTTextureSupport(!(deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2) &&
                                   !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2) &&
                                   !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) &&
                                   !(isWindowsVistaOrGreater() && adapterId.VendorId == VENDOR_ID_AMD));
    }
    else
    {
        caps.setNPOTTextureSupport(false);
    }

    caps.setDrawBuffersSupport(false);
    caps.setTextureStorageSupport(true);

    // Must support a minimum of 2:1 anisotropy for max anisotropy to be considered supported, per the spec
    caps.setAnisotropicFilteringSupport((deviceCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && deviceCaps.MaxAnisotropy >= 2);
    caps.setMaxTextureAnisotropy(static_cast<GLfloat>(deviceCaps.MaxAnisotropy));

    // Check occlusion query support by trying to create one
    IDirect3DQuery9 *occlusionQuery = NULL;
    caps.setOcclusionQuerySupport(SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &occlusionQuery)) && occlusionQuery);
    SafeRelease(occlusionQuery);

    // Check event query support by trying to create one
    IDirect3DQuery9 *eventQuery = NULL;
    caps.setFenceQuerySupport(SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_EVENT, &eventQuery)) && eventQuery);
    SafeRelease(eventQuery);

    caps.setANGLETimerQuerySupport(false); // Unimplemented
    caps.setRobustnessSupport(true);
    caps.setMinMaxBlendingSupport(true);
    caps.setANGLEFramebufferBlitSupport(true);
    caps.setANGLEFramebufferMultisampleSupport(gl::GetMaximumSupportedSamples(formatSupport) > 1);
    caps.setANGLEInstancedArraysSupport(deviceCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0));
    caps.setANGLEPackReverseRowOrderSupport(true);
    caps.setStandardDerivativeSupport(true);
    caps.setTextureLODSupport(true);
    caps.setFragDepthSupport(true);
    caps.setANGLETextureUsageSupport(true);
    caps.setANGLETranslatedSourceSupport(true);
    caps.setColorBufferFloatSupport(gl::GetColorBufferFloatSupport(formatSupport));

    return caps;
}

}

}
