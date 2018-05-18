//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Wrapper functions for RWByteAddressBuffer Load/Store functions.
//

#ifndef COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_

#include <set>

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
    TString registerShaderStorageBlockFunction(const TType &type, SSBOMethod method);

    void shaderStorageBlockFunctionHeader(TInfoSinkBase &out);

  private:
    struct ShaderStorageBlockFunction
    {
        bool operator<(const ShaderStorageBlockFunction &rhs) const;
        TString functionName;
        TString typeString;
        SSBOMethod method;
        TType type;
    };

    static void OutputSSBOLoadFunctionBody(
        TInfoSinkBase &out,
        const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &imageFunction);
    static void OutputSSBOStoreFunctionBody(
        TInfoSinkBase &out,
        const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &imageFunction);
    using ShaderStorageBlockFunctionSet = std::set<ShaderStorageBlockFunction>;
    ShaderStorageBlockFunctionSet mRegisteredShaderStorageBlockFunctions;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKFUNCTIONHLSL_H_
