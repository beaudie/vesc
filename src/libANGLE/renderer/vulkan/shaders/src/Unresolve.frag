//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unresolve.frag: Unresolve color or depth/stencil attachments.

#version 450 core
#if UnresolveStencil
#extension GL_ARB_shader_stencil_export : require
#endif

// Use different types (float, int and uint) so that all necessary types would be already present in
// the SPIR-V.
layout(location = 0) out vec4 colorOut0;
layout(location = 1) out ivec4 colorOut1;
layout(location = 2) out uvec4 colorOut2;
layout(location = 3) out vec4 colorOut3;
layout(location = 4) out ivec4 colorOut4;
layout(location = 5) out uvec4 colorOut5;
layout(location = 6) out vec4 colorOut6;
layout(location = 7) out ivec4 colorOut7;
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput colorIn0;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform isubpassInput colorIn1;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform usubpassInput colorIn2;
layout(input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput colorIn3;
layout(input_attachment_index = 4, set = 0, binding = 4) uniform isubpassInput colorIn4;
layout(input_attachment_index = 5, set = 0, binding = 5) uniform usubpassInput colorIn5;
layout(input_attachment_index = 6, set = 0, binding = 6) uniform subpassInput colorIn6;
layout(input_attachment_index = 7, set = 0, binding = 7) uniform isubpassInput colorIn7;
#if UnresolveDepth
layout(input_attachment_index = 8, set = 0, binding = 8) uniform subpassInput depthIn;
#endif
#if UnresolveStencil
layout(input_attachment_index = 8, set = 0, binding = 9) uniform usubpassInput stencilIn;
#endif

void main()
{
    colorOut0 = subpassLoad(colorIn0);
    colorOut1 = subpassLoad(colorIn1);
    colorOut2 = subpassLoad(colorIn2);
    colorOut3 = subpassLoad(colorIn3);
    colorOut4 = subpassLoad(colorIn4);
    colorOut5 = subpassLoad(colorIn5);
    colorOut6 = subpassLoad(colorIn6);
    colorOut7 = subpassLoad(colorIn7);
#if UnresolveDepth
    gl_FragDepth = subpassLoad(depthIn).x;
#endif
#if UnresolveStencil
    gl_FragStencilRefARB = int(subpassLoad(stencilIn).x);
#endif
}
