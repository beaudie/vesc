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
#include "libANGLE/VertexArray.h"

namespace gl
{

//
// Implementation of Vertex Buffer Binding Point in ES3.1
//
struct VertexBufferBinding
{
    GLuint stride;  // VERTEX_BINDING_STRIDE
    GLuint divisor;

    union {
        const GLvoid *bindingPointer;
        GLintptr bindingOffset;
    };
    BindingPointer<Buffer> buffer;

    VertexBufferBinding();
};

// Implementation of Vertex Attribute Format Information Record in ES3.1
struct VertexAttribute
{
    bool enabled; // From glEnable/DisableVertexAttribArray

    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLintptr relativeOffset;

    size_t bindingIndex;  // Index of VertexBufferBinding object in VertexArray

    GLuint vertexAttribArrayStride;  // For queries of VERTEX_ATTRIB_ARRAY_STRIDE

    explicit VertexAttribute(size_t attribIndex);
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib);
size_t ComputeVertexAttributeStride(const VertexAttribute &attrib,
                                    const VertexBufferBinding &binding);
size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          const VertexBufferBinding &binding,
                                          size_t drawCount,
                                          size_t instanceCount);

GLintptr ComputeVertexAttributeOffset(const VertexAttribute &attrib,
                                      const VertexBufferBinding &binding);

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
