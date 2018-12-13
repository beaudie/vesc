//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout.cpp:
//   Implementation for block layout classes and methods.
//

#include "compiler/translator/blocklayout.h"

#include "common/mathutil.h"
#include "common/utilities.h"

namespace sh
{

namespace
{
class BlockLayoutMapVisitor : public BlockEncoderVisitor
{
  public:
    BlockLayoutMapVisitor(BlockLayoutMap *blockInfoOut,
                          const std::string &instanceName,
                          BlockLayoutEncoder *encoder)
        : BlockEncoderVisitor(instanceName, instanceName, encoder), mInfoOut(blockInfoOut)
    {}

    void encodeVariable(const ShaderVariable &variable,
                        const BlockMemberInfo &variableInfo,
                        const std::string &name,
                        const std::string &mappedName) override
    {
        ASSERT(!gl::IsSamplerType(variable.type));
        (*mInfoOut)[name] = variableInfo;
    }

  private:
    BlockLayoutMap *mInfoOut;
};

template <typename VarT>
void GetInterfaceBlockInfo(const std::vector<VarT> &fields,
                           const std::string &prefix,
                           sh::BlockLayoutEncoder *encoder,
                           bool inRowMajorLayout,
                           BlockLayoutMap *blockInfoOut)
{
    BlockLayoutMapVisitor visitor(blockInfoOut, prefix, encoder);
    TraverseShaderVariables(fields.data(), fields.size(), inRowMajorLayout, &visitor);
}

void TraverseStructVariable(const ShaderVariable &variable,
                            bool isRowMajorLayout,
                            ShaderVariableVisitor *visitor)
{
    const std::vector<ShaderVariable> &fields = variable.fields;

    visitor->enterStructAccess(variable);
    TraverseShaderVariables(fields.data(), fields.size(), isRowMajorLayout, visitor);
    visitor->exitStructAccess(variable);
}

void TraverseStructArrayVariable(const ShaderVariable &variable,
                                 unsigned int arrayNestingIndex,
                                 bool inRowMajorLayout,
                                 ShaderVariableVisitor *visitor)
{
    visitor->enterArray(variable, arrayNestingIndex);

    // Nested arrays are processed starting from outermost (arrayNestingIndex 0u) and ending at the
    // innermost. We make a special case for unsized arrays.
    const unsigned int currentArraySize = variable.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < std::max(currentArraySize, 1u);
         ++arrayElement)
    {
        visitor->enterArrayElement(variable, arrayNestingIndex, arrayElement);

        if (arrayNestingIndex + 1u < variable.arraySizes.size())
        {
            TraverseStructArrayVariable(variable, arrayNestingIndex + 1u, inRowMajorLayout,
                                        visitor);
        }
        else
        {
            TraverseStructVariable(variable, inRowMajorLayout, visitor);
        }

        visitor->exitArrayElement(variable, arrayNestingIndex, arrayElement);
    }

    visitor->exitArray(variable, arrayNestingIndex);
}

void TraverseArrayOfArraysVariable(const ShaderVariable &variable,
                                   unsigned int arrayNestingIndex,
                                   bool isRowMajorMatrix,
                                   ShaderVariableVisitor *visitor)
{
    visitor->enterArray(variable, arrayNestingIndex);

    const unsigned int currentArraySize = variable.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < currentArraySize; ++arrayElement)
    {
        visitor->enterArrayElement(variable, arrayNestingIndex, arrayElement);

        if (arrayNestingIndex + 2u < variable.arraySizes.size())
        {
            TraverseArrayOfArraysVariable(variable, arrayNestingIndex + 1u, isRowMajorMatrix,
                                          visitor);
        }
        else if (gl::IsSamplerType(variable.type))
        {
            visitor->visitSampler(variable);
        }
        else
        {
            visitor->visitVariable(variable, arrayNestingIndex + 1u, isRowMajorMatrix);
        }

        visitor->exitArrayElement(variable, arrayNestingIndex, arrayElement);
    }

    visitor->exitArray(variable, arrayNestingIndex);
}

