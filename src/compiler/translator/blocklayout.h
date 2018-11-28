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
#include <map>
#include <stack>
#include <vector>

#include <GLSLANG/ShaderLang.h>
#include "angle_gl.h"

namespace sh
{
struct ShaderVariable;
struct InterfaceBlockField;
struct Uniform;
struct Varying;
struct InterfaceBlock;

struct BlockMemberInfo
{
    constexpr BlockMemberInfo() = default;

    constexpr BlockMemberInfo(int offset,
                              int arrayStride,
                              int matrixStride,
                              bool isRowMajorMatrix,
                              int topLevelArrayStride)
        : offset(offset),
          arrayStride(arrayStride),
          matrixStride(matrixStride),
          isRowMajorMatrix(isRowMajorMatrix),
          topLevelArrayStride(topLevelArrayStride)
    {}

    // A single integer identifying the offset of an active variable.
    int offset = -1;

    // A single integer identifying the stride between array elements in an active variable.
    int arrayStride = -1;

    // A single integer identifying the stride between columns of a column-major matrix or rows of a
    // row-major matrix.
    int matrixStride = -1;

    // A single integer identifying whether an active variable is a row-major matrix.
    bool isRowMajorMatrix = false;

    // A single integer identifying the number of active array elements of the top-level shader
    // storage block member containing the active variable.
    int topLevelArrayStride = -1;
};

constexpr BlockMemberInfo kDefaultBlockMemberInfo;

class BlockLayoutEncoder
{
  public:
    BlockLayoutEncoder();
    virtual ~BlockLayoutEncoder() {}

    BlockMemberInfo encodeType(GLenum type,
                               const std::vector<unsigned int> &arraySizes,
                               bool isRowMajorMatrix);

    size_t getBlockSize() const { return mCurrentOffset * kBytesPerComponent; }

    // Called when entering a new structure or array.
    // Returns the offset of the aggregate type.
    virtual void enterAggregateType(const ShaderVariable *fields, size_t fieldCount) = 0;

    virtual BlockMemberInfo exitAggregateType(bool isRowMajor) = 0;

    static constexpr size_t kBytesPerComponent           = 4u;
    static constexpr unsigned int kComponentsPerRegister = 4u;

    static size_t getBlockRegister(const BlockMemberInfo &info);
    static size_t getBlockRegisterElement(const BlockMemberInfo &info);

  protected:
    size_t mCurrentOffset;
    std::stack<size_t> mAggregateOffsetStack;

    void nextRegister();

    virtual void getBlockLayoutInfo(GLenum type,
                                    const std::vector<unsigned int> &arraySizes,
                                    bool isRowMajorMatrix,
                                    int *arrayStrideOut,
                                    int *matrixStrideOut) = 0;
    virtual void advanceOffset(GLenum type,
                               const std::vector<unsigned int> &arraySizes,
                               bool isRowMajorMatrix,
                               int arrayStride,
                               int matrixStride)          = 0;
};

// Block layout according to the std140 block layout
// See "Standard Uniform Block Layout" in Section 2.11.6 of the OpenGL ES 3.0 specification

class Std140BlockEncoder : public BlockLayoutEncoder
{
  public:
    Std140BlockEncoder();

    void enterAggregateType(const ShaderVariable *fields, size_t fieldCount) override;
    BlockMemberInfo exitAggregateType(bool isRowMajor) override;

  protected:
    void getBlockLayoutInfo(GLenum type,
                            const std::vector<unsigned int> &arraySizes,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override;
    void advanceOffset(GLenum type,
                       const std::vector<unsigned int> &arraySizes,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override;
};

class Std430BlockEncoder : public Std140BlockEncoder
{
  public:
    Std430BlockEncoder();

    void enterAggregateType(const ShaderVariable *fields, size_t fieldCount) override;
};

using BlockLayoutMap = std::map<std::string, BlockMemberInfo>;

void GetInterfaceBlockInfo(const std::vector<InterfaceBlockField> &fields,
                           const std::string &prefix,
                           sh::BlockLayoutEncoder *encoder,
                           BlockLayoutMap *blockInfoOut);

// Used for laying out the default uniform block on the Vulkan backend.
void GetUniformBlockInfo(const std::vector<Uniform> &uniforms,
                         const std::string &prefix,
                         sh::BlockLayoutEncoder *encoder,
                         BlockLayoutMap *blockInfoOut);

}  // namespace sh

#endif  // COMMON_BLOCKLAYOUT_H_
