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

//
// Implementation of Generic Vertex Attribute formats for ES3.1
//
struct AttributeFormat final : angle::NonCopyable
{
    explicit AttributeFormat(GLuint formatIndex);
    AttributeFormat(AttributeFormat &&format);
    AttributeFormat &operator=(AttributeFormat &&format);

    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLuint bindingIndex;  // Index of AttributeBinding object in VertexArray
    GLintptr relativeOffset;
};

//
// Implementation of Generic Vertex Attribute.
// Including an AttributeFormat and a pointer to the AttributeBinding array.
//
struct VertexAttribute final : angle::NonCopyable
{
  public:
    VertexAttribute(VertexAttribute &&attrib);
    VertexAttribute(GLuint index, const std::vector<AttributeBinding> *bindings);

    VertexAttribute &operator=(VertexAttribute &&attrib);

    const AttributeBinding &getBinding() const { return (*bindings)[format.bindingIndex]; }

    bool enabled;           // For glEnable/DisableVertexAttribArray
    const GLvoid *pointer;  // For VERTEX_ATTRIB_ARRAY_POINTER
    GLuint stride;  // Only for queries of VERTEX_ATTRIB_ARRAY_STRIDE and shouldn't be used for real
                    // rendering
    AttributeFormat format;

  private:
    friend class VertexArray;
    const std::vector<AttributeBinding> *bindings;
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);
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
