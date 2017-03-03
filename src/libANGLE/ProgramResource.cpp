//
// Copyright (c) 2010-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/ProgramResource.h"

#include "common/utilities.h"

#include <cstring>

namespace gl
{

ShaderRefs::ShaderRefs()
    : referencedByVertexShader(0), referencedByFragmentShader(0), referencedByComputeShader(0)
{
}

ShaderRefs::~ShaderRefs()
{
}

ShaderRefs::ShaderRefs(const ShaderRefs &other)
    : referencedByVertexShader(other.referencedByVertexShader),
      referencedByFragmentShader(other.referencedByFragmentShader),
      referencedByComputeShader(other.referencedByComputeShader)
{
}

ShaderRefs &ShaderRefs::operator=(const ShaderRefs &other)
{
    referencedByVertexShader   = other.referencedByVertexShader;
    referencedByFragmentShader = other.referencedByFragmentShader;
    referencedByComputeShader  = other.referencedByComputeShader;

    return *this;
}

bool ShaderRefs::operator==(const ShaderRefs &other) const
{
    return (referencedByVertexShader == other.referencedByVertexShader &&
            referencedByFragmentShader == other.referencedByFragmentShader &&
            referencedByComputeShader == other.referencedByComputeShader);
}

LinkedField::LinkedField() : blockIndex(-1), blockInfo(sh::BlockMemberInfo::getDefaultBlockInfo())
{
}

LinkedField::LinkedField(const int index, const sh::BlockMemberInfo &info)
{
    blockIndex = index;
    blockInfo  = info;
}

LinkedField::~LinkedField()
{
}

LinkedField::LinkedField(const LinkedField &other) : ShaderRefs(other)
{
    blockIndex = other.blockIndex;
    blockInfo  = other.blockInfo;
}

LinkedField &LinkedField::operator=(const LinkedField &other)
{
    ShaderRefs::operator=(other);
    return *this;
}

LinkedUniform::LinkedUniform()
{
}

LinkedUniform::LinkedUniform(GLenum typeIn,
                             GLenum precisionIn,
                             const std::string &nameIn,
                             unsigned int arraySizeIn,
                             const int blockIndexIn,
                             const sh::BlockMemberInfo &blockInfoIn)
    : LinkedField(blockIndexIn, blockInfoIn)
{
    type      = typeIn;
    precision = precisionIn;
    name      = nameIn;
    arraySize = arraySizeIn;
}

LinkedUniform::LinkedUniform(const sh::Uniform &uniform) : sh::Uniform(uniform)
{
}

LinkedUniform::LinkedUniform(const LinkedUniform &uniform)
    : LinkedField(uniform), sh::Uniform(uniform)
{
    // This function is not intended to be called during runtime.
    ASSERT(uniform.mLazyData.empty());
}

LinkedUniform &LinkedUniform::operator=(const LinkedUniform &uniform)
{
    // This function is not intended to be called during runtime.
    ASSERT(uniform.mLazyData.empty());

    sh::Uniform::operator=(uniform);
    LinkedField::operator=(uniform);

    return *this;
}

LinkedUniform::~LinkedUniform()
{
}

bool LinkedUniform::isInDefaultBlock() const
{
    return blockIndex == -1;
}

size_t LinkedUniform::dataSize() const
{
    ASSERT(type != GL_STRUCT_ANGLEX);
    if (mLazyData.empty())
    {
        mLazyData.resize(VariableExternalSize(type) * elementCount());
        ASSERT(!mLazyData.empty());
    }

    return mLazyData.size();
}

uint8_t *LinkedUniform::data()
{
    if (mLazyData.empty())
    {
        // dataSize() will init the data store.
        size_t size = dataSize();
        memset(mLazyData.data(), 0, size);
    }

    return mLazyData.data();
}

const uint8_t *LinkedUniform::data() const
{
    return const_cast<LinkedUniform *>(this)->data();
}

bool LinkedUniform::isSampler() const
{
    return IsSamplerType(type);
}

bool LinkedUniform::isImage() const
{
    return IsImageType(type);
}

bool LinkedUniform::isField() const
{
    return name.find('.') != std::string::npos;
}

size_t LinkedUniform::getElementSize() const
{
    return VariableExternalSize(type);
}

size_t LinkedUniform::getElementComponents() const
{
    return VariableComponentCount(type);
}

uint8_t *LinkedUniform::getDataPtrToElement(size_t elementIndex)
{
    ASSERT((!isArray() && elementIndex == 0) || (isArray() && elementIndex < arraySize));
    return data() + (elementIndex > 0 ? (getElementSize() * elementIndex) : 0u);
}

const uint8_t *LinkedUniform::getDataPtrToElement(size_t elementIndex) const
{
    return const_cast<LinkedUniform *>(this)->getDataPtrToElement(elementIndex);
}

LinkedBufferVariable::LinkedBufferVariable() : topLevelArraySize(1), topLevelArrayStride(0)
{
}

LinkedBufferVariable::~LinkedBufferVariable()
{
}

BufferBacked::BufferBacked() : bufferBinding(0), dataSize(0)
{
}

BufferBacked::~BufferBacked()
{
}

BufferBacked::BufferBacked(const BufferBacked &other) : ShaderRefs(other)
{
    bufferBinding = other.bufferBinding;
    dataSize      = other.dataSize;
    memberIndexes = other.memberIndexes;
}

BufferBacked &BufferBacked::operator=(const BufferBacked &other)
{
    ShaderRefs::operator=(other);
    bufferBinding       = other.bufferBinding;
    dataSize            = other.dataSize;
    memberIndexes       = other.memberIndexes;

    return *this;
}

LinkedBlock::LinkedBlock() : isArray(false), arrayElement(0)
{
}

LinkedBlock::LinkedBlock(const std::string &nameIn, bool isArrayIn, unsigned int arrayElementIn)
    : name(nameIn), isArray(isArrayIn), arrayElement(arrayElementIn)
{
}

std::string LinkedBlock::nameWithArrayIndex() const
{
    std::stringstream fullNameStr;
    fullNameStr << name;
    if (isArray)
    {
        fullNameStr << "[" << arrayElement << "]";
    }

    return fullNameStr.str();
}
}