std::string CollapseNameStack(const std::vector<std::string> &nameStack)
{
    std::stringstream strstr;
    for (const std::string &part : nameStack)
    {
        strstr << part;
    }
    return strstr.str();
}
}  // anonymous namespace

BlockLayoutEncoder::BlockLayoutEncoder() : mCurrentOffset(0), mStructureBaseAlignment(0) {}

BlockMemberInfo BlockLayoutEncoder::encodeType(GLenum type,
                                               const std::vector<unsigned int> &arraySizes,
                                               bool isRowMajorMatrix)
{
    int arrayStride;
    int matrixStride;

    getBlockLayoutInfo(type, arraySizes, isRowMajorMatrix, &arrayStride, &matrixStride);

    const BlockMemberInfo memberInfo(static_cast<int>(mCurrentOffset * BytesPerComponent),
                                     static_cast<int>(arrayStride * BytesPerComponent),
                                     static_cast<int>(matrixStride * BytesPerComponent),
                                     isRowMajorMatrix);

    advanceOffset(type, arraySizes, isRowMajorMatrix, arrayStride, matrixStride);

    return memberInfo;
}

void BlockLayoutEncoder::increaseCurrentOffset(size_t offsetInBytes)
{
    mCurrentOffset += (offsetInBytes / BytesPerComponent);
}

void BlockLayoutEncoder::setStructureBaseAlignment(size_t baseAlignment)
{
    mStructureBaseAlignment = baseAlignment;
}

// static
size_t BlockLayoutEncoder::GetBlockRegister(const BlockMemberInfo &info)
{
    return (info.offset / BytesPerComponent) / ComponentsPerRegister;
}

// static
size_t BlockLayoutEncoder::GetBlockRegisterElement(const BlockMemberInfo &info)
{
    return (info.offset / BytesPerComponent) % ComponentsPerRegister;
}

void BlockLayoutEncoder::nextRegister()
{
    mCurrentOffset = rx::roundUp<size_t>(mCurrentOffset, ComponentsPerRegister);
}

Std140BlockEncoder::Std140BlockEncoder() {}

void Std140BlockEncoder::enterAggregateType()
{
    nextRegister();
}

void Std140BlockEncoder::exitAggregateType()
{
    nextRegister();
}

void Std140BlockEncoder::getBlockLayoutInfo(GLenum type,
                                            const std::vector<unsigned int> &arraySizes,
                                            bool isRowMajorMatrix,
                                            int *arrayStrideOut,
                                            int *matrixStrideOut)
{
    // We assume we are only dealing with 4 byte components (no doubles or half-words currently)
    ASSERT(gl::VariableComponentSize(gl::VariableComponentType(type)) == BytesPerComponent);

    size_t baseAlignment = 0;
    int matrixStride     = 0;
    int arrayStride      = 0;

    if (gl::IsMatrixType(type))
    {
        baseAlignment = ComponentsPerRegister;
        matrixStride  = ComponentsPerRegister;

        if (!arraySizes.empty())
        {
            const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
            arrayStride            = ComponentsPerRegister * numRegisters;
        }
    }
    else if (!arraySizes.empty())
    {
        baseAlignment = ComponentsPerRegister;
        arrayStride   = ComponentsPerRegister;
    }
    else
    {
        const int numComponents = gl::VariableComponentCount(type);
        baseAlignment           = (numComponents == 3 ? 4u : static_cast<size_t>(numComponents));
    }

    mCurrentOffset = rx::roundUp(mCurrentOffset, baseAlignment);

    *matrixStrideOut = matrixStride;
    *arrayStrideOut  = arrayStride;
}

void Std140BlockEncoder::advanceOffset(GLenum type,
                                       const std::vector<unsigned int> &arraySizes,
                                       bool isRowMajorMatrix,
                                       int arrayStride,
                                       int matrixStride)
{
    if (!arraySizes.empty())
    {
        mCurrentOffset += arrayStride * gl::ArraySizeProduct(arraySizes);
    }
    else if (gl::IsMatrixType(type))
    {
        const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
        mCurrentOffset += matrixStride * numRegisters;
    }
    else
    {
        mCurrentOffset += gl::VariableComponentCount(type);
    }
}

Std430BlockEncoder::Std430BlockEncoder() {}

