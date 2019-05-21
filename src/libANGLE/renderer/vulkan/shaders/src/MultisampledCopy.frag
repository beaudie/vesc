//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultisampledCopy.frag: Helper shader for multisampled color resolve.  If one of the framebuffers
// involved in resolve is flipped but not the other, this shader is used to perform the flip.  A
// subpass resolve operation is used after this draw call, avoiding any intermediate copies.

#version 450 core

#extension GL_EXT_samplerless_texture_functions : require

#define MAKE_SRC_RESOURCE(prefix, type) prefix ## type

#if IsFloat
#define SRC_RESOURCE(type) type
#define Type vec4
#elif IsInt
#define SRC_RESOURCE(type) MAKE_SRC_RESOURCE(i, type)
#define Type ivec4
#elif IsUint
#define SRC_RESOURCE(type) MAKE_SRC_RESOURCE(u, type)
#define Type uvec4
#else
#error "Not all formats are accounted for"
#endif

#if SrcIsArray
#define SRC_RESOURCE_NAME texture2DMSArray
#else
#define SRC_RESOURCE_NAME texture2DMS
#endif

layout(push_constant) uniform PushConstants {
    // Robust access.
    ivec2 srcExtent;
    // Translation from source to destination coordinates.
    ivec2 srcOffset;
    ivec2 destOffset;
    int srcLayer;
    // Mask to output only to attachments that are actually present.
    int outputMask;
    // Flip control.
    bool flipX;
    bool flipY;
} params;

layout(set = 0, binding = 0) uniform SRC_RESOURCE(SRC_RESOURCE_NAME) src;

layout(location = 0) out Type colorOut0;
layout(location = 1) out Type colorOut1;
layout(location = 2) out Type colorOut2;
layout(location = 3) out Type colorOut3;
layout(location = 4) out Type colorOut4;
layout(location = 5) out Type colorOut5;
layout(location = 6) out Type colorOut6;
layout(location = 7) out Type colorOut7;

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

    Type srcValue;
    if (any(lessThan(srcImageCoords, ivec2(0))) ||
        any(lessThanEqual(params.srcExtent, srcImageCoords)))
    {
        srcValue = Type(0, 0, 0, 1);
    }
    else
    {
        // TODO: enable per-sample shading in the pipeline
#if SrcIsArray
        srcValue = texelFetch(src, ivec3(srcImageCoords, params.srcLayer), gl_SampleID);
#else
        srcValue = texelFetch(src, srcImageCoords, gl_SampleID);
#endif
    }

    // Note: not exporting to render targets that are not present optimizes the number of export
    // instructions, which would have otherwise been a likely bottleneck.
    if ((params.outputMask & (1 << 0)) != 0)
    {
        colorOut0 = srcValue;
    }
    if ((params.outputMask & (1 << 1)) != 0)
    {
        colorOut1 = srcValue;
    }
    if ((params.outputMask & (1 << 2)) != 0)
    {
        colorOut2 = srcValue;
    }
    if ((params.outputMask & (1 << 3)) != 0)
    {
        colorOut3 = srcValue;
    }
    if ((params.outputMask & (1 << 4)) != 0)
    {
        colorOut4 = srcValue;
    }
    if ((params.outputMask & (1 << 5)) != 0)
    {
        colorOut5 = srcValue;
    }
    if ((params.outputMask & (1 << 6)) != 0)
    {
        colorOut6 = srcValue;
    }
    if ((params.outputMask & (1 << 7)) != 0)
    {
        colorOut7 = srcValue;
    }
}
