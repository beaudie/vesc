//
// Copyright (c) 2010-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_UNIFORM_H_
#define LIBANGLE_UNIFORM_H_

#include <string>
#include <vector>

#include "angle_gl.h"
#include "common/debug.h"
#include "common/MemoryBuffer.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/angletypes.h"

namespace gl
{

// Helper struct representing a single shader uniform
struct LinkedUniform : public sh::Uniform
{
    LinkedUniform();
    LinkedUniform(GLenum type,
                  GLenum precision,
                  const std::string &name,
                  unsigned int arraySize,
                  const int binding,
                  const int location,
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

    int blockIndex;
    sh::BlockMemberInfo blockInfo;

  private:
    mutable angle::MemoryBuffer mLazyData;
};

// Helper struct representing a single buffer varable
struct BufferVariable
{
    BufferVariable(const std::string &name,
                   const int blockIndex,
                   const unsigned int arraySize,
                   const unsigned int topLevelArraySize,
                   const unsigned int topLevelArrayStride,
                   const sh::BlockMemberInfo &blockInfo);
    BufferVariable(const BufferVariable &bufferVariable);
    BufferVariable &operator=(const BufferVariable &bufferVariable);
    ~BufferVariable();

    std::string name;
    int blockIndex;
    unsigned int arraySize;
    unsigned int topLevelArraySize;
    unsigned int topLevelArrayStride;
    sh::BlockMemberInfo blockInfo;
};

// Helper struct representing a single shader interface block
struct InterfaceBlock
{
    InterfaceBlock();
    InterfaceBlock(const std::string &nameIn,
                   int bindingIn,
                   bool isArrayIn,
                   unsigned int arrayElementIn);
    InterfaceBlock(const InterfaceBlock &other) = default;
    InterfaceBlock &operator=(const InterfaceBlock &other) = default;

    std::string nameWithArrayIndex() const;

    std::string name;
    int binding;
    bool isArray;
    unsigned int arrayElement;
    unsigned int dataSize;

    bool vertexStaticUse;
    bool fragmentStaticUse;
    bool computeStaticUse;

    std::vector<unsigned int> memberIndexes;
};

}

#endif   // LIBANGLE_UNIFORM_H_