void Std430BlockEncoder::nextRegister()
{
    mCurrentOffset = rx::roundUp<size_t>(mCurrentOffset, mStructureBaseAlignment);
}

void Std430BlockEncoder::getBlockLayoutInfo(GLenum type,
                                            const std::vector<unsigned int> &arraySizes,
                                            bool isRowMajorMatrix,
                                            int *arrayStrideOut,
                                            int *matrixStrideOut)
{
    // We assume we are only dealing with 4 byte components (no doubles or half-words currently)
    ASSERT(gl::VariableComponentSize(gl::VariableComponentType(type)) == BytesPerComponent);

    size_t baseAlignment = 0;
    int matrixStride     = 0;
    int arrayStride      = 0;

    if (gl::IsMatrixType(type))
    {
        const int numComponents = gl::MatrixComponentCount(type, isRowMajorMatrix);
        baseAlignment           = (numComponents == 3 ? 4u : static_cast<size_t>(numComponents));
        matrixStride            = baseAlignment;

        if (!arraySizes.empty())
        {
            const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
            arrayStride            = matrixStride * numRegisters;
        }
    }
    else
    {
        const int numComponents = gl::VariableComponentCount(type);
        baseAlignment           = (numComponents == 3 ? 4u : static_cast<size_t>(numComponents));
        if (!arraySizes.empty())
        {
            arrayStride = baseAlignment;
        }
    }
    mStructureBaseAlignment = std::max(baseAlignment, mStructureBaseAlignment);
    mCurrentOffset          = rx::roundUp(mCurrentOffset, baseAlignment);

    *matrixStrideOut = matrixStride;
    *arrayStrideOut  = arrayStride;
}

void GetInterfaceBlockInfo(const std::vector<InterfaceBlockField> &fields,
                           const std::string &prefix,
                           sh::BlockLayoutEncoder *encoder,
                           BlockLayoutMap *blockInfoOut)
{
    // Matrix packing is always recorded in individual fields, so they'll set the row major layout
    // flag to true if needed.
    GetInterfaceBlockInfo(fields, prefix, encoder, false, blockInfoOut);
}

void GetUniformBlockInfo(const std::vector<Uniform> &uniforms,
                         const std::string &prefix,
                         sh::BlockLayoutEncoder *encoder,
                         BlockLayoutMap *blockInfoOut)
{
    // Matrix packing is always recorded in individual fields, so they'll set the row major layout
    // flag to true if needed.
    GetInterfaceBlockInfo(uniforms, prefix, encoder, false, blockInfoOut);
}

// VariableNameVisitor implementation.
VariableNameVisitor::VariableNameVisitor(const std::string &namePrefix,
                                         const std::string &mappedNamePrefix)
{
    if (!namePrefix.empty())
    {
        mNameStack.push_back(namePrefix + ".");
    }

    if (!mappedNamePrefix.empty())
    {
        mMappedNameStack.push_back(mappedNamePrefix + ".");
    }
}

VariableNameVisitor::~VariableNameVisitor() = default;

void VariableNameVisitor::enterArray(const ShaderVariable &arrayVar, unsigned int arrayNestingIndex)
{
    if (arrayNestingIndex == 0)
    {
        mNameStack.push_back(arrayVar.name);
        mMappedNameStack.push_back(arrayVar.mappedName);
    }
}

void VariableNameVisitor::exitArray(const ShaderVariable &arrayVar, unsigned int arrayNestingIndex)
{
    if (arrayNestingIndex == 0)
    {
        mNameStack.pop_back();
        mMappedNameStack.pop_back();
    }
}

void VariableNameVisitor::enterArrayElement(const ShaderVariable &arrayVar,
                                            unsigned int arrayNestingIndex,
                                            unsigned int arrayElement)
{
    std::stringstream strstr;
    strstr << "[" << arrayElement << "]";
    std::string elementString = strstr.str();
    mNameStack.push_back(elementString);
    mMappedNameStack.push_back(elementString);
}

std::string VariableNameVisitor::collapseNameStack() const
{
    return CollapseNameStack(mNameStack);
}

