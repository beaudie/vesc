//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderInterfaceVariableInfoMap.cpp:
//    Implements helper class for shader compilers
//

#include "libANGLE/renderer/ShaderInterfaceVariableInfoMap.h"
namespace rx
{

ShaderInterfaceVariableInfo::ShaderInterfaceVariableInfo() {}

// ShaderInterfaceVariableInfoMap implementation.
ShaderInterfaceVariableInfoMap::ShaderInterfaceVariableInfoMap() = default;

ShaderInterfaceVariableInfoMap::~ShaderInterfaceVariableInfoMap() = default;

void ShaderInterfaceVariableInfoMap::clear()
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        for (ShaderVariableType variableType : angle::AllEnums<ShaderVariableType>())
        {
            mData[shaderType][variableType].clear();
            mIndexedResourceIndexMap[shaderType][variableType].clear();
        }
        mNameToTypeAndIndexMap[shaderType].clear();
    }
}

void ShaderInterfaceVariableInfoMap::setActiveStages(gl::ShaderType shaderType,
                                                     ShaderVariableType variableType,
                                                     const std::string &variableName,
                                                     gl::ShaderBitSet activeStages)
{
    ASSERT(hasVariable(shaderType, variableName));
    uint32_t index = mNameToTypeAndIndexMap[shaderType][variableName].index;
    mData[shaderType][variableType][index].activeStages = activeStages;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getMutable(
    gl::ShaderType shaderType,
    ShaderVariableType variableType,
    const std::string &variableName)
{
    ASSERT(hasVariable(shaderType, variableName));
    uint32_t index = mNameToTypeAndIndexMap[shaderType][variableName].index;
    return mData[shaderType][variableType][index];
}

void ShaderInterfaceVariableInfoMap::markAsDuplicate(gl::ShaderType shaderType,
                                                     ShaderVariableType variableType,
                                                     const std::string &variableName)
{
    ASSERT(hasVariable(shaderType, variableName));
    uint32_t index = mNameToTypeAndIndexMap[shaderType][variableName].index;
    mData[shaderType][variableType][index].isDuplicate = true;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::add(gl::ShaderType shaderType,
                                                                 ShaderVariableType variableType,
                                                                 const std::string &variableName)
{
    ASSERT(!hasVariable(shaderType, variableName));
    uint32_t index = static_cast<uint32_t>(mData[shaderType][variableType].size());
    mNameToTypeAndIndexMap[shaderType][variableName] = {variableType, index};
    mData[shaderType][variableType].resize(index + 1);
    return mData[shaderType][variableType][index];
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::addOrGet(
    gl::ShaderType shaderType,
    ShaderVariableType variableType,
    const std::string &variableName)
{
    if (!hasVariable(shaderType, variableName))
    {
        return add(shaderType, variableType, variableName);
    }
    else
    {
        uint32_t index = mNameToTypeAndIndexMap[shaderType][variableName].index;
        return mData[shaderType][variableType][index];
    }
}

ShaderInterfaceVariableInfoMap::Iterator ShaderInterfaceVariableInfoMap::getIterator(
    gl::ShaderType shaderType) const
{
    return Iterator(mNameToTypeAndIndexMap[shaderType].begin(),
                    mNameToTypeAndIndexMap[shaderType].end());
}

bool ShaderInterfaceVariableInfoMap::hasVariable(gl::ShaderType shaderType,
                                                 const std::string &variableName) const
{
    auto iter = mNameToTypeAndIndexMap[shaderType].find(variableName);
    return (iter != mNameToTypeAndIndexMap[shaderType].end());
}

const ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getVariableByName(
    gl::ShaderType shaderType,
    const std::string &variableName) const
{
    auto iter = mNameToTypeAndIndexMap[shaderType].find(variableName);
    ASSERT(iter != mNameToTypeAndIndexMap[shaderType].end());
    TypeAndIndex typeAndIndex = iter->second;
    return mData[shaderType][typeAndIndex.variableType][typeAndIndex.index];
}

bool ShaderInterfaceVariableInfoMap::hasTransformFeedbackInfo(gl::ShaderType shaderType,
                                                              uint32_t bufferIndex) const
{
    std::string bufferName = rx::GetXfbBufferName(bufferIndex);
    return hasVariable(shaderType, bufferName);
}

void ShaderInterfaceVariableInfoMap::mapIndexedResourceByName(gl::ShaderType shaderType,
                                                              ShaderVariableType variableType,
                                                              uint32_t resourceIndex,
                                                              const std::string &variableName)
{
    ASSERT(hasVariable(shaderType, variableName));
    const auto &iter                 = mNameToTypeAndIndexMap[shaderType].find(variableName);
    const TypeAndIndex &typeAndIndex = iter->second;
    ASSERT(typeAndIndex.variableType == variableType);
    mapIndexedResource(shaderType, variableType, resourceIndex, typeAndIndex.index);
}

void ShaderInterfaceVariableInfoMap::mapIndexedResource(gl::ShaderType shaderType,
                                                        ShaderVariableType variableType,
                                                        uint32_t resourceIndex,
                                                        uint32_t variableIndex)
{
    mIndexedResourceIndexMap[shaderType][variableType][resourceIndex] = variableIndex;
}

const gl::ShaderMap<ShaderInterfaceVariableInfoMap::VariableTypeToIndexMap>
    &ShaderInterfaceVariableInfoMap::getIndexedResourceMap() const
{
    return mIndexedResourceIndexMap;
}

uint32_t ShaderInterfaceVariableInfoMap::getDefaultUniformBinding(gl::ShaderType shaderType) const
{
    return getDefaultUniformInfo(shaderType).binding;
}

uint32_t ShaderInterfaceVariableInfoMap::getXfbBufferBinding(uint32_t xfbBufferIndex) const
{
    return getTransformFeedbackInfo(gl::ShaderType::Vertex, xfbBufferIndex).binding;
}

uint32_t ShaderInterfaceVariableInfoMap::getAtomicCounterBufferBinding(
    gl::ShaderType shaderType,
    uint32_t atomicCounterBufferIndex) const
{
    ASSERT(hasAtomicCounterInfo(shaderType));
    return getAtomicCounterInfo(shaderType).binding + atomicCounterBufferIndex;
}
}  // namespace rx
