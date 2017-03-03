//
// Copyright (c) 2010-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_PROGRAMRESOURCE_H_
#define LIBANGLE_PROGRAMRESOURCE_H_

#include <string>
#include <vector>

#include "angle_gl.h"
#include "common/debug.h"
#include "common/MemoryBuffer.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/angletypes.h"

namespace gl
{

// Struct identifying whether the active resource is referenced by shaders respectively
struct ShaderRefs
{
    ShaderRefs();
    virtual ~ShaderRefs();

    ShaderRefs(const ShaderRefs &other);
    ShaderRefs &operator=(const ShaderRefs &other);
    bool operator==(const ShaderRefs &other) const;
    bool operator!=(const ShaderRefs &other) const { return !operator==(other); }

    int referencedByVertexShader;
    int referencedByFragmentShader;
    int referencedByComputeShader;
};

// Block info of a interface block field
struct LinkedField : public ShaderRefs
{
    LinkedField();
    virtual ~LinkedField();

    LinkedField(const int index, const sh::BlockMemberInfo &info);
    LinkedField(const LinkedField &other);
    LinkedField &operator=(const LinkedField &other);

    int blockIndex;
    sh::BlockMemberInfo blockInfo;
};

// Helper struct representing a single shader uniform
struct LinkedUniform : public LinkedField, sh::Uniform
{
    LinkedUniform();
    LinkedUniform(GLenum type,
                  GLenum precision,
                  const std::string &name,
                  unsigned int arraySize,
                  const int blockIndex,
                  const sh::BlockMemberInfo &blockInfo);
    LinkedUniform(const sh::Uniform &uniform);
    LinkedUniform(const LinkedUniform &uniform);
    LinkedUniform &operator=(const LinkedUniform &uniform);
    ~LinkedUniform();

    size_t dataSize() const;
    uint8_t *data();
    const uint8_t *data() const;
    bool isSampler() const;
    bool isImage() const;
    bool isInDefaultBlock() const;
    bool isField() const;
    size_t getElementSize() const;
    size_t getElementComponents() const;
    uint8_t *getDataPtrToElement(size_t elementIndex);
    const uint8_t *getDataPtrToElement(size_t elementIndex) const;

  private:
    mutable rx::MemoryBuffer mLazyData;
};

struct LinkedBufferVariable : public LinkedField, sh::InterfaceBlockField
{
    LinkedBufferVariable();
    ~LinkedBufferVariable();

    int topLevelArraySize;
    int topLevelArrayStride;
};

// Common buffer info for atomic counter, uniform block, and shader storage block buffer.
struct BufferBacked : ShaderRefs
{
    BufferBacked();
    ~BufferBacked();
    BufferBacked(const BufferBacked &other);
    BufferBacked &operator=(const BufferBacked &other);

    int bufferBinding;
    int numActiveVariables() const { return memberIndexes.size(); }
    unsigned int dataSize;
    std::vector<unsigned int> memberIndexes;
};

// Helper struct representing a single shader uniform or shader storage block
struct LinkedBlock : BufferBacked
{
    LinkedBlock();
    LinkedBlock(const std::string &nameIn, bool isArrayIn, unsigned int arrayElementIn);
    LinkedBlock(const LinkedBlock &other) = default;
    LinkedBlock &operator=(const LinkedBlock &other) = default;

    std::string nameWithArrayIndex() const;

    std::string name;
    bool isArray;
    unsigned int arrayElement;
};
}

#endif  // LIBANGLE_PROGRAMRESOURCE_H_
