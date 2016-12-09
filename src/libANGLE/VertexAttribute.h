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
// Implementation of Generic Vertex Attribute Bindings for ES3.1
//
struct VertexBinding final : angle::NonCopyable
{
    VertexBinding();
    VertexBinding(VertexBinding &&binding);
    VertexBinding &operator=(VertexBinding &&binding);

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

// Wrapper of a VertexAttribute and its VertexBinding.
// In many situations it will be more convenient to use this wrapper.
struct VertexInfo final : angle::NonCopyable
{
    VertexInfo(VertexInfo &&vertexInfo);
    VertexInfo(GLuint attribIndex, VertexBinding *binding);
    VertexInfo &operator=(VertexInfo &&vertexInfo);

    VertexAttribute attrib;
    VertexBinding *binding;
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);
bool operator==(const VertexBinding &a, const VertexBinding &b);
bool operator!=(const VertexBinding &a, const VertexBinding &b);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib);
size_t ComputeVertexAttributeStride(const VertexInfo &vertexInfo);

size_t ComputeVertexAttributeElementCount(const VertexInfo &vertexInfo,
                                          size_t drawCount,
                                          size_t instanceCount);

GLintptr ComputeVertexAttributeOffset(const VertexInfo &vertexInfo);

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