std::string VariableNameVisitor::collapseMappedNameStack() const
{
    return CollapseNameStack(mMappedNameStack);
}

void VariableNameVisitor::visitSampler(const sh::ShaderVariable &sampler)
{
    if (!sampler.isArrayOfArrays())
    {
        mNameStack.push_back(sampler.name);
        mMappedNameStack.push_back(sampler.mappedName);
    }

    std::string name       = collapseNameStack();
    std::string mappedName = collapseMappedNameStack();

    if (!sampler.isArrayOfArrays())
    {
        mNameStack.pop_back();
        mMappedNameStack.pop_back();
    }

    visitNamedSampler(sampler, name, mappedName);
}

void VariableNameVisitor::visitVariable(const ShaderVariable &variable,
                                        unsigned int arrayNestingIndex,
                                        bool isRowMajor)
{
    if (!variable.isArrayOfArrays())
    {
        mNameStack.push_back(variable.name);
        mMappedNameStack.push_back(variable.mappedName);
    }

    std::string name       = collapseNameStack();
    std::string mappedName = collapseMappedNameStack();

    if (!variable.isArrayOfArrays())
    {
        mNameStack.pop_back();
        mMappedNameStack.pop_back();
    }

    visitNamedVariable(variable, arrayNestingIndex, isRowMajor, name, mappedName);
}

// BlockEncoderVisitor implementation.
BlockEncoderVisitor::BlockEncoderVisitor(const std::string &namePrefix,
                                         const std::string &mappedNamePrefix,
                                         BlockLayoutEncoder *encoder)
    : VariableNameVisitor(namePrefix, mappedNamePrefix), mEncoder(encoder)
{}

BlockEncoderVisitor::~BlockEncoderVisitor() = default;

void BlockEncoderVisitor::enterStructAccess(const ShaderVariable &structVar)
{
    VariableNameVisitor::enterStructAccess(structVar);
    mEncoder->enterAggregateType();
}

void BlockEncoderVisitor::exitStructAccess(const ShaderVariable &structVar)
{
    mEncoder->exitAggregateType();
    VariableNameVisitor::exitStructAccess(structVar);
}

void BlockEncoderVisitor::visitNamedVariable(const ShaderVariable &variable,
                                             unsigned int arrayNestingIndex,
                                             bool isRowMajor,
                                             const std::string &name,
                                             const std::string &mappedName)
{
    std::vector<unsigned int> innermostArraySize(
        1u, variable.getNestedArraySize(arrayNestingIndex + 1u));
    BlockMemberInfo variableInfo =
        mEncoder->encodeType(variable.type, innermostArraySize, isRowMajor);
    encodeVariable(variable, variableInfo, name, mappedName);
}

void TraverseShaderVariable(const ShaderVariable &variable,
                            bool isRowMajorLayout,
                            ShaderVariableVisitor *visitor)
{
    bool rowMajorLayout = (isRowMajorLayout || variable.isRowMajorLayout);
    bool isRowMajor     = rowMajorLayout && gl::IsMatrixType(variable.type);

    if (variable.isStruct())
    {
        if (variable.isArray())
        {
            TraverseStructArrayVariable(variable, 0u, rowMajorLayout, visitor);
        }
        else
        {
            visitor->enterStruct(variable);
            TraverseStructVariable(variable, rowMajorLayout, visitor);
            visitor->exitStruct(variable);
        }
    }
    else if (variable.isArrayOfArrays())
    {
        TraverseArrayOfArraysVariable(variable, 0u, isRowMajor, visitor);
    }
    else if (gl::IsSamplerType(variable.type))
    {
        visitor->visitSampler(variable);
    }
    else
    {
        visitor->visitVariable(variable, 0, isRowMajor);
    }
}

void TraverseShaderVariables(const ShaderVariable *vars,
                             size_t numVars,
                             bool isRowMajorLayout,
                             ShaderVariableVisitor *visitor)
{
    for (size_t variableIndex = 0; variableIndex < numVars; ++variableIndex)
    {
        const ShaderVariable &variable = vars[variableIndex];
        TraverseShaderVariable(variable, isRowMajorLayout, visitor);
    }
}
}  // namespace sh
