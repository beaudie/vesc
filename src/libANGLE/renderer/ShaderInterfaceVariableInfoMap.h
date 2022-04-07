//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Header for the shared ShaderInterfaceVariableInfoMap class, used by both the
// Direct-to-Metal and Metal-SPIRV backends

#ifndef LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_
#define LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_

#include "common/FastVector.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

#include <functional>

#include <stdio.h>

namespace rx
{

enum class ShaderVariableType
{
    Attribute,
    AtomicCounter,
    FramebufferFetch,
    DefaultUniform,
    Varying,
    Texture,
    Image,
    UniformBuffer,
    ShaderStorageBuffer,
    TransformFeedback,
    Output,
    SecondaryOutput,
    EnumCount,
};

struct TypeAndIndex
{
    ShaderVariableType variableType;
    uint32_t index;
};

class ShaderInterfaceVariableInfoMap final : angle::NonCopyable
{
  public:
    ShaderInterfaceVariableInfoMap();
    ~ShaderInterfaceVariableInfoMap();

    void clear();
    ShaderInterfaceVariableInfo &add(gl::ShaderType shaderType,
                                     ShaderVariableType variableType,
                                     const std::string &variableName);
    void markAsDuplicate(gl::ShaderType shaderType,
                         ShaderVariableType variableType,
                         const std::string &variableName);
    ShaderInterfaceVariableInfo &addOrGet(gl::ShaderType shaderType,
                                          ShaderVariableType variableType,
                                          const std::string &variableName);
    size_t variableCount(gl::ShaderType shaderType) const
    {
        return mNameToTypeAndIndexMap[shaderType].size();
    }

    void setActiveStages(gl::ShaderType shaderType,
                         ShaderVariableType variableType,
                         const std::string &variableName,
                         gl::ShaderBitSet activeStages);
    ShaderInterfaceVariableInfo &getMutable(gl::ShaderType shaderType,
                                            ShaderVariableType variableType,
                                            const std::string &variableName);

    const ShaderInterfaceVariableInfo &getDefaultUniformInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getIndexedVariableInfo(gl::ShaderType shaderType,
                                                              ShaderVariableType variableType,
                                                              uint32_t variableIndex) const;
    bool hasAtomicCounterInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getAtomicCounterInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getFramebufferFetchInfo(gl::ShaderType shaderType) const;
    bool hasTransformFeedbackInfo(gl::ShaderType shaderType, uint32_t bufferIndex) const;
    const ShaderInterfaceVariableInfo &getTransformFeedbackInfo(gl::ShaderType shaderType,
                                                                uint32_t bufferIndex) const;

    uint32_t getDefaultUniformBinding(gl::ShaderType shaderType) const;
    uint32_t getXfbBufferBinding(uint32_t xfbBufferIndex) const;
    uint32_t getAtomicCounterBufferBinding(gl::ShaderType shaderType,
                                           uint32_t atomicCounterBufferIndex) const;

    using VariableNameToInfoArray = std::vector<ShaderInterfaceVariableInfo>;
    using VariableTypeToInfoMap = angle::PackedEnumMap<ShaderVariableType, VariableNameToInfoArray>;
    using NameToTypeAndIndexMap = angle::HashMap<std::string, TypeAndIndex>;

    static constexpr size_t kResourceFastMapMax = 32;
    using ResourceIndexMap                      = angle::FastMap<uint32_t, kResourceFastMapMax>;
    using VariableTypeToIndexMap = angle::PackedEnumMap<ShaderVariableType, ResourceIndexMap>;

    class Iterator final
    {
      public:
        Iterator(NameToTypeAndIndexMap::const_iterator beginIt,
                 NameToTypeAndIndexMap::const_iterator endIt)
            : mBeginIt(beginIt), mEndIt(endIt)
        {}
        NameToTypeAndIndexMap::const_iterator begin() { return mBeginIt; }
        NameToTypeAndIndexMap::const_iterator end() { return mEndIt; }

      private:
        NameToTypeAndIndexMap::const_iterator mBeginIt;
        NameToTypeAndIndexMap::const_iterator mEndIt;
    };

    Iterator getIterator(gl::ShaderType shaderType) const;

    bool hasVariable(gl::ShaderType shaderType, const std::string &variableName) const;
    const ShaderInterfaceVariableInfo &getVariableByName(gl::ShaderType shaderType,
                                                         const std::string &variableName) const;
    void mapIndexedResourceByName(gl::ShaderType shaderType,
                                  ShaderVariableType variableType,
                                  uint32_t resourceIndex,
                                  const std::string &variableName);
    void mapIndexedResource(gl::ShaderType shaderType,
                            ShaderVariableType variableType,
                            uint32_t resourceIndex,
                            uint32_t variableIndex);

    const gl::ShaderMap<VariableTypeToIndexMap> &getIndexedResourceMap() const;

  private:
    gl::ShaderMap<VariableTypeToInfoMap> mData;
    gl::ShaderMap<NameToTypeAndIndexMap> mNameToTypeAndIndexMap;
    gl::ShaderMap<VariableTypeToIndexMap> mIndexedResourceIndexMap;
};

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getDefaultUniformInfo(gl::ShaderType shaderType) const
{
    return mData[shaderType][ShaderVariableType::DefaultUniform][0];
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getIndexedVariableInfo(gl::ShaderType shaderType,
                                                       ShaderVariableType variableType,
                                                       uint32_t resourceIndex) const
{
    ASSERT(resourceIndex < mIndexedResourceIndexMap[shaderType][variableType].size());
    uint32_t variableIndex = mIndexedResourceIndexMap[shaderType][variableType][resourceIndex];
    ASSERT(variableIndex < mData[shaderType][variableType].size());
    return mData[shaderType][variableType][variableIndex];
}

ANGLE_INLINE bool ShaderInterfaceVariableInfoMap::hasAtomicCounterInfo(
    gl::ShaderType shaderType) const
{
    return !mData[shaderType][ShaderVariableType::AtomicCounter].empty();
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getAtomicCounterInfo(gl::ShaderType shaderType) const
{
    return mData[shaderType][ShaderVariableType::AtomicCounter][0];
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getFramebufferFetchInfo(gl::ShaderType shaderType) const
{
    return mData[shaderType][ShaderVariableType::FramebufferFetch][0];
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getTransformFeedbackInfo(gl::ShaderType shaderType,
                                                         uint32_t bufferIndex) const
{
    return mData[shaderType][ShaderVariableType::TransformFeedback][bufferIndex];
}
}  // namespace rx
#endif  // LIBANGLE_RENDERER_SHADERINTERFACEVARIABLEINFOMAP_H_
