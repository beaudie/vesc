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
#include "libANGLE/renderer/Format.h"

namespace gl
{
class VertexArray;

struct VertexBindingState
{
    // make sure default values are invalid
    VertexBindingState() : stride(16u), divisor(0), offset(0) {}

    GLuint getStride() const { return stride; }
    GLuint getDivisor() const { return divisor; }
    GLintptr getOffset() const { return offset; }
    GLuint stride;
    GLuint divisor;
    GLintptr offset;

    bool operator==(const VertexBindingState &other) const
    {
        return stride == other.stride && divisor == other.divisor && offset == other.offset;
    }
};

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

    GLuint getStride() const { return mState.stride; }
    void setStride(GLuint strideIn) { mState.stride = strideIn; }

    GLuint getDivisor() const { return mState.divisor; }
    void setDivisor(GLuint divisorIn) { mState.divisor = divisorIn; }

    GLintptr getOffset() const { return mState.offset; }
    void setOffset(GLintptr offsetIn) { mState.offset = offsetIn; }

    inline const VertexBindingState &getInnerState() const { return mState; }

    const BindingPointer<Buffer> &getBuffer() const { return mBuffer; }

    ANGLE_INLINE void setBuffer(const gl::Context *context, Buffer *bufferIn)
    {
        mBuffer.set(context, bufferIn);
    }

    // Skips ref counting for better inlined performance.
    ANGLE_INLINE void assignBuffer(Buffer *bufferIn) { mBuffer.assign(bufferIn); }

    void onContainerBindingChanged(const Context *context, int incr) const;

    const AttributesMask &getBoundAttributesMask() const { return mBoundAttributesMask; }

    void setBoundAttribute(size_t index) { mBoundAttributesMask.set(index); }

    void resetBoundAttribute(size_t index) { mBoundAttributesMask.reset(index); }

  private:
    VertexBindingState mState;

    BindingPointer<Buffer> mBuffer;

    // Mapping from this binding to all of the attributes that are using this binding.
    AttributesMask mBoundAttributesMask;
};

struct VertexAttributeState
{
    // make sure default values are invalid
    VertexAttributeState()
        : enabled(false),
          format(&angle::Format::Get(angle::FormatID::R32G32B32A32_FLOAT)),
          pointer(nullptr),
          relativeOffset(0),
          arrayStride(0)
    {}

    bool enabled;
    // pointer to some static data
    const angle::Format *format;
    // user provided pointer
    const void *pointer;
    GLuint relativeOffset;
    GLuint arrayStride;

    bool operator==(const VertexAttributeState &other) const
    {
        return enabled == other.enabled && format == other.format && pointer == other.pointer &&
               relativeOffset == other.relativeOffset && arrayStride == other.arrayStride;
    }
};

//
// Implementation of Generic Vertex Attributes for ES3.1
//
class VertexAttribute final : private angle::NonCopyable
{
  public:
    explicit VertexAttribute(GLuint bindingIndex);
    VertexAttribute(VertexAttribute &&attrib);
    VertexAttribute &operator=(VertexAttribute &&attrib);

    // Called from VertexArray.
    void updateCachedElementLimit(const VertexBinding &binding);
    GLint64 getCachedElementLimit() const { return mCachedElementLimit; }

    inline bool isEnabled() const { return mState.enabled; }
    inline void updateEnabled(bool enabled) { mState.enabled = enabled; }

    inline const angle::Format *getFormat() const { return mState.format; }
    inline void setFormat(const angle::Format *format) { mState.format = format; }

    inline const void *getPointer() const { return mState.pointer; }
    inline void setPointer(const void *pointer) { mState.pointer = pointer; }

    inline GLuint getRelativeOffset() const { return mState.relativeOffset; }
    inline void setRelativeOffset(GLuint relativeOffset) { mState.relativeOffset = relativeOffset; }

    inline GLuint getArrayStride() const { return mState.arrayStride; }
    inline void setArrayStride(GLuint arrayStride) { mState.arrayStride = arrayStride; }

    inline const VertexAttributeState &getInnerState() const { return mState; }

    GLuint bindingIndex;

    // Special value for the cached element limit on the integer overflow case.
    static constexpr GLint64 kIntegerOverflow = std::numeric_limits<GLint64>::min();

  private:
    VertexAttributeState mState;

    // This is kept in sync by the VertexArray. It is used to optimize draw call validation.
    GLint64 mCachedElementLimit;
};

ANGLE_INLINE size_t ComputeVertexAttributeTypeSize(const VertexAttribute &attrib)
{
    ASSERT(attrib.getFormat());
    return attrib.getFormat()->pixelBytes;
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
    } Values;
    VertexAttribType Type;

    VertexAttribCurrentValueData();

    void setFloatValues(const GLfloat floatValues[4]);
    void setIntValues(const GLint intValues[4]);
    void setUnsignedIntValues(const GLuint unsignedIntValues[4]);
};

bool operator==(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);
bool operator!=(const VertexAttribCurrentValueData &a, const VertexAttribCurrentValueData &b);

}  // namespace gl

#include "VertexAttribute.inc"

#endif  // LIBANGLE_VERTEXATTRIBUTE_H_
