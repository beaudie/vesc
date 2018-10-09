//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Wrapper functions for RWByteAddressBuffer Load/Store functions.
//

#include "compiler/translator/ShaderStorageBlockFunctionHLSL.h"

#include "compiler/translator/UtilsHLSL.h"

namespace sh
{

namespace
{

unsigned int GetVectorStride(TLayoutBlockStorage storage)
{
    if (storage == EbsStd140)
    {
        // The base alignment and stride of arrays of scalars and vectors are rounded up a multiple
        // of the base alignment of a vec4
        return 16;
    }
    else
    {
        // TODO(jiajia.qin@intel.com): Calculate the stride for other layout storage.
        // http://anglebug.com/1951
        return 16;
    }
}

}  // anonymous namespace

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOLoadFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunction &ssboFunction)
{
    const char *convertString;
    switch (ssboFunction.type.getBasicType())
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
            convertString = "asint(";
            break;
        default:
            UNREACHABLE();
            return;
    }

    out << "    " << ssboFunction.typeString << " result";
    if (ssboFunction.type.isScalar())
    {
        out << " = " << convertString << "buffer.Load(loc));\n";
    }
    else if (ssboFunction.type.isVector())
    {
        out << " = " << convertString << "buffer.Load" << ssboFunction.type.getNominalSize()
            << "(loc));\n";
    }
    else if (ssboFunction.type.isMatrix())
    {
        out << " = {";
        unsigned int vectorStride =
            GetVectorStride(ssboFunction.type.getLayoutQualifier().blockStorage);
        for (int rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
        {
            out << "asfloat(buffer.Load" << ssboFunction.type.getCols() << "(loc +"
                << rowIndex * vectorStride << ")), ";
        }

        out << "};\n";
    }
    else
    {
        // TODO(jiajia.qin@intel.com): Process all possible return types. http://anglebug.com/1951
        out << ";\n";
    }

    out << "    return result;\n";
    return;
}

// static
void ShaderStorageBlockFunctionHLSL::OutputSSBOStoreFunctionBody(
    TInfoSinkBase &out,
    const ShaderStorageBlockFunction &ssboFunction)
{
    if (ssboFunction.type.isScalar())
    {
        out << "    buffer.Store(loc, asuint(value));\n";
    }
    else if (ssboFunction.type.isVector())
    {
        out << "    buffer.Store" << ssboFunction.type.getNominalSize()
            << "(loc, asuint(value));\n";
    }
    else if (ssboFunction.type.isMatrix())
    {
        unsigned int vectorStride =
            GetVectorStride(ssboFunction.type.getLayoutQualifier().blockStorage);
        for (int rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
        {
            out << "    buffer.Store" << ssboFunction.type.getCols() << "(loc +"
                << rowIndex * vectorStride << ", asuint(value[" << rowIndex << "]));\n";
        }
    }
    else
    {
        // TODO(jiajia.qin@intel.com): Process all possible return types. http://anglebug.com/1951
    }
}

bool ShaderStorageBlockFunctionHLSL::ShaderStorageBlockFunction::operator<(
    const ShaderStorageBlockFunction &rhs) const
{
    return std::tie(functionName, typeString, method) <
           std::tie(rhs.functionName, rhs.typeString, rhs.method);
}

TString ShaderStorageBlockFunctionHLSL::registerShaderStorageBlockFunction(const TType &type,
                                                                           SSBOMethod method)
{
    ShaderStorageBlockFunction ssboFunction;
    ssboFunction.typeString = TypeString(type);
    ssboFunction.method     = method;
    ssboFunction.type       = type;

    switch (method)
    {
        case SSBOMethod::LOAD:
            ssboFunction.functionName = ssboFunction.typeString + "_Load";
            break;
        case SSBOMethod::STORE:
            ssboFunction.functionName = ssboFunction.typeString + "_Store";
            break;
        default:
            UNREACHABLE();
    }

    mRegisteredShaderStorageBlockFunctions.insert(ssboFunction);
    return ssboFunction.functionName;
}

void ShaderStorageBlockFunctionHLSL::shaderStorageBlockFunctionHeader(TInfoSinkBase &out)
{
    for (const ShaderStorageBlockFunction &ssboFunction : mRegisteredShaderStorageBlockFunctions)
    {
        if (ssboFunction.method == SSBOMethod::LOAD)
        {
            // Function header
            out << ssboFunction.typeString << " " << ssboFunction.functionName
                << "(RWByteAddressBuffer buffer, uint loc)\n";
            out << "{\n";
            OutputSSBOLoadFunctionBody(out, ssboFunction);
        }
        else
        {
            // Function header
            out << "void " << ssboFunction.functionName << "(RWByteAddressBuffer buffer, uint loc, "
                << ssboFunction.typeString << " value)\n";
            out << "{\n";
            OutputSSBOStoreFunctionBody(out, ssboFunction);
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
