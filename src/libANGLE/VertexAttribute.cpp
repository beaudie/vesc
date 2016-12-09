//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state classes for mananging GLES 3 Vertex Array Objects.
//

#include "libANGLE/VertexAttribute.h"

namespace gl
{

// [OpenGL ES 3.1] (November 3, 2016) Section 20 Page 361
// Table 20.2: Vertex Array Object State
AttribBinding::AttribBinding() : stride(16u), divisor(0), offset(0)
{
}

AttribBinding::AttribBinding(AttribBinding &&binding)
{
    *this = std::move(binding);
}

AttribBinding &AttribBinding::operator=(AttribBinding &&binding)
{
    if (this != &binding)
    {
        stride  = binding.stride;
        divisor = binding.divisor;
        offset  = binding.offset;

        buffer.set(binding.buffer.get());
        binding.buffer.set(nullptr);
    }
    return *this;
}

AttribFormat::AttribFormat(GLuint formatIndex)
    : enabled(false),
      pointer(nullptr),
      stride(0),
      type(GL_FLOAT),
      size(4u),
      normalized(false),
      pureInteger(false),
      bindingIndex(formatIndex),
      relativeOffset(0)
{
}

VertexAttribute::VertexAttribute(VertexAttribute &&attrib)
    : format(attrib.format), bindings(attrib.bindings)
{
    ASSERT(bindings);
}

VertexAttribute::VertexAttribute(GLuint index, const std::vector<AttribBinding> *bindings)
    : format(index), bindings(bindings)
{
    ASSERT(bindings);
}

VertexAttribute &VertexAttribute::operator=(VertexAttribute &&attrib)
{
    if (this != &attrib)
    {
        ASSERT(bindings == attrib.bindings);
        format = attrib.format;
    }
    return *this;
}

GLintptr ComputeVertexAttributeOffset(const VertexAttribute &attrib)
{
    return attrib.getAttribBinding().offset + attrib.getAttribFormat().relativeOffset;
}

size_t ComputeVertexAttributeTypeSize(const VertexAttribute& attrib)
{
    const AttribFormat &format = attrib.getAttribFormat();
    GLuint size                = format.size;
    switch (format.type)
    {
      case GL_BYTE:                        return size * sizeof(GLbyte);
      case GL_UNSIGNED_BYTE:               return size * sizeof(GLubyte);
      case GL_SHORT:                       return size * sizeof(GLshort);
      case GL_UNSIGNED_SHORT:              return size * sizeof(GLushort);
      case GL_INT:                         return size * sizeof(GLint);
      case GL_UNSIGNED_INT:                return size * sizeof(GLuint);
      case GL_INT_2_10_10_10_REV:          return 4;
      case GL_UNSIGNED_INT_2_10_10_10_REV: return 4;
      case GL_FIXED:                       return size * sizeof(GLfixed);
      case GL_HALF_FLOAT:                  return size * sizeof(GLhalf);
      case GL_FLOAT:                       return size * sizeof(GLfloat);
      default: UNREACHABLE();              return size * sizeof(GLfloat);
    }
}

size_t ComputeVertexAttributeStride(const VertexAttribute &attrib)
{
    // In ES 3.1, VertexAttribPointer will store the type size in the binding stride.
    // Hence, rendering always uses the binding's stride.
    return attrib.getAttribFormat().enabled ? attrib.getAttribBinding().stride : 16u;
}

size_t ComputeVertexAttributeElementCount(const VertexAttribute &attrib,
                                          size_t drawCount,
                                          size_t instanceCount)
{
    const AttribBinding &binding = attrib.getAttribBinding();
    // For instanced rendering, we draw "instanceDrawCount" sets of "vertexDrawCount" vertices.
    //
    // A vertex attribute with a positive divisor loads one instanced vertex for every set of
    // non-instanced vertices, and the instanced vertex index advances once every "mDivisor"
    // instances.
    if (instanceCount > 0 && binding.divisor > 0)
    {
        // When instanceDrawCount is not a multiple attrib.divisor, the division must round up.
        // For instance, with 5 non-instanced vertices and a divisor equal to 3, we need 2 instanced
        // vertices.
        return (instanceCount + binding.divisor - 1u) / binding.divisor;
    }

    return drawCount;
}

}  // namespace gl
