//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Structure describing vertex buffer binding information
// Representing a vertex buffer binding point in ES3.1
//

#ifndef LIBANGLE_VERTEXBUFFERBINDING_H_
#define LIBANGLE_VERTEXBUFFERBINDING_H_

#include "libANGLE/Buffer.h"

namespace gl
{

struct VertexBufferBinding
{
    size_t index;
    GLuint stride;  // 0 means natural stride
    GLuint divisor;

    union {
        const GLvoid *pointer;
        GLintptr offset;
    };
    size_t attribIndex;
    BindingPointer<Buffer> buffer;

    VertexBufferBinding();
};

}  // namespace gl

#endif  // LIBANGLE_VERTEXBUFFERBINDING_H_