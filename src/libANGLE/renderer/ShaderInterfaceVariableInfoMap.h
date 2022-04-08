//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Header for the shared ShaderInterfaceVariableInfoMap class, used by both the
// Direct-to-Metal and Metal-SPIRV backends

#ifndef LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_
#define LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_

#include <functional>

#include <stdio.h>
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/renderer_utils.h"
namespace rx
{

enum class IndexedShaderVariableType
{
    Texture,
    Image,
    UniformBuffer,
    ShaderStorageBuffer,
};

// TODO: http://anglebug.com/4524: Need a different hash key than a string, since that's slow to
// calculate.
class ShaderInterfaceVariableInfoMap final : angle::NonCopyable
{
  public:
    ShaderInterfaceVariableInfoMap();
    ~ShaderInterfaceVariableInfoMap();

    void clear();
    bool contains(gl::ShaderType shaderType, const std::string &variableName) const;
    const ShaderInterfaceVariableInfo &get(gl::ShaderType shaderType,
                                           const std::string &variableName) const;
    ShaderInterfaceVariableInfo &add(gl::ShaderType shaderType, const std::string &variableName);
    void markAsDuplicate(gl::ShaderType shaderType, const std::string &variableName);
    ShaderInterfaceVariableInfo &addOrGet(gl::ShaderType shaderType,
                                          const std::string &variableName);
    size_t variableCount(gl::ShaderType shaderType) const { return mData[shaderType].size(); }

    ShaderInterfaceVariableInfo &getVaryingMutable(gl::ShaderType frontShaderType,
                                                   const std::string &varyingName);

    const ShaderInterfaceVariableInfo &getDefaultUniformInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getIndexedVariableInfo(
        const gl::ProgramExecutable &executable,
        gl::ShaderType shaderType,
        IndexedShaderVariableType variableType,
        uint32_t variableIndex) const;
    bool hasAtomicCounterInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getAtomicCounterInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getFramebufferFetchInfo(
        const gl::ProgramExecutable &executable,
        gl::ShaderType shaderType) const;

    using VariableNameToInfoMap = angle::HashMap<std::string, ShaderInterfaceVariableInfo>;

    class Iterator final
    {
      public:
        Iterator(VariableNameToInfoMap::const_iterator beginIt,
                 VariableNameToInfoMap::const_iterator endIt)
            : mBeginIt(beginIt), mEndIt(endIt)
        {}
        VariableNameToInfoMap::const_iterator begin() { return mBeginIt; }
        VariableNameToInfoMap::const_iterator end() { return mEndIt; }

      private:
        VariableNameToInfoMap::const_iterator mBeginIt;
        VariableNameToInfoMap::const_iterator mEndIt;
    };

    Iterator getIterator(gl::ShaderType shaderType) const;

  private:
    gl::ShaderMap<VariableNameToInfoMap> mData;
};

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getDefaultUniformInfo(gl::ShaderType shaderType) const
{
    const char *uniformName = kDefaultUniformNames[shaderType];
    return get(shaderType, uniformName);
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getIndexedVariableInfo(const gl::ProgramExecutable &executable,
                                                       gl::ShaderType shaderType,
                                                       IndexedShaderVariableType variableType,
                                                       uint32_t variableIndex) const
{
    switch (variableType)
    {
        case IndexedShaderVariableType::Image:
        {
            const std::vector<gl::LinkedUniform> &uniforms = executable.getUniforms();
            uint32_t uniformIndex = executable.getUniformIndexFromImageIndex(variableIndex);
            const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];
            const std::string samplerName         = GlslangGetMappedSamplerName(imageUniform.name);
            return get(shaderType, samplerName);
        }
        case IndexedShaderVariableType::ShaderStorageBuffer:
        {
            const std::vector<gl::InterfaceBlock> &blocks = executable.getShaderStorageBlocks();
            const gl::InterfaceBlock &block               = blocks[variableIndex];
            const std::string blockName                   = block.mappedName;
            return get(shaderType, blockName);
        }
        case IndexedShaderVariableType::Texture:
        {
            const std::vector<gl::LinkedUniform> &uniforms = executable.getUniforms();
            uint32_t uniformIndex = executable.getUniformIndexFromSamplerIndex(variableIndex);
            const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];
            const std::string samplerName = GlslangGetMappedSamplerName(samplerUniform.name);
            return get(shaderType, samplerName);
        }
        case IndexedShaderVariableType::UniformBuffer:
        {
            const std::vector<gl::InterfaceBlock> &blocks = executable.getUniformBlocks();
            const gl::InterfaceBlock &block               = blocks[variableIndex];
            const std::string blockName                   = block.mappedName;
            return get(shaderType, blockName);
        }
    }

    UNREACHABLE();
    return mData[shaderType].begin()->second;
}

ANGLE_INLINE bool ShaderInterfaceVariableInfoMap::hasAtomicCounterInfo(
    gl::ShaderType shaderType) const
{
    std::string blockName(sh::vk::kAtomicCountersBlockName);
    return contains(shaderType, blockName);
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getAtomicCounterInfo(gl::ShaderType shaderType) const
{
    std::string blockName(sh::vk::kAtomicCountersBlockName);
    return get(shaderType, blockName);
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getFramebufferFetchInfo(const gl::ProgramExecutable &executable,
                                                        gl::ShaderType shaderType) const
{
    const std::vector<gl::LinkedUniform> &uniforms = executable.getUniforms();
    const uint32_t baseUniformIndex                = executable.getFragmentInoutRange().low();
    const gl::LinkedUniform &baseInputAttachment   = uniforms.at(baseUniformIndex);
    std::string baseMappedName                     = baseInputAttachment.mappedName;
    return get(shaderType, baseMappedName);
}
}  // namespace rx
#endif  // LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_
