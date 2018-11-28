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
bool IsRowMajorLayout(const InterfaceBlockField &var)
{
    return var.isRowMajorLayout;
}

bool IsRowMajorLayout(const ShaderVariable &var)
{
    return false;
}

template <typename VarT>
void GetInterfaceBlockInfo(const std::vector<VarT> &fields,
                           const std::string &prefix,
                           sh::BlockLayoutEncoder *encoder,
                           bool inRowMajorLayout,
                           BlockLayoutMap *blockInfoOut);

template <typename VarT>
void GetInterfaceBlockStructMemberInfo(const std::vector<VarT> &fields,
                                       const std::string &fieldName,
                                       sh::BlockLayoutEncoder *encoder,
                                       bool inRowMajorLayout,
                                       BlockLayoutMap *blockInfoOut)
{
    encoder->enterAggregateType(fields.data(), fields.size());
    GetInterfaceBlockInfo(fields, fieldName, encoder, inRowMajorLayout, blockInfoOut);
    encoder->exitAggregateType(false);
}

template <typename VarT>
void GetInterfaceBlockStructArrayMemberInfo(const VarT &field,
                                            unsigned int arrayNestingIndex,
                                            const std::string &arrayName,
                                            sh::BlockLayoutEncoder *encoder,
                                            bool inRowMajorLayout,
                                            BlockLayoutMap *blockInfoOut)
{
    // Nested arrays are processed starting from outermost (arrayNestingIndex 0u) and ending at the
    // innermost.
    const unsigned int currentArraySize = field.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < currentArraySize; ++arrayElement)
    {
        const std::string elementName = arrayName + ArrayString(arrayElement);
        if (arrayNestingIndex + 1u < field.arraySizes.size())
        {
            GetInterfaceBlockStructArrayMemberInfo(field, arrayNestingIndex + 1u, elementName,
                                                   encoder, inRowMajorLayout, blockInfoOut);
        }
        else
        {
            GetInterfaceBlockStructMemberInfo(field.fields, elementName, encoder, inRowMajorLayout,
                                              blockInfoOut);
        }
    }
}

template <typename VarT>
void GetInterfaceBlockArrayOfArraysMemberInfo(const VarT &field,
                                              unsigned int arrayNestingIndex,
                                              const std::string &arrayName,
                                              sh::BlockLayoutEncoder *encoder,
                                              bool isRowMajorMatrix,
                                              BlockLayoutMap *blockInfoOut)
{
    const unsigned int currentArraySize = field.getNestedArraySize(arrayNestingIndex);
    for (unsigned int arrayElement = 0u; arrayElement < currentArraySize; ++arrayElement)
    {
        const std::string elementName = arrayName + ArrayString(arrayElement);
        if (arrayNestingIndex + 2u < field.arraySizes.size())
        {
            GetInterfaceBlockArrayOfArraysMemberInfo(field, arrayNestingIndex + 1u, elementName,
                                                     encoder, isRowMajorMatrix, blockInfoOut);
        }
        else
        {
            std::vector<unsigned int> innermostArraySize(
                1u, field.getNestedArraySize(arrayNestingIndex + 1u));
            (*blockInfoOut)[elementName] =
                encoder->encodeType(field.type, innermostArraySize, isRowMajorMatrix);
        }
    }
}

template <typename VarT>
void GetInterfaceBlockInfo(const std::vector<VarT> &fields,
                           const std::string &prefix,
                           sh::BlockLayoutEncoder *encoder,
                           bool inRowMajorLayout,
                           BlockLayoutMap *blockInfoOut)
{
    for (const VarT &field : fields)
    {
        // Skip samplers. On Vulkan we use this for the default uniform block, so samplers may be
        // included.
        if (gl::IsSamplerType(field.type))
        {
            continue;
        }

        const std::string &fieldName = (prefix.empty() ? field.name : prefix + "." + field.name);

        bool rowMajorLayout = (inRowMajorLayout || IsRowMajorLayout(field));

        if (field.isStruct())
        {
            if (field.isArray())
            {
                GetInterfaceBlockStructArrayMemberInfo(field, 0u, fieldName, encoder,
                                                       rowMajorLayout, blockInfoOut);
            }
            else
            {
                GetInterfaceBlockStructMemberInfo(field.fields, fieldName, encoder, rowMajorLayout,
                                                  blockInfoOut);
            }
        }
        else if (field.isArrayOfArrays())
        {
            GetInterfaceBlockArrayOfArraysMemberInfo(field, 0u, fieldName, encoder,
                                                     rowMajorLayout && gl::IsMatrixType(field.type),
                                                     blockInfoOut);
        }
        else
        {
            (*blockInfoOut)[fieldName] = encoder->encodeType(
                field.type, field.arraySizes, rowMajorLayout && gl::IsMatrixType(field.type));
        }
    }
}

}  // anonymous namespace

