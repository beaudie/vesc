//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/OutputESSL.h"
#include "compiler/translator/util.h"

namespace sh
{

TOutputESSL::TOutputESSL(TInfoSinkBase &objSink,
                         ShArrayIndexClampingStrategy clampingStrategy,
                         ShHashFunction64 hashFunction,
                         NameMap &nameMap,
                         TSymbolTable &symbolTable,
                         sh::GLenum shaderType,
                         int shaderVersion,
                         bool usesMultiview,
                         bool forceHighp,
                         ShCompileOptions compileOptions)
    : TOutputGLSLBase(objSink,
                      clampingStrategy,
                      hashFunction,
                      nameMap,
                      symbolTable,
                      shaderType,
                      shaderVersion,
                      usesMultiview,
                      SH_ESSL_OUTPUT,
                      compileOptions),
      mForceHighp(forceHighp)
{
}

bool TOutputESSL::writeVariablePrecision(TPrecision precision)
{
    if (precision == EbpUndefined)
        return false;

    TInfoSinkBase &out = objSink();
    if (mForceHighp)
        out << getPrecisionString(EbpHigh);
    else
        out << getPrecisionString(precision);
    return true;
}

void TOutputESSL::visitSymbol(TIntermSymbol *node)
{
    TInfoSinkBase &out    = objSink();
    const TString &symbol = node->getSymbol();

    const bool isGLBuiltin = symbol.compare(0, 3, "gl_") != 0;
    if (mUsesMultiview && isGLBuiltin && getShaderType() == GL_VERTEX_SHADER &&
        IsVaryingOut(node->getType().getQualifier()))
    {
        out << "angle_vert_output_" << symbol;
    }
    else if (mUsesMultiview && isGLBuiltin && getShaderType() == GL_FRAGMENT_SHADER &&
             IsVaryingIn(node->getType().getQualifier()))
    {
        out << "angle_frag_input_" << symbol;
    }
    else
    {
        TOutputGLSLBase::visitSymbol(node);
    }
}

}  // namespace sh
