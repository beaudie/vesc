//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unresolve.frag: Read from input attachments and output color.  This shader is used to do the
// opposite of a resolve operation (hence the name unresolve).

#version 450 core

#if IsFloat
#define Type vec4
#define SubpassType subpassInput
#elif IsSint
#define Type ivec4
#define SubpassType isubpassInput
#elif IsUint
#define Type uvec4
#define SubpassType usubpassInput
#else
#error "Not all formats are accounted for"
#endif

#if Attachment0
#define ATTACHMENT 0
#elif Attachment1
#define ATTACHMENT 1
#elif Attachment2
#define ATTACHMENT 2
#elif Attachment3
#define ATTACHMENT 3
#elif Attachment4
#define ATTACHMENT 4
#elif Attachment5
#define ATTACHMENT 5
#elif Attachment6
#define ATTACHMENT 6
#elif Attachment7
#define ATTACHMENT 7
#else
#error "Not all attachment index possibilities are accounted for"
#endif

layout(location = ATTACHMENT) out Type colorOut;
layout(input_attachment_index = ATTACHMENT, set = 0, binding = 0) uniform SubpassType colorIn;

void main()
{
    colorOut = subpassLoad(colorIn);
}