// BlockLayoutEncoder implementation.
BlockLayoutEncoder::BlockLayoutEncoder() : mCurrentOffset(0) {}

BlockMemberInfo BlockLayoutEncoder::encodeType(GLenum type,
                                               const std::vector<unsigned int> &arraySizes,
                                               bool isRowMajorMatrix)
{
    int arrayStride;
    int matrixStride;

    getBlockLayoutInfo(type, arraySizes, isRowMajorMatrix, &arrayStride, &matrixStride);

    const BlockMemberInfo memberInfo(static_cast<int>(mCurrentOffset * kBytesPerComponent),
                                     static_cast<int>(arrayStride * kBytesPerComponent),
                                     static_cast<int>(matrixStride * kBytesPerComponent),
                                     isRowMajorMatrix, -1);

    advanceOffset(type, arraySizes, isRowMajorMatrix, arrayStride, matrixStride);

    return memberInfo;
}

// static
size_t BlockLayoutEncoder::getBlockRegister(const BlockMemberInfo &info)
{
    return (info.offset / kBytesPerComponent) / kComponentsPerRegister;
}

// static
size_t BlockLayoutEncoder::getBlockRegisterElement(const BlockMemberInfo &info)
{
    return (info.offset / kBytesPerComponent) % kComponentsPerRegister;
}

void BlockLayoutEncoder::nextRegister()
{
    mCurrentOffset = rx::roundUp<size_t>(mCurrentOffset, kComponentsPerRegister);
}

// Std140BlockEncoder implementation.
Std140BlockEncoder::Std140BlockEncoder() {}

void Std140BlockEncoder::enterAggregateType(const ShaderVariable *fields, size_t fieldCount)
{
    nextRegister();
    mAggregateOffsetStack.push(mCurrentOffset);
}

BlockMemberInfo Std140BlockEncoder::exitAggregateType(bool isRowMajor)
{
    nextRegister();

    ASSERT(!mAggregateOffsetStack.empty());
    size_t startOffset = mAggregateOffsetStack.top();
    mAggregateOffsetStack.pop();

    size_t arrayStride = (mCurrentOffset - startOffset) * kBytesPerComponent;

    return BlockMemberInfo(startOffset, arrayStride, 0, isRowMajor, -1);
}

void Std140BlockEncoder::getBlockLayoutInfo(GLenum type,
                                            const std::vector<unsigned int> &arraySizes,
                                            bool isRowMajorMatrix,
                                            int *arrayStrideOut,
                                            int *matrixStrideOut)
{
    // We assume we are only dealing with 4 byte components (no doubles or half-words currently)
    ASSERT(gl::VariableComponentSize(gl::VariableComponentType(type)) == kBytesPerComponent);

    size_t baseAlignment = 0;
    int matrixStride     = 0;
    int arrayStride      = 0;

    if (gl::IsMatrixType(type))
    {
        baseAlignment = kComponentsPerRegister;
        matrixStride  = kComponentsPerRegister;

        if (!arraySizes.empty())
        {
            const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
            arrayStride            = kComponentsPerRegister * numRegisters;
        }
    }
    else if (!arraySizes.empty())
    {
        baseAlignment = kComponentsPerRegister;
        arrayStride   = kComponentsPerRegister;
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

// Std430BlockEncoder implementation.
Std430BlockEncoder::Std430BlockEncoder() {}

void Std430BlockEncoder::enterAggregateType(const ShaderVariable *fields, size_t fieldCount)
{
    // TODO(jiajia.qin@intel.com): Compute structure base alignment. http://anglebug.com/1920
    nextRegister();
    mAggregateOffsetStack.push(mCurrentOffset);
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
}  // namespace sh
