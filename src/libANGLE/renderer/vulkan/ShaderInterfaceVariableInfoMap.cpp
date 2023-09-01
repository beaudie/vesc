//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderInterfaceVariableInfoMap: Maps shader interface variable SPIR-V ids to their Vulkan
// mapping.
//

#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"

namespace rx
{
namespace
{
uint32_t HashSPIRVId(uint32_t id)
{
    ASSERT(id >= sh::vk::spirv::kIdShaderVariablesBegin);
    return id - sh::vk::spirv::kIdShaderVariablesBegin;
}

void LoadShaderInterfaceVariableXfbInfo(gl::BinaryInputStream *stream,
                                        ShaderInterfaceVariableXfbInfo *xfb)
{
    stream->readStruct(&xfb->podStruct);
    xfb->arrayElements.resize(stream->readInt<size_t>());
    for (ShaderInterfaceVariableXfbInfo &arrayElement : xfb->arrayElements)
    {
        stream->readStruct(&arrayElement.podStruct);
    }
}

void SaveShaderInterfaceVariableXfbInfo(const ShaderInterfaceVariableXfbInfo &xfb,
                                        gl::BinaryOutputStream *stream)
{
    stream->writeStruct(xfb.podStruct);
    stream->writeInt(xfb.arrayElements.size());
    for (const ShaderInterfaceVariableXfbInfo &arrayElement : xfb.arrayElements)
    {
        stream->writeStruct(arrayElement.podStruct);
    }
}
}  // anonymous namespace

ShaderInterfaceVariableInfo::ShaderInterfaceVariableInfo() {}

// ShaderInterfaceVariableInfoMap implementation.
ShaderInterfaceVariableInfoMap::ShaderInterfaceVariableInfoMap() = default;

ShaderInterfaceVariableInfoMap::~ShaderInterfaceVariableInfoMap() = default;

void ShaderInterfaceVariableInfoMap::clear()
{
    mData.clear();
    mXFBData.clear();
    mXFBInfoCount = 0;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mIdToIndexMap[shaderType].clear();
    }
    std::fill(mInputPerVertexActiveMembers.begin(), mInputPerVertexActiveMembers.end(),
              gl::PerVertexMemberBitSet{});
    std::fill(mOutputPerVertexActiveMembers.begin(), mOutputPerVertexActiveMembers.end(),
              gl::PerVertexMemberBitSet{});
}

