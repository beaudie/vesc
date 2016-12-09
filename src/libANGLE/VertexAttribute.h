//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper structure describing a single vertex attribute.
//

#ifndef LIBANGLE_VERTEXATTRIBUTE_H_
#define LIBANGLE_VERTEXATTRIBUTE_H_

#include "libANGLE/Buffer.h"

namespace gl
{

//
// Implementation of Generic Vertex Attribute Binding for ES3.1
//
struct VertexBinding
{
    VertexBinding();

    GLuint stride;
    GLuint divisor;
    GLintptr offset;

    BindingPointer<Buffer> buffer;
};

// Implementation of Generic Vertex Attributes for ES3.1
struct VertexAttribute
{
    explicit VertexAttribute(GLuint attribIndex);

    bool enabled; // From glEnable/DisableVertexAttribArray

    const void *pointer;
    GLuint stride;  // For queries of VERTEX_ATTRIB_ARRAY_STRIDE
    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLuint bindingIndex;  // Index of VertexBinding object in VertexArray
    GLintptr relativeOffset;
};

// Wrapper of a VertexAttribute and its VertexBinding
struct VertexAttribBinding
{
    explicit VertexAttribBinding(const VertexAttribute &attrib, const VertexBinding &binding);

    const VertexAttribute &attrib;
    const VertexBinding &binding;
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib);
size_t ComputeVertexAttributeStride(const VertexAttribute &attrib, const VertexBinding &binding);

size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          const VertexBinding &binding,
                                          size_t drawCount,
                                          size_t instanceCount);

GLintptr ComputeVertexAttributeOffset(const VertexAttribute &attrib, const VertexBinding &binding);

struct VertexAttribCurrentValueData
{
    union
    {
        GLfloat FloatValues[4];
        GLint IntValues[4];
        GLuint UnsignedIntValues[4];
    };
    GLenum Type;

    VertexAttribCurrentValueData();

    void setFloatValues(const GLfloat floatValues[4]);
    void setIntValues(const GLint intValues[4]);
    void setUnsignedIntValues(const GLuint unsignedIntValues[4]);
};

bool operator==(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);
bool operator!=(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);

}  // namespace gl

#include "VertexAttribute.inl"

#endif // LIBANGLE_VERTEXATTRIBUTE_H_
