//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockFunctionHLSL: Wrapper functions for RWByteAddressBuffer Load/Store functions.
//

#include "compiler/translator/ShaderStorageBlockFunctionHLSL.h"

#include "compiler/translator/UtilsHLSL.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/blocklayoutHLSL.h"
#include "compiler/translator/util.h"

namespace sh
{

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
        if (ssboFunction.rowMajor)
        {
            out << " = {";
            for (int index = 0; index < ssboFunction.type.getNominalSize(); index++)
            {
                out << convertString << "buffer.Load(loc + " << index * ssboFunction.matrixStride
                    << ")),";
            }
            out << "};\n";
        }
        else
        {
            out << " = " << convertString << "buffer.Load" << ssboFunction.type.getNominalSize()
                << "(loc));\n";
        }
    }
    else if (ssboFunction.type.isMatrix())
    {
        out << " = {";
        if (ssboFunction.rowMajor)
        {
            for (int columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                out << "{ ";
                for (int rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
                {
                    out << "asfloat(buffer.Load(loc + "
                        << rowIndex * ssboFunction.matrixStride +
                               columnIndex * BlockLayoutEncoder::BytesPerComponent
                        << "))";
                    if (rowIndex != (ssboFunction.type.getRows() - 1))
                    {
                        out << ",";
                    }
                }
                out << "}, ";
            }
        }
        else
        {
            for (int columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                out << "asfloat(buffer.Load" << ssboFunction.type.getRows() << "(loc + "
                    << columnIndex * ssboFunction.matrixStride << ")), ";
            }
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
        if (ssboFunction.type.getBasicType() == EbtBool)
        {
            out << "    uint _tmp = uint(value);\n"
                << "    buffer.Store(loc, _tmp);\n";
        }
        else
        {
            out << "    buffer.Store(loc, asuint(value));\n";
        }
    }
    else if (ssboFunction.type.isVector())
    {
        if (ssboFunction.rowMajor)
        {
            for (int index = 0; index < ssboFunction.type.getNominalSize(); index++)
            {
                // Don't need to worry about bool value since there is no bool matrix.
                out << "buffer.Store(loc + " << index * ssboFunction.matrixStride
                    << ", asuint(value[" << index << "]));\n";
            }
        }
        else
        {
            if (ssboFunction.type.getBasicType() == EbtBool)
            {
                out << "    uint" << ssboFunction.type.getNominalSize() << " _tmp = uint"
                    << ssboFunction.type.getNominalSize() << "(value);\n";
                out << "    buffer.Store" << ssboFunction.type.getNominalSize() << "(loc, _tmp);\n";
            }
            else
            {
                out << "    buffer.Store" << ssboFunction.type.getNominalSize()
                    << "(loc, asuint(value));\n";
            }
        }
    }
    else if (ssboFunction.type.isMatrix())
    {
        if (ssboFunction.rowMajor)
        {
            for (int columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                for (int rowIndex = 0; rowIndex < ssboFunction.type.getRows(); rowIndex++)
                {
                    out << "    buffer.Store(loc + "
                        << rowIndex * ssboFunction.matrixStride +
                               columnIndex * BlockLayoutEncoder::BytesPerComponent
                        << ", asuint(value[" << columnIndex << "][" << rowIndex << "]));\n";
                }
            }
        }
        else
        {
            for (int columnIndex = 0; columnIndex < ssboFunction.type.getCols(); columnIndex++)
            {
                out << "    buffer.Store" << ssboFunction.type.getRows() << "(loc + "
                    << columnIndex * ssboFunction.matrixStride << ", asuint(value[" << columnIndex
                    << "]));\n";
            }
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

TString ShaderStorageBlockFunctionHLSL::registerShaderStorageBlockFunction(
    const TType &type,
    SSBOMethod method,
    TLayoutBlockStorage storage,
    bool rowMajor,
    int matrixStride)
{
    ShaderStorageBlockFunction ssboFunction;
    ssboFunction.typeString = TypeString(type);
    ssboFunction.method     = method;
    ssboFunction.type       = type;
    ssboFunction.rowMajor     = rowMajor;
    ssboFunction.matrixStride = matrixStride;

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
    if (matrixStride > 0)
    {
        if (rowMajor)
        {
            ssboFunction.functionName += "_rm_";
        }
        else
        {
            ssboFunction.functionName += "_cm_";
        }
        ssboFunction.functionName += getBlockStorageString(storage);
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
