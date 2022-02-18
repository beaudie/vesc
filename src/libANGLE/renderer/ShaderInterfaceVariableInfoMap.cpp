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
    for (VariableNameToInfoMap &shaderMap : mData)
    {
        shaderMap.clear();
    }
}

bool ShaderInterfaceVariableInfoMap::contains(gl::ShaderType shaderType,
                                              const std::string &variableName) const
{
    return mData[shaderType].find(variableName) != mData[shaderType].end();
}

const ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::get(
    gl::ShaderType shaderType,
    const std::string &variableName) const
{
    auto it = mData[shaderType].find(variableName);
    ASSERT(it != mData[shaderType].end());
    return it->second;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::get(gl::ShaderType shaderType,
                                                                 const std::string &variableName)
{
    auto it = mData[shaderType].find(variableName);
    ASSERT(it != mData[shaderType].end());
    return it->second;
}

void ShaderInterfaceVariableInfoMap::markAsDuplicate(gl::ShaderType shaderType,
                                                     const std::string &variableName)
{
    ASSERT(contains(shaderType, variableName));
    mData[shaderType][variableName].isDuplicate = true;
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::add(gl::ShaderType shaderType,
                                                                 const std::string &variableName)
{
    ASSERT(!contains(shaderType, variableName));
    return mData[shaderType][variableName];
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::addOrGet(
    gl::ShaderType shaderType,
    const std::string &variableName)
{
    return mData[shaderType][variableName];
}

ShaderInterfaceVariableInfoMap::Iterator ShaderInterfaceVariableInfoMap::getIterator(
    gl::ShaderType shaderType) const
{
    return Iterator(mData[shaderType].begin(), mData[shaderType].end());
}
uint32_t ShaderInterfaceVariableInfoMap::getActiveShaderCount() const
{
    uint32_t count = 0;
    for (const VariableNameToInfoMap &infoMap : mData)
    {
        count += infoMap.empty() ? 0 : 1;
    }
    return count;
}

uint32_t ShaderInterfaceVariableInfoMap::getXfbBufferBinding(uint32_t xfbBufferIndex) const
{
    // The XFB descriptors are after the uniform data in each shader.
    uint32_t uniformBufferCount = getActiveShaderCount();
    return uniformBufferCount + xfbBufferIndex;
}
}  // namespace rx