void ShaderInterfaceVariableInfoMap::save(gl::BinaryOutputStream *stream)
{
    for (const IdToIndexMap &idToIndexMap : mIdToIndexMap)
    {
        stream->writeInt(idToIndexMap.size());
        if (idToIndexMap.size() > 0)
        {
            stream->writeBytes(reinterpret_cast<const uint8_t *>(idToIndexMap.data()),
                               idToIndexMap.size() * sizeof(*idToIndexMap.data()));
        }
    }

    stream->writeVector(mData);

    ASSERT(mXFBData.size() <= mData.size());
    stream->writeInt(mXFBInfoCount);
    if (mXFBInfoCount > 0)
    {
        size_t xfbInfoCount = 0;
        for (size_t xfbIndex = 0; xfbIndex < mXFBData.size(); xfbIndex++)
        {
            if (!mXFBData[xfbIndex])
            {
                continue;
            }
            stream->writeInt(xfbIndex);
            xfbInfoCount++;
            XFBInterfaceVariableInfo &info = *mXFBData[xfbIndex];
            SaveShaderInterfaceVariableXfbInfo(info.xfb, stream);
            stream->writeInt(info.fieldXfb.size());
            for (const ShaderInterfaceVariableXfbInfo &xfb : info.fieldXfb)
            {
                SaveShaderInterfaceVariableXfbInfo(xfb, stream);
            }
        }
        ASSERT(xfbInfoCount == mXFBInfoCount);
    }

    // Store gl_PerVertex members only for stages that have it.
    stream->writeInt(mOutputPerVertexActiveMembers[gl::ShaderType::Vertex].bits());
    stream->writeInt(mInputPerVertexActiveMembers[gl::ShaderType::TessControl].bits());
    stream->writeInt(mOutputPerVertexActiveMembers[gl::ShaderType::TessControl].bits());
    stream->writeInt(mInputPerVertexActiveMembers[gl::ShaderType::TessEvaluation].bits());
    stream->writeInt(mOutputPerVertexActiveMembers[gl::ShaderType::TessEvaluation].bits());
    stream->writeInt(mInputPerVertexActiveMembers[gl::ShaderType::Geometry].bits());
    stream->writeInt(mOutputPerVertexActiveMembers[gl::ShaderType::Geometry].bits());
}
void ShaderInterfaceVariableInfoMap::load(gl::BinaryInputStream *stream)
{
    gl::ShaderMap<gl::PerVertexMemberBitSet> inputPerVertexActiveMembers;
    gl::ShaderMap<gl::PerVertexMemberBitSet> outputPerVertexActiveMembers;

    for (IdToIndexMap &idToIndexMap : mIdToIndexMap)
    {
        // ASSERT(idToIndexMap.empty());
        size_t count = stream->readInt<size_t>();
        if (count > 0)
        {
            idToIndexMap.resizeAndCopyData(stream->getBytes(count * sizeof(*idToIndexMap.data())),
                                           count);
        }
    }

    stream->readVector(&mData);

    ASSERT(mXFBData.empty());
    mXFBInfoCount = stream->readInt<size_t>();
    if (mXFBInfoCount > 0)
    {
        mXFBData.resize(mData.size());
        for (size_t i = 0; i < mXFBInfoCount; ++i)
        {
            size_t xfbIndex    = stream->readInt<size_t>();
            mXFBData[xfbIndex] = std::make_unique<XFBInterfaceVariableInfo>();

            XFBInterfaceVariableInfo &info = *mXFBData[xfbIndex];
            LoadShaderInterfaceVariableXfbInfo(stream, &info.xfb);
            info.fieldXfb.resize(stream->readInt<size_t>());
            for (ShaderInterfaceVariableXfbInfo &xfb : info.fieldXfb)
            {
                LoadShaderInterfaceVariableXfbInfo(stream, &xfb);
            }
        }
    }

    outputPerVertexActiveMembers[gl::ShaderType::Vertex] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    inputPerVertexActiveMembers[gl::ShaderType::TessControl] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    outputPerVertexActiveMembers[gl::ShaderType::TessControl] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    inputPerVertexActiveMembers[gl::ShaderType::TessEvaluation] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    outputPerVertexActiveMembers[gl::ShaderType::TessEvaluation] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    inputPerVertexActiveMembers[gl::ShaderType::Geometry] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());
    outputPerVertexActiveMembers[gl::ShaderType::Geometry] =
        gl::PerVertexMemberBitSet(stream->readInt<uint8_t>());

    mInputPerVertexActiveMembers.swap(inputPerVertexActiveMembers);
    mOutputPerVertexActiveMembers.swap(outputPerVertexActiveMembers);
}

void ShaderInterfaceVariableInfoMap::setInputPerVertexActiveMembers(
    gl::ShaderType shaderType,
    gl::PerVertexMemberBitSet activeMembers)
{
    // Input gl_PerVertex is only meaningful for tessellation and geometry stages
    ASSERT(shaderType == gl::ShaderType::TessControl ||
           shaderType == gl::ShaderType::TessEvaluation || shaderType == gl::ShaderType::Geometry ||
           activeMembers.none());
    mInputPerVertexActiveMembers[shaderType] = activeMembers;
}

