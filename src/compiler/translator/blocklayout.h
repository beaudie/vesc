//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout.h:
//   Methods and classes related to uniform layout and packing in GLSL and HLSL.
//

#ifndef COMMON_BLOCKLAYOUT_H_
#define COMMON_BLOCKLAYOUT_H_

#include <cstddef>
#include <vector>

#include "angle_gl.h"
#include "common/utilities.h"

#include <GLSLANG/ShaderLang.h>

namespace sh
{
struct ShaderVariable;
struct InterfaceBlockField;
struct Uniform;
struct Varying;
struct InterfaceBlock;

struct BlockMemberInfo
{
    BlockMemberInfo() : offset(-1), arrayStride(-1), matrixStride(-1), isRowMajorMatrix(false) {}

    BlockMemberInfo(int offset, int arrayStride, int matrixStride, bool isRowMajorMatrix)
        : offset(offset),
          arrayStride(arrayStride),
          matrixStride(matrixStride),
          isRowMajorMatrix(isRowMajorMatrix)
    {
    }

    static BlockMemberInfo getDefaultBlockInfo() { return BlockMemberInfo(-1, -1, -1, false); }

    int offset;
    int arrayStride;
    int matrixStride;
    bool isRowMajorMatrix;
};

class BlockLayoutEncoder
{
  public:
    BlockLayoutEncoder();
    virtual ~BlockLayoutEncoder() {}

    BlockMemberInfo encodeType(GLenum type, unsigned int arraySize, bool isRowMajorMatrix);

    size_t getBlockSize() const { return mCurrentOffset * BytesPerComponent; }

    virtual void enterAggregateType() = 0;
    virtual void exitAggregateType()  = 0;

    static const size_t BytesPerComponent           = 4u;
    static const unsigned int ComponentsPerRegister = 4u;

    static size_t getBlockRegister(const BlockMemberInfo &info);
    static size_t getBlockRegisterElement(const BlockMemberInfo &info);

  protected:
    size_t mCurrentOffset;

    void nextRegister();

    virtual void getBlockLayoutInfo(GLenum type,
                                    unsigned int arraySize,
                                    bool isRowMajorMatrix,
                                    int *arrayStrideOut,
                                    int *matrixStrideOut) = 0;
    virtual void advanceOffset(GLenum type,
                               unsigned int arraySize,
                               bool isRowMajorMatrix,
                               int arrayStride,
                               int matrixStride) = 0;
};

// Block layout according to the std140 block layout
// See "Standard Uniform Block Layout" in Section 2.11.6 of the OpenGL ES 3.0 specification

class Std140BlockEncoder : public BlockLayoutEncoder
{
  public:
    Std140BlockEncoder();

    void enterAggregateType() override;
    void exitAggregateType() override;

  protected:
    void getBlockLayoutInfo(GLenum type,
                            unsigned int arraySize,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override;
    void advanceOffset(GLenum type,
                       unsigned int arraySize,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override;
};

inline bool IsRowMajorLayout(const sh::InterfaceBlockField &var)
{
    return var.isRowMajorLayout;
}

inline bool IsRowMajorLayout(const sh::ShaderVariable &var)
{
    return false;
}

template <typename VarT, typename ProcessorT>
void GetUniformBlockInfo(const std::vector<VarT> &fields,
                         const std::string &prefix,
                         sh::BlockLayoutEncoder *encoder,
                         bool inRowMajorLayout,
                         ProcessorT *processor)
{
    for (const VarT &field : fields)
    {
        const std::string &fieldName = (prefix.empty() ? field.name : prefix + "." + field.name);

        if (field.isStruct())
        {
            bool rowMajorLayout = (inRowMajorLayout || IsRowMajorLayout(field));

            for (unsigned int arrayElement = 0; arrayElement < field.elementCount(); arrayElement++)
            {
                encoder->enterAggregateType();

                const std::string uniformElementName =
                    fieldName + (field.isArray() ? ArrayString(arrayElement) : "");
                GetUniformBlockInfo(field.fields, uniformElementName, encoder, rowMajorLayout,
                                    processor);

                encoder->exitAggregateType();
            }
        }
        else
        {
            bool isRowMajorMatrix = (gl::IsMatrixType(field.type) && inRowMajorLayout);
            const BlockMemberInfo &blockMemberInfo =
                encoder->encodeType(field.type, field.arraySize, isRowMajorMatrix);

            (*processor)(fieldName, blockMemberInfo);
        }
    }
}

}  // namespace sh

#endif  // COMMON_BLOCKLAYOUT_H_
