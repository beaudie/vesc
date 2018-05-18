//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Helper functions for writing implementations of ESSL shader
// storage block read/write access into RWByteAddressBuffer Load/Store functions.
//

#include "compiler/translator/ShaderStorageBlockFunctionHLSL.h"
#include "compiler/translator/UtilsHLSL.h"

namespace sh
{

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOLoadFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &ssboFunction)
{
    TString convertString;
    switch (ssboFunction.basicType)
    {
        case EbtFloat:
            convertString = "asfloat(";
            break;
        case EbtInt:
            convertString = "asint(";
            break;
        case EbtUInt:
            convertString = "asuint(";
            break;
        case EbtBool:
            convertString = "asuint(";
            break;
        default:
            UNREACHABLE();
            break;
    }
    if (ssboFunction.returnType == "float" || ssboFunction.returnType == "int" ||
        ssboFunction.returnType == "uint")
    {
        out << "    " << ssboFunction.returnType << " result = " << convertString
            << "buffer.Load(loc));\n";
        out << "    return result;\n";
        return;
    }

    // TODO(jiajia.qin@intel.com): Process all possible return types.
    out << "    return 1.0;\n";
}

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOStoreFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction &ssboFunction)
{
    if (ssboFunction.returnType == "float" || ssboFunction.returnType == "int" ||
        ssboFunction.returnType == "uint")
    {
        out << "    buffer.Store(loc, asuint(value));\n";
    }

    // TODO(jiajia.qin@intel.com): Process all possible return types.
}

bool ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction::operator<(
    const ShaderStorageBlockFunction &rhs) const
{
    return std::tie(functionName, returnType, basicType, method) <
           std::tie(rhs.functionName, rhs.returnType, rhs.basicType, rhs.method);
}

void ShaderStorageBlockFunctionHLSL::useShaderStorageBlockFunction(const TString &name,
                                                                   const TString &returnType,
                                                                   const TBasicType &basictype,
                                                                   SSBOMethod method)
{
    ShaderStorageBlockFunction ssboFunction;
    ssboFunction.functionName = name;
    ssboFunction.returnType   = returnType;
    ssboFunction.basicType    = basictype;
    ssboFunction.method       = method;

    mUsedShaderStorageBlockFunctions.insert(ssboFunction);
}

void ShaderStorageBlockFunctionHLSL::shaderStorageBlockFunctionHeader(TInfoSinkBase &out)
{
    for (const ShaderStorageBlockFunction &ssboFunction : mUsedShaderStorageBlockFunctions)
    {
        if (ssboFunction.method == SSBOMethod::LOAD)
        {
            // Function header
            // TODO(jiajia.qin@intel.com): Add swizzle process.
            out << ssboFunction.returnType << " " << ssboFunction.functionName
                << "(RWByteAddressBuffer buffer, uint loc)\n";
            out << "{\n";
            OutputSSBOLoadFunctionBody(out, ssboFunction);
        }
        else
        {
            // Function header
            // TODO(jiajia.qin@intel.com): Add swizzle process.
            out << "void " << ssboFunction.functionName << "(RWByteAddressBuffer buffer, uint loc, "
                << ssboFunction.returnType << " value)\n";
            out << "{\n";
            OutputSSBOStoreFunctionBody(out, ssboFunction);
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
