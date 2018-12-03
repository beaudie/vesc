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
class BlockLayoutMapVisitor : public VariableNameBlockVisitor
{
  public:
    BlockLayoutMapVisitor(BlockLayoutMap *blockInfoOut, const std::string &instanceName)
        : VariableNameBlockVisitor(instanceName), mInfoOut(blockInfoOut)
    {}

    void visitVariable(const ShaderVariable &variable, const BlockMemberInfo &variableInfo) override
    {
        if (gl::IsSamplerType(variable.type))
            return;

        std::string name  = collapseNameStack(variable.name);
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
    BlockLayoutMapVisitor visitor(blockInfoOut, prefix);
    EncodeShaderVariables(fields.data(), fields.size(), inRowMajorLayout, encoder, &visitor);
}

void EncodeStructVariable(const ShaderVariable &variable,
                          bool isRowMajorLayout,
                          BlockLayoutEncoder *encoder,
                          BlockEncodingVisitor *visitor)
{
    const std::vector<ShaderVariable> &fields = variable.fields;

    visitor->enterStructAccess(variable);

    encoder->enterAggregateType();
    EncodeShaderVariables(fields.data(), fields.size(), isRowMajorLayout, encoder, visitor);
    encoder->exitAggregateType();

    visitor->exitStructAccess(variable);
}

void EncodeStructArrayVariable(const ShaderVariable &variable,
                               unsigned int arrayNestingIndex,
                               bool inRowMajorLayout,
                               BlockLayoutEncoder *encoder,
                               BlockEncodingVisitor *visitor)
{
    // Nested arrays are processed starting from outermost (arrayNestingIndex 0u) and ending at the
    // innermost.
    const unsigned int currentArraySize = variable.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < currentArraySize; ++arrayElement)
    {
        visitor->enterArrayElement(variable, arrayNestingIndex, arrayElement);

        if (arrayNestingIndex + 1u < variable.arraySizes.size())
        {
            EncodeStructArrayVariable(variable, arrayNestingIndex + 1u, inRowMajorLayout, encoder,
                                      visitor);
        }
        else
        {
            EncodeStructVariable(variable, inRowMajorLayout, encoder, visitor);
        }

        visitor->exitArrayElement(variable, arrayNestingIndex, arrayElement);
    }
}

void EncodeArrayOfArraysVariable(const ShaderVariable &variable,
                                 unsigned int arrayNestingIndex,
                                 bool isRowMajorMatrix,
                                 BlockLayoutEncoder *encoder,
                                 BlockEncodingVisitor *visitor)
{
    const unsigned int currentArraySize = variable.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < currentArraySize; ++arrayElement)
    {
        if (arrayNestingIndex + 2u < variable.arraySizes.size())
        {
            EncodeArrayOfArraysVariable(variable, arrayNestingIndex + 1u, isRowMajorMatrix, encoder,
                                        visitor);
        }
        else
        {
            std::vector<unsigned int> innermostArraySize(
                1u, variable.getNestedArraySize(arrayNestingIndex + 1u));
            BlockMemberInfo variableInfo =
                encoder->encodeType(variable.type, innermostArraySize, isRowMajorMatrix);

            visitor->visitVariable(variable, variableInfo);
        }
    }
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

void EncodeShaderVariable(const ShaderVariable &variable,
                          bool isRowMajorLayout,
                          BlockLayoutEncoder *encoder,
                          BlockEncodingVisitor *visitor)
{
    bool rowMajorLayout = (isRowMajorLayout || variable.isRowMajorLayout);
    bool isRowMajor     = rowMajorLayout && gl::IsMatrixType(variable.type);

    if (variable.isStruct())
    {
        visitor->enterStruct(variable);

        if (variable.isArray())
        {
            EncodeStructArrayVariable(variable, 0u, rowMajorLayout, encoder, visitor);
        }
        else
        {
            EncodeStructVariable(variable, rowMajorLayout, encoder, visitor);
        }

        visitor->exitStruct(variable);
    }
    else if (variable.isArrayOfArrays())
    {
        EncodeArrayOfArraysVariable(variable, 0u, isRowMajor, encoder, visitor);
    }
    else
    {
        BlockMemberInfo variableInfo =
            encoder->encodeType(variable.type, variable.arraySizes, isRowMajor);
        if (visitor)
        {
            visitor->visitVariable(variable, variableInfo);
        }
    }
}

void EncodeShaderVariables(const ShaderVariable *vars,
                           size_t numVars,
                           bool isRowMajorLayout,
                           BlockLayoutEncoder *encoder,
                           BlockEncodingVisitor *visitor)
{
    for (size_t variableIndex = 0; variableIndex < numVars; ++variableIndex)
    {
        const ShaderVariable &variable = vars[variableIndex];
        EncodeShaderVariable(variable, isRowMajorLayout, encoder, visitor);
    }
}

void VariableNameBlockVisitor::enterArrayElement(const ShaderVariable &arrayVar,
                                                 unsigned int arrayNestingIndex,
                                                 unsigned int arrayElement)
{
    std::stringstream strstr;
    strstr << "[" << arrayElement << "]";
    mNameStack.push_back(strstr.str());
}

std::string VariableNameBlockVisitor::collapseNameStack(const std::string &top) const
{
    std::stringstream strstr;
    for (const std::string &part : mNameStack)
    {
        strstr << part;
    }
    strstr << top;
    return strstr.str();
}
}  // namespace sh
