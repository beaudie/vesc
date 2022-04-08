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
    for (VariableTypeToInfoMap &shaderMap : mData)
    {
        for (VariableNameToInfoMap &typeMap : shaderMap)
        {
            typeMap.clear();
        }
    }
}

bool ShaderInterfaceVariableInfoMap::contains(gl::ShaderType shaderType,
                                              ShaderVariableType variableType,
                                              const std::string &variableName) const
{
    return mData[shaderType][variableType].find(variableName) !=
           mData[shaderType][variableType].end();
}

const ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::get(
    gl::ShaderType shaderType,
    ShaderVariableType variableType,
    const std::string &variableName) const
{
    auto it = mData[shaderType][variableType].find(variableName);
    ASSERT(it != mData[shaderType][variableType].end());
    return it->second;
}

void ShaderInterfaceVariableInfoMap::setActiveStages(gl::ShaderType shaderType,
                                                     ShaderVariableType variableType,
                                                     const std::string &variableName,
                                                     gl::ShaderBitSet activeStages)
{
    auto it = mData[shaderType][variableType].find(variableName);
    ASSERT(it != mData[shaderType][variableType].end());
    it->second.activeStages = activeStages;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getMutable(
    gl::ShaderType shaderType,
    ShaderVariableType variableType,
    const std::string &variableName)
{
    auto it = mData[shaderType][variableType].find(variableName);
    ASSERT(it != mData[shaderType][variableType].end());
    return it->second;
}

void ShaderInterfaceVariableInfoMap::markAsDuplicate(gl::ShaderType shaderType,
                                                     ShaderVariableType variableType,
                                                     const std::string &variableName)
{
    ASSERT(contains(shaderType, variableType, variableName));
    mData[shaderType][variableType][variableName].isDuplicate = true;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::add(gl::ShaderType shaderType,
                                                                 ShaderVariableType variableType,
                                                                 const std::string &variableName)
{
    ASSERT(!contains(shaderType, variableType, variableName));
    mNameToTypeMap[shaderType][variableName] = variableType;
    return mData[shaderType][variableType][variableName];
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::addOrGet(
    gl::ShaderType shaderType,
    ShaderVariableType variableType,
    const std::string &variableName)
{
    mNameToTypeMap[shaderType][variableName] = variableType;
    return mData[shaderType][variableType][variableName];
}

ShaderInterfaceVariableInfoMap::Iterator ShaderInterfaceVariableInfoMap::getIterator(
    gl::ShaderType shaderType,
    ShaderVariableType variableType) const
{
    return Iterator(mData[shaderType][variableType].begin(), mData[shaderType][variableType].end());
}

}  // namespace rx
