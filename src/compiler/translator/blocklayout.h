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
    BlockMemberInfo()
        : offset(-1),
          arrayStride(-1),
          matrixStride(-1),
          isRowMajorMatrix(false),
          topLevelArrayStride(-1)
    {}

    BlockMemberInfo(int offset, int arrayStride, int matrixStride, bool isRowMajorMatrix)
        : offset(offset),
          arrayStride(arrayStride),
          matrixStride(matrixStride),
          isRowMajorMatrix(isRowMajorMatrix),
          topLevelArrayStride(-1)
    {}

    BlockMemberInfo(int offset,
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

    static BlockMemberInfo getDefaultBlockInfo() { return BlockMemberInfo(-1, -1, -1, false, -1); }

    int offset;
    int arrayStride;
    int matrixStride;
    bool isRowMajorMatrix;
    int topLevelArrayStride;  // Only used for shader storage block members.
};

class BlockLayoutEncoder
{
  public:
    BlockLayoutEncoder();
    virtual ~BlockLayoutEncoder() {}

    BlockMemberInfo encodeType(GLenum type,
                               const std::vector<unsigned int> &arraySizes,
                               bool isRowMajorMatrix);

    size_t getBlockSize() const { return mCurrentOffset * BytesPerComponent; }
    size_t getStructureBaseAlignment() const { return mStructureBaseAlignment; }
    void increaseCurrentOffset(size_t offsetInBytes);
    void setStructureBaseAlignment(size_t baseAlignment);

    virtual void enterAggregateType() = 0;
    virtual void exitAggregateType()  = 0;

    static const size_t BytesPerComponent           = 4u;
    static const unsigned int ComponentsPerRegister = 4u;

    static size_t GetBlockRegister(const BlockMemberInfo &info);
    static size_t GetBlockRegisterElement(const BlockMemberInfo &info);

  protected:
    size_t mCurrentOffset;
    size_t mStructureBaseAlignment;

    virtual void nextRegister();

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

// Will return default values for everything.
class DummyBlockEncoder : public BlockLayoutEncoder
{
  public:
    DummyBlockEncoder() = default;

    void enterAggregateType() override {}
    void exitAggregateType() override {}

  protected:
    void getBlockLayoutInfo(GLenum type,
                            const std::vector<unsigned int> &arraySizes,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override
    {
        *arrayStrideOut  = 0;
        *matrixStrideOut = 0;
    }

    void advanceOffset(GLenum type,
                       const std::vector<unsigned int> &arraySizes,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override
    {}
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

  protected:
    void nextRegister() override;
    void getBlockLayoutInfo(GLenum type,
                            const std::vector<unsigned int> &arraySizes,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override;
};

using BlockLayoutMap = std::map<std::string, BlockMemberInfo>;

void GetInterfaceBlockInfo(const std::vector<InterfaceBlockField> &fields,
                           const std::string &prefix,
                           BlockLayoutEncoder *encoder,
                           BlockLayoutMap *blockInfoOut);

// Used for laying out the default uniform block on the Vulkan backend.
void GetUniformBlockInfo(const std::vector<Uniform> &uniforms,
                         const std::string &prefix,
                         BlockLayoutEncoder *encoder,
                         BlockLayoutMap *blockInfoOut);

class BlockEncodingVisitor
{
  public:
    virtual ~BlockEncodingVisitor() {}

    virtual void enterStruct(const ShaderVariable &structVar) {}
    virtual void exitStruct(const ShaderVariable &structVar) {}

    virtual void enterStructAccess(const ShaderVariable &structVar) {}
    virtual void exitStructAccess(const ShaderVariable &structVar) {}

    virtual void enterArray(const ShaderVariable &arrayVar, unsigned int arrayNestingIndex) {}
    virtual void exitArray(const ShaderVariable &arrayVar, unsigned int arrayNestingIndex) {}

    virtual void enterArrayElement(const ShaderVariable &arrayVar,
                                   unsigned int arrayNestingIndex,
                                   unsigned int arrayElement)
    {}
    virtual void exitArrayElement(const ShaderVariable &arrayVar,
                                  unsigned int arrayNestingIndex,
                                  unsigned int arrayElement)
    {}

    virtual void visitVariable(const ShaderVariable &variable,
                               const BlockMemberInfo &variableInfo) = 0;

  protected:
    BlockEncodingVisitor() {}
};

void EncodeShaderVariable(const ShaderVariable &variable,
                          bool isRowMajorLayout,
                          BlockLayoutEncoder *encoder,
                          BlockEncodingVisitor *visitor);
void EncodeShaderVariables(const ShaderVariable *vars,
                           size_t numVars,
                           bool isRowMajorLayout,
                           BlockLayoutEncoder *encoder,
                           BlockEncodingVisitor *visitor);

class VariableNameBlockVisitor : public BlockEncodingVisitor
{
  public:
    VariableNameBlockVisitor(const std::string &instanceName)
    {
        if (!instanceName.empty())
        {
            mNameStack.push_back(instanceName + ".");
        }
    }

    void enterStruct(const ShaderVariable &structVar) override
    {
        mNameStack.push_back(structVar.name);
    }

    void exitStruct(const ShaderVariable &structVar) override { mNameStack.pop_back(); }

    void enterStructAccess(const ShaderVariable &structVar) override { mNameStack.push_back("."); }

    void exitStructAccess(const ShaderVariable &structVar) override { mNameStack.pop_back(); }

    void enterArrayElement(const ShaderVariable &arrayVar,
                           unsigned int arrayNestingIndex,
                           unsigned int arrayElement) override;

    void exitArrayElement(const ShaderVariable &arrayVar,
                          unsigned int arrayNestingIndex,
                          unsigned int arrayElement) override
    {
        mNameStack.pop_back();
    }

  protected:
    std::string collapseNameStack(const std::string &top) const;

  private:
    std::vector<std::string> mNameStack;
};
}  // namespace sh

#endif  // COMMON_BLOCKLAYOUT_H_
