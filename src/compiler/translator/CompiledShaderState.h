//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompiledShaderState.h:
//   Defines a struct containing any data that is needed to build
//   a ShaderState from a TCompiler.
//

#ifndef COMPILER_TRANSLATOR_COMPILEDSHADERSTATE_H_
#define COMPILER_TRANSLATOR_COMPILEDSHADERSTATE_H_

#include "compiler/translator/blocklayout.h"

#include "common/BinaryStream.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"

#include "GLSLANG/ShaderLang.h"
#include "GLSLANG/ShaderVars.h"

#include <string>

namespace sh
{

namespace
{
template <typename VarT>
std::vector<VarT> GetActiveShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    std::vector<VarT> result;
    for (size_t varIndex = 0; varIndex < variableList->size(); varIndex++)
    {
        const VarT &var = variableList->at(varIndex);
        if (var.active)
        {
            result.push_back(var);
        }
    }
    return result;
}

template <typename VarT>
const std::vector<VarT> &GetShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    return *variableList;
}
}  // namespace

// @todo this type is also defined in compiler/Compiler.h and libANGLE/renderer_utils.h. Move this
// to a single common definition?
using SpecConstUsageBits = angle::PackedEnumBitSet<sh::vk::SpecConstUsage, uint32_t>;

// Helper functions for serializing shader variables
void WriteBlockMemberInfo(gl::BinaryOutputStream *stream, const sh::BlockMemberInfo &var);
void LoadBlockMemberInfo(gl::BinaryInputStream *stream, sh::BlockMemberInfo *var);

void WriteShaderVar(gl::BinaryOutputStream *stream, const sh::ShaderVariable &var);
void LoadShaderVar(gl::BinaryInputStream *stream, sh::ShaderVariable *var);

void WriteShInterfaceBlock(gl::BinaryOutputStream *stream, const sh::InterfaceBlock &block);
void LoadShInterfaceBlock(gl::BinaryInputStream *stream, sh::InterfaceBlock *block);

bool CompareShaderVar(const sh::ShaderVariable &x, const sh::ShaderVariable &y);

struct CompiledShaderState
{
    CompiledShaderState(gl::ShaderType shaderType);
    ~CompiledShaderState();

    void serialize(gl::BinaryOutputStream &stream) const;
    void deserialize(gl::BinaryInputStream &stream);

    const gl::ShaderType shaderType;

    int shaderVersion;
    std::string translatedSource;
    sh::BinaryBlob compiledBinary;
    sh::WorkGroupSize localSize;

    std::vector<sh::ShaderVariable> inputVaryings;
    std::vector<sh::ShaderVariable> outputVaryings;
    std::vector<sh::ShaderVariable> uniforms;
    std::vector<sh::InterfaceBlock> uniformBlocks;
    std::vector<sh::InterfaceBlock> shaderStorageBlocks;
    std::vector<sh::ShaderVariable> allAttributes;
    std::vector<sh::ShaderVariable> activeAttributes;
    std::vector<sh::ShaderVariable> activeOutputVariables;

    bool hasDiscard;
    bool enablesPerSampleShading;
    gl::BlendEquationBitSet advancedBlendEquations;
    SpecConstUsageBits specConstUsageBits;

    // ANGLE_multiview.
    int numViews;

    // Geometry Shader
    Optional<gl::PrimitiveMode> geometryShaderInputPrimitiveType;
    Optional<gl::PrimitiveMode> geometryShaderOutputPrimitiveType;
    Optional<GLint> geometryShaderMaxVertices;
    int geometryShaderInvocations;

    // Tessellation Shader
    int tessControlShaderVertices;
    GLenum tessGenMode;
    GLenum tessGenSpacing;
    GLenum tessGenVertexOrder;
    GLenum tessGenPointMode;
};
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_COMPILEDSHADERSTATE_H_