void ShaderInterfaceVariableInfoMap::setOutputPerVertexActiveMembers(
    gl::ShaderType shaderType,
    gl::PerVertexMemberBitSet activeMembers)
{
    // Output gl_PerVertex is only meaningful for vertex, tessellation and geometry stages
    ASSERT(shaderType == gl::ShaderType::Vertex || shaderType == gl::ShaderType::TessControl ||
           shaderType == gl::ShaderType::TessEvaluation || shaderType == gl::ShaderType::Geometry ||
           activeMembers.none());
    mOutputPerVertexActiveMembers[shaderType] = activeMembers;
}

void ShaderInterfaceVariableInfoMap::setVariableIndex(gl::ShaderType shaderType,
                                                      uint32_t id,
                                                      VariableIndex index)
{
    mIdToIndexMap[shaderType][HashSPIRVId(id)] = index;
}

const VariableIndex &ShaderInterfaceVariableInfoMap::getVariableIndex(gl::ShaderType shaderType,
                                                                      uint32_t id) const
{
    return mIdToIndexMap[shaderType].at(HashSPIRVId(id));
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getMutable(gl::ShaderType shaderType,
                                                                        uint32_t id)
{
    ASSERT(hasVariable(shaderType, id));
    uint32_t index = getVariableIndex(shaderType, id).index;
    return mData[index];
}

XFBVariableInfoPtr &ShaderInterfaceVariableInfoMap::getXFBMutable(gl::ShaderType shaderType,
                                                                  uint32_t id)
{
    ASSERT(hasVariable(shaderType, id));
    uint32_t index = getVariableIndex(shaderType, id).index;
    if (index >= mXFBData.size())
    {
        mXFBData.resize(index + 1);
    }
    if (!mXFBData[index])
    {
        mXFBData[index]                   = std::make_unique<XFBInterfaceVariableInfo>();
        mData[index].hasTransformFeedback = true;
        mXFBInfoCount++;
    }
    return mXFBData[index];
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::add(gl::ShaderType shaderType,
                                                                 uint32_t id)
{
    ASSERT(!hasVariable(shaderType, id));
    uint32_t index = static_cast<uint32_t>(mData.size());
    setVariableIndex(shaderType, id, {index});
    mData.resize(index + 1);
    return mData[index];
}

void ShaderInterfaceVariableInfoMap::addResource(gl::ShaderBitSet shaderTypes,
                                                 const gl::ShaderMap<uint32_t> &idInShaderTypes,
                                                 uint32_t descriptorSet,
                                                 uint32_t binding)
{
    uint32_t index = static_cast<uint32_t>(mData.size());
    mData.resize(index + 1);
    ShaderInterfaceVariableInfo *info = &mData[index];

    info->descriptorSet = descriptorSet;
    info->binding       = binding;
    info->activeStages  = shaderTypes;

    for (const gl::ShaderType shaderType : shaderTypes)
    {
        const uint32_t id = idInShaderTypes[shaderType];
        ASSERT(!hasVariable(shaderType, id));
        setVariableIndex(shaderType, id, {index});
    }
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::addOrGet(gl::ShaderType shaderType,
                                                                      uint32_t id)
{
    if (!hasVariable(shaderType, id))
    {
        return add(shaderType, id);
    }
    else
    {
        uint32_t index = getVariableIndex(shaderType, id).index;
        return mData[index];
    }
}

bool ShaderInterfaceVariableInfoMap::hasVariable(gl::ShaderType shaderType, uint32_t id) const
{
    const uint32_t hashedId = HashSPIRVId(id);
    return hashedId < mIdToIndexMap[shaderType].size() &&
           mIdToIndexMap[shaderType].at(hashedId).index != VariableIndex::kInvalid;
}

bool ShaderInterfaceVariableInfoMap::hasTransformFeedbackInfo(gl::ShaderType shaderType,
                                                              uint32_t bufferIndex) const
{
    return hasVariable(shaderType, SpvGetXfbBufferBlockId(bufferIndex));
}
}  // namespace rx
