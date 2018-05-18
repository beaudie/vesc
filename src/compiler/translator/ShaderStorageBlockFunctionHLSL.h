//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Helper functions for writing implementations of ESSL shader
// storage block read/write access into RWByteAddressBuffer Load/Store functions.
//

#ifndef COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_

#include <set>

#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Types.h"

namespace sh
{

enum class SSBOMethod
{
    LOAD,
    STORE
};

class ShaderStorageBlockFunctionHLSL final : angle::NonCopyable
{
  public:
    void useShaderStorageBlockFunction(const TString &name,
                                       const TString &returnType,
                                       const TBasicType &type,
                                       SSBOMethod method);

    void shaderStorageBlockFunctionHeader(TInfoSinkBase &out);

  private:
    struct ShaderStorageBlockFunction
    {
        bool operator<(const ShaderStorageBlockFunction &rhs) const;
        TString functionName;
        TString returnType;
        TBasicType basicType;
        SSBOMethod method;
    };

    static void OutputSSBOLoadFunctionBody(
        TInfoSinkBase &out,
        const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &imageFunction);
    static void OutputSSBOStoreFunctionBody(
        TInfoSinkBase &out,
        const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &imageFunction);
    using ShaderStorageBlockFunctionSet = std::set<ShaderStorageBlockFunction>;
    ShaderStorageBlockFunctionSet mUsedShaderStorageBlockFunctions;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_
