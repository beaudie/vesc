//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper structure describing a single vertex attribute.
// Representing a vertex attribute format information record in ES3.1
//

#ifndef LIBANGLE_VERTEXATTRIBUTE_H_
#define LIBANGLE_VERTEXATTRIBUTE_H_

#include "libANGLE/Buffer.h"
#include "libANGLE/VertexBufferBinding.h"

namespace gl
{

struct VertexAttribute
{
    bool enabled;  // From glEnable/DisableVertexAttribArray

    size_t index;
    GLenum type;
    GLuint size;
    bool normalized;
    bool pureInteger;
    GLintptr relativeOffset;

    explicit VertexAttribute(size_t index, VertexBufferBinding *vertexBufferBinding);

    void bindVertexBuffer(VertexBufferBinding *vertexBufferBinding);
    VertexBufferBinding *vertexBufferBinding() const { return binding; }
    size_t bindingIndex() const;

    GLuint stride() const;
    GLuint divisor() const;

    GLintptr vertexBindingOffset() const;
    GLintptr offset() const;
    GLvoid *pointer() const;

    BindingPointer<Buffer> *buffer() const;

  private:
    VertexBufferBinding *binding;
};

bool operator==(const VertexAttribute &a, const VertexAttribute &b);
bool operator!=(const VertexAttribute &a, const VertexAttribute &b);

size_t ComputeVertexAttributeTypeSize(const VertexAttribute& attrib);
size_t ComputeVertexAttributeStride(const VertexAttribute& attrib);
size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          size_t drawCount,
                                          size_t instanceCount);

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
