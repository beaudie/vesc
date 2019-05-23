//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResolveDepthStencil.frag: Resolve multisampled depth/stencil images.

#version 450 core

#extension GL_EXT_samplerless_texture_functions : require
#extension GL_ARB_shader_stencil_export : require

#define MAKE_SRC_RESOURCE(prefix, type) prefix ## type

#define DEPTH_SRC_RESOURCE(type) type
#define STENCIL_SRC_RESOURCE(type) MAKE_SRC_RESOURCE(i, type)

#if SrcIsArray
#define SRC_RESOURCE_NAME texture2DMSArray
#else
#define SRC_RESOURCE_NAME texture2DMS
#endif

#if ResolveDepth
#define IsResolveDepth 1
#elif ResolveStencil
#define IsResolveStencil 1
#elif ResolveDepthStencil
#define IsResolveDepth 1
#define IsResolveStencil 1
#else
#error "Not all resolve targets are accounted for"
#endif

layout(push_constant) uniform PushConstants {
    // Translation from source to destination coordinates.
    ivec2 srcOffset;
    ivec2 destOffset;
    int srcLayer;
    // Flip control.
    bool flipX;
    bool flipY;
} params;

#if IsResolveDepth
layout(set = 0, binding = 0) uniform DEPTH_SRC_RESOURCE(SRC_RESOURCE_NAME) depth;
#endif
#if IsResolveStencil
layout(set = 0, binding = 1) uniform STENCIL_SRC_RESOURCE(SRC_RESOURCE_NAME) stencil;
#endif

void main()
{
    ivec2 destSubImageCoords = ivec2(gl_FragCoord.xy) - params.destOffset;

    ivec2 srcSubImageCoords = destSubImageCoords;

    // If flipping, srcOffset would contain the opposite coordinates, so we can
    // simply reverse the direction in which x/y grows.
    if (params.flipX)
        srcSubImageCoords.x = -srcSubImageCoords.x;
    if (params.flipY)
        srcSubImageCoords.y = -srcSubImageCoords.y;

    ivec2 srcImageCoords = params.srcOffset + srcSubImageCoords;

    // TODO: clamp the image coords to image size?

    // TODO: enable per-sample shading in the pipeline

    // Note: always resolve using sample 0.  GLES3 gives us freedom in choosing how to resolve
    // depth/stencil images.

#if IsResolveDepth
#if SrcIsArray
    float depthValue = texelFetch(depth, ivec3(srcImageCoords, params.srcLayer), 0).x;
#else
    float depthValue = texelFetch(depth, srcImageCoords, 0).x;
#endif

    gl_FragDepth = depthValue;
#endif

#if IsResolveStencil
#if SrcIsArray
    int stencilValue = texelFetch(stencil, ivec3(srcImageCoords, params.srcLayer), 0).x;
#else
    int stencilValue = texelFetch(stencil, srcImageCoords, 0).x;
#endif

    gl_FragStencilRefARB = stencilValue;
#endif
}
