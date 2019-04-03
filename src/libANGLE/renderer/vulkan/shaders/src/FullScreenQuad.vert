//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FullScreenQuad.vert: Simple full-screen quad vertex shader.

#version 450 core

// This push constant is placed in the range 0-16, so any fragment shader that uses this must have
// its push constants start at offset 16.
layout(push_constant) uniform PushConstants {
    float depth;
    // Padding to align to 16 bytes.
    uint _[3];
} params;

const vec2 kQuadVertices[] = {
    vec2(-1, 1),
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, -1),
    vec2(1, 1),
};

void main()
{
    gl_Position = vec4(kQuadVertices[gl_VertexIndex], params.depth, 1);
}
