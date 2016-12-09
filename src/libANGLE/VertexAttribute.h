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
class VertexArray;

//
// Implementation of Generic Vertex Attribute Bindings for ES3.1
//
struct AttributeBinding final : angle::NonCopyable
{
    AttributeBinding();
    AttributeBinding(AttributeBinding &&binding);
    AttributeBinding &operator=(AttributeBinding &&binding);

    GLuint stride;
    GLuint divisor;
    GLintptr offset;

    BindingPointer<Buffer> buffer;
};

// Implementation of Generic Vertex Attribute formats for ES3.1
struct AttributeFormat
{
    explicit AttributeFormat(GLuint formatIndex);

    bool enabled; // From glEnable/DisableVertexAttribArray

    const GLvoid *pointer;
    GLuint stride;  // For queries of VERTEX_ATTRIB_ARRAY_STRIDE
    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLuint bindingIndex;  // Index of VertexBinding object in VertexArray
    GLintptr relativeOffset;
};

// Wrapper of an AttribFormat and a pointer to its AttribBinding array
// In many situations it will be more convenient to use this wrapper
struct VertexAttribute final : angle::NonCopyable
{
  public:
    VertexAttribute(VertexAttribute &&attrib);
    VertexAttribute(GLuint index, const std::vector<AttributeBinding> *bindings);

    VertexAttribute &operator=(VertexAttribute &&attrib);

    const AttributeFormat &getFormat() const { return format; }
    const AttributeBinding &getBinding() const { return (*bindings)[format.bindingIndex]; }

  private:
    friend class VertexArray;
    AttributeFormat format;
    const std::vector<AttributeBinding> *bindings;
};

bool operator==(const AttributeFormat &a, const AttributeFormat &b);
bool operator!=(const AttributeFormat &a, const AttributeFormat &b);
bool operator==(const AttributeBinding &a, const AttributeBinding &b);
bool operator!=(const AttributeBinding &a, const AttributeBinding &b);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib);
size_t ComputeVertexAttributeStride(const VertexAttribute &attrib);

size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          size_t drawCount,
                                          size_t instanceCount);

GLintptr ComputeVertexAttributeOffset(const VertexAttribute &attrib);

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
