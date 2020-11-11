//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DriverUniform.h: Add code to support driver uniforms
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_DRIVERUNIFORM_H_
#define COMPILER_TRANSLATOR_TREEUTIL_DRIVERUNIFORM_H_

#include "common/angleutils.h"
#include "compiler/translator/Types.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TIntermNode;
class TSymbolTable;
class TIntermTyped;
class TIntermSwizzle;
class TIntermBinary;

class DriverUniform
{
  public:
    DriverUniform() : mDriverUniforms(nullptr), mEmulatedDepthRangeType(nullptr) {}
    virtual ~DriverUniform() = default;

    bool addComputeDriverUniformsToShader(TIntermBlock *root, TSymbolTable *symbolTable);
    bool addGraphicsDriverUniformsToShader(TIntermBlock *root, TSymbolTable *symbolTable);

    TIntermBinary *getFlipXYRef() const;
    TIntermBinary *getNegFlipXYRef() const;
    TIntermBinary *getFragRotationMatrixRef() const;
    TIntermBinary *getPreRotationMatrixRef() const;
    TIntermBinary *getViewportRef() const;
    TIntermBinary *getHalfRenderAreaRef() const;
    TIntermBinary *getAbcBufferOffsets() const;
    TIntermBinary *getClipDistancesEnabled() const;
    TIntermSwizzle *getNegFlipYRef() const;
    TIntermBinary *getDepthRangeReservedFieldRef() const;

  protected:
    TIntermBinary *createDriverUniformRef(const char *fieldName) const;
    virtual TFieldList *createUniformFields() const;

    const TVariable *mDriverUniforms;
    TType *mEmulatedDepthRangeType;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_DRIVERUNIFORM_H_
