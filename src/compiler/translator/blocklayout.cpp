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
}  // anonymous namespace

BlockLayoutEncoder::BlockLayoutEncoder() : mCurrentOffset(0)
{
}

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

// static
size_t BlockLayoutEncoder::getBlockRegister(const BlockMemberInfo &info)
{
    return (info.offset / BytesPerComponent) / ComponentsPerRegister;
}

// static
size_t BlockLayoutEncoder::getBlockRegisterElement(const BlockMemberInfo &info)
{
    return (info.offset / BytesPerComponent) % ComponentsPerRegister;
}

void BlockLayoutEncoder::nextRegister()
{
    mCurrentOffset = rx::roundUp<size_t>(mCurrentOffset, ComponentsPerRegister);
}

Std140BlockEncoder::Std140BlockEncoder()
{
}

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
        unsigned int arraySizeProduct = 1u;
        for (unsigned int arraySize : arraySizes)
        {
            arraySizeProduct *= arraySize;
        }
        mCurrentOffset += arrayStride * arraySizeProduct;
    }
    else if (gl::IsMatrixType(type))
    {
        ASSERT(matrixStride == ComponentsPerRegister);
        const int numRegisters = gl::MatrixRegisterCount(type, isRowMajorMatrix);
        mCurrentOffset += ComponentsPerRegister * numRegisters;
    }
    else
    {
        mCurrentOffset += gl::VariableComponentCount(type);
    }
}

template <typename VarT>
void GetUniformBlockStructMemberArrayInfo(const VarT &field,
                                          unsigned int arrayNestingIndex,
                                          const std::string &prefix,
                                          sh::BlockLayoutEncoder *encoder,
                                          bool inRowMajorLayout,
                                          BlockLayoutMap *blockInfoOut)
{
    // Nested arrays are processed starting from outermost (arrayNestingIndex 0u) and ending at the
    // innermost.
    for (unsigned int arrayElement = 0u;
         arrayElement < field.arraySizes[field.arraySizes.size() - 1u - arrayNestingIndex];
         ++arrayElement)
    {
        const std::string elementName = prefix + ArrayString(arrayElement);
        if (arrayNestingIndex + 1u < field.arraySizes.size())
        {
            GetUniformBlockStructMemberArrayInfo(field, arrayNestingIndex + 1u, elementName,
                                                 encoder, inRowMajorLayout, blockInfoOut);
        }
        else
        {
            GetUniformBlockStructMemberInfo(field.fields, elementName, encoder, inRowMajorLayout,
                                            blockInfoOut);
        }
    }
}

template <typename VarT>
void GetUniformBlockStructMemberInfo(const std::vector<VarT> &fields,
                                     const std::string &fieldName,
                                     sh::BlockLayoutEncoder *encoder,
                                     bool inRowMajorLayout,
                                     BlockLayoutMap *blockInfoOut)
{
    encoder->enterAggregateType();
    GetUniformBlockInfo(fields, fieldName, encoder, inRowMajorLayout, blockInfoOut);
    encoder->exitAggregateType();
}

template <typename VarT>
void GetUniformBlockInfo(const std::vector<VarT> &fields,
                         const std::string &prefix,
                         sh::BlockLayoutEncoder *encoder,
                         bool inRowMajorLayout,
                         BlockLayoutMap *blockInfoOut)
{
    for (const VarT &field : fields)
    {
        const std::string &fieldName = (prefix.empty() ? field.name : prefix + "." + field.name);

        if (field.isStruct())
        {
            bool rowMajorLayout = (inRowMajorLayout || IsRowMajorLayout(field));

            if (field.isArray())
            {
                GetUniformBlockStructMemberArrayInfo(field, 0u, fieldName, encoder, rowMajorLayout,
                                                     blockInfoOut);
            }
            else
            {
                GetUniformBlockStructMemberInfo(field.fields, fieldName, encoder, rowMajorLayout,
                                                blockInfoOut);
            }
        }
        else
        {
            bool isRowMajorMatrix = (gl::IsMatrixType(field.type) && inRowMajorLayout);
            (*blockInfoOut)[fieldName] =
                encoder->encodeType(field.type, field.arraySizes, isRowMajorMatrix);
        }
    }
}

template void GetUniformBlockInfo(const std::vector<InterfaceBlockField> &,
                                  const std::string &,
                                  sh::BlockLayoutEncoder *,
                                  bool,
                                  BlockLayoutMap *);

template void GetUniformBlockInfo(const std::vector<Uniform> &,
                                  const std::string &,
                                  sh::BlockLayoutEncoder *,
                                  bool,
                                  BlockLayoutMap *);

template void GetUniformBlockInfo(const std::vector<ShaderVariable> &,
                                  const std::string &,
                                  sh::BlockLayoutEncoder *,
                                  bool,
                                  BlockLayoutMap *);

}  // namespace sh
