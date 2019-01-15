//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Helper structures about Generic Vertex Attribute.
//

#ifndef LIBANGLE_VERTEXATTRIBUTE_H_
#define LIBANGLE_VERTEXATTRIBUTE_H_

#include "libANGLE/Buffer.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class VertexArray;

//
// Implementation of Generic Vertex Attribute Bindings for ES3.1. The members are intentionally made
// private in order to hide implementation details.
//
class VertexBinding final : angle::NonCopyable
{
  public:
    VertexBinding();
    explicit VertexBinding(GLuint boundAttribute);
    VertexBinding(VertexBinding &&binding);
    ~VertexBinding();
    VertexBinding &operator=(VertexBinding &&binding);

    GLuint getStride() const { return mStride; }
    void setStride(GLuint strideIn) { mStride = strideIn; }

    GLuint getDivisor() const { return mDivisor; }
    void setDivisor(GLuint divisorIn) { mDivisor = divisorIn; }

    GLintptr getOffset() const { return mOffset; }
    void setOffset(GLintptr offsetIn) { mOffset = offsetIn; }

    const BindingPointer<Buffer> &getBuffer() const { return mBuffer; }

    ANGLE_INLINE void setBuffer(const gl::Context *context, Buffer *bufferIn, bool containerIsBound)
    {
        if (containerIsBound)
        {
            if (mBuffer.get())
                mBuffer->onNonTFBindingChanged(-1);
            if (bufferIn)
                bufferIn->onNonTFBindingChanged(1);
        }
        mBuffer.set(context, bufferIn);
    }

    void onContainerBindingChanged(const Context *context, int incr) const;

    const AttributesMask &getBoundAttributesMask() const { return mBoundAttributesMask; }

    void setBoundAttribute(size_t index) { mBoundAttributesMask.set(index); }

    void resetBoundAttribute(size_t index) { mBoundAttributesMask.reset(index); }

  private:
    GLuint mStride;
    GLuint mDivisor;
    GLintptr mOffset;

    BindingPointer<Buffer> mBuffer;

    // Mapping from this binding to all of the attributes that are using this binding.
    AttributesMask mBoundAttributesMask;
};

//
// Implementation of Generic Vertex Attributes for ES3.1
//
struct VertexAttribute final : private angle::NonCopyable
{
    explicit VertexAttribute(GLuint bindingIndex);
    VertexAttribute(VertexAttribute &&attrib);
    VertexAttribute &operator=(VertexAttribute &&attrib);

    // Called from VertexArray.
    void updateCachedElementLimit(const VertexBinding &binding);
    GLint64 getCachedElementLimit() const { return mCachedElementLimit; }

    bool enabled;  // For glEnable/DisableVertexAttribArray
    VertexAttribType type;
    GLuint size;
    bool normalized;
    bool pureInteger;

    const void *pointer;
    GLuint relativeOffset;

    GLuint vertexAttribArrayStride;  // ONLY for queries of VERTEX_ATTRIB_ARRAY_STRIDE
    GLuint bindingIndex;

    // Special value for the cached element limit on the integer overflow case.
    static constexpr GLint64 kIntegerOverflow = std::numeric_limits<GLint64>::min();

  private:
    // This is kept in sync by the VertexArray. It is used to optimize draw call validation.
    GLint64 mCachedElementLimit;
};

namespace priv
{
constexpr angle::PackedEnumMap<VertexAttribType, size_t> kVertexAttribTypeComponentSize = {{
    {VertexAttribType::Byte, sizeof(GLbyte)},
    {VertexAttribType::UnsignedByte, sizeof(GLubyte)},
    {VertexAttribType::Short, sizeof(GLshort)},
    {VertexAttribType::UnsignedShort, sizeof(GLushort)},
    {VertexAttribType::Int, sizeof(GLint)},
    {VertexAttribType::UnsignedInt, sizeof(GLuint)},
    {VertexAttribType::Float, sizeof(GLfloat)},
    {VertexAttribType::HalfFloat, sizeof(GLhalf)},
    {VertexAttribType::Fixed, sizeof(GLfixed)},
    {VertexAttribType::Int2101010, 4},
    {VertexAttribType::UnsignedInt2101010, 4},
}};

// Packed attribute types like Int2101010 don't scale by their component size.
constexpr angle::PackedEnumMap<VertexAttribType, size_t> kVertexAttribTypeSizeScale = {{
    {VertexAttribType::Byte, 1},
    {VertexAttribType::UnsignedByte, 1},
    {VertexAttribType::Short, 1},
    {VertexAttribType::UnsignedShort, 1},
    {VertexAttribType::Int, 1},
    {VertexAttribType::UnsignedInt, 1},
    {VertexAttribType::Float, 1},
    {VertexAttribType::HalfFloat, 1},
    {VertexAttribType::Fixed, 1},
}};
}  // namespace priv

ANGLE_INLINE size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib)
{
    return priv::kVertexAttribTypeComponentSize[attrib.type] *
           priv::kVertexAttribTypeSizeScale[attrib.type] * attrib.size;
}

// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
size_t ComputeVertexAttributeStride(const VertexAttribute &attrib, const VertexBinding &binding);

// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
GLintptr ComputeVertexAttributeOffset(const VertexAttribute &attrib, const VertexBinding &binding);

size_t ComputeVertexBindingElementCount(GLuint divisor, size_t drawCount, size_t instanceCount);

struct VertexAttribCurrentValueData
{
    union
    {
        GLfloat FloatValues[4];
        GLint IntValues[4];
        GLuint UnsignedIntValues[4];
    };
    VertexAttribType Type;

    VertexAttribCurrentValueData();

    void setFloatValues(const GLfloat floatValues[4]);
    void setIntValues(const GLint intValues[4]);
    void setUnsignedIntValues(const GLuint unsignedIntValues[4]);
};

bool operator==(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);
bool operator!=(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);

}  // namespace gl

#include "VertexAttribute.inl"

#endif  // LIBANGLE_VERTEXATTRIBUTE_H_
