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

struct ShaderRef
{
    ShaderRef();
    virtual ~ShaderRef();

    void setRef(GLenum shaderType);

    bool vertexStaticUse;
    bool fragmentStaticUse;
    bool computeStaticUse;
};

// Helper struct representing a single shader uniform
struct LinkedUniform : public sh::Uniform, public ShaderRef
{
    LinkedUniform();
    LinkedUniform(GLenum type,
                  GLenum precision,
                  const std::string &name,
                  unsigned int arraySize,
                  const int binding,
                  const int offset,
                  const int location,
                  const int bufferIndex,
                  const sh::BlockMemberInfo &blockInfo);
    LinkedUniform(const sh::Uniform &uniform);
    LinkedUniform(const LinkedUniform &uniform);
    LinkedUniform &operator=(const LinkedUniform &uniform);
    ~LinkedUniform();

    size_t dataSize() const;
    uint8_t *data()
    {
        if (mLazyData.empty())
        {
            // dataSize() will init the data store.
            size_t size = dataSize();
            memset(mLazyData.data(), 0, size);
        }

        return mLazyData.data();
    }
    const uint8_t *data() const;
    bool isSampler() const;
    bool isImage() const;
    bool isAtomicCounter() const;
    bool isInDefaultBlock() const;
    bool isField() const;
    size_t getElementSize() const;
    size_t getElementComponents() const;
    uint8_t *getDataPtrToElement(size_t elementIndex);
    const uint8_t *getDataPtrToElement(size_t elementIndex) const;

    // Identifies the containing buffer backed resource -- interface block or atomic counter buffer.
    int bufferIndex;
    sh::BlockMemberInfo blockInfo;

  private:
    mutable angle::MemoryBuffer mLazyData;
};

// Parent struct for atomic counter, uniform block, and shader storage block buffer, which all
// contain a group of shader variables, and have a GL buffer backed.
struct ShaderVariableBuffer : public ShaderRef
{
    ShaderVariableBuffer();
    virtual ~ShaderVariableBuffer();
    int numActiveVariables() const { return static_cast<int>(memberIndexes.size()); }

    int binding;
    unsigned int dataSize;
    std::vector<unsigned int> memberIndexes;
};

using AtomicCounterBuffer = ShaderVariableBuffer;

// Helper struct representing a single shader uniform block
struct UniformBlock : public ShaderVariableBuffer
{
    UniformBlock();
    UniformBlock(const std::string &nameIn,
                 bool isArrayIn,
                 unsigned int arrayElementIn,
                 int bindingIn);

    std::string nameWithArrayIndex() const;

    std::string name;
    bool isArray;
    unsigned int arrayElement;
};

}

#endif   // LIBANGLE_UNIFORM_H_
