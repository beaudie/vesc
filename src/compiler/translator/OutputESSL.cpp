//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/OutputESSL.h"

namespace sh
{

TOutputESSL::TOutputESSL(TInfoSinkBase &objSink,
                         ShArrayIndexClampingStrategy clampingStrategy,
                         ShHashFunction64 hashFunction,
                         NameMap &nameMap,
                         TSymbolTable &symbolTable,
                         sh::GLenum shaderType,
                         int shaderVersion,
                         bool forceHighp,
                         ShCompileOptions compileOptions)
    : TOutputGLSLBase(objSink,
                      clampingStrategy,
                      hashFunction,
                      nameMap,
                      symbolTable,
                      shaderType,
                      shaderVersion,
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
    TInfoSinkBase &out = objSink();

    const TString &symbol = node->getSymbol();
    if (symbol == "gl_FragDepthEXT" && getShaderVersion() >= 300)
    {
        out << "gl_FragDepth";
    }
    else
    {
        TOutputGLSLBase::visitSymbol(node);
    }
}

TString TOutputESSL::translateTextureFunction(const TString &name)
{
    if (getShaderVersion() >= 300)
    {
        static constexpr std::pair<const char *, const char *> essl300Renames[] = {
            // EXT_shader_texture_lod
            {"texture2DLodEXT", "textureLod"},         {"texture2DProjLodEXT", "textureProjLod"},
            {"texture2DProjLodEXT", "textureProjLod"}, {"textureCubeLodEXT", "textureLod"},
            {"texture2DGradEXT", "textureGrad"},       {"texture2DProjGradEXT", "textureProjGrad"},
            {"textureCubeGradEXT", "textureGrad"},
        };

        for (const auto &rename : essl300Renames)
        {
            if (name == rename.first)
            {
                return rename.second;
            }
        }
    }

    return name;
}

}  // namespace sh
