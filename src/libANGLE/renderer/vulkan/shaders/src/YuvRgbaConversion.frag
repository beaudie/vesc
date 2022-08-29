//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// YuvRgbaConversion.grab: Convert YUV to RGBA via fullscreen draw. Assumes sampler conversion object is available.

#version 450 core

layout (push_constant) uniform PushConstants
{
    vec2 invExtents;
};

// We use yuvImage with a sampler conversion object, so it's implicitly rgba
// once imageStore happens.
layout (set = 0, binding = 0) uniform sampler2D yuvImage;

layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(texture(yuvImage, gl_FragCoord.xy * invExtents).rgb, 1.0);
}
