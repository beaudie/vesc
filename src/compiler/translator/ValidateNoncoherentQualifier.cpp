//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ValidateNoncoherentQualifier validates noncoherent qualifier. It checks whether noncoherent
// qualifier are missed at gl_LastFragData. 'inout' variable is already checked at ParseContext.cpp
//

#include "compiler/translator/ValidateNoncoherentQualifier.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/ParseContext.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

void error(const TIntermSymbol &symbol, const char *reason, TDiagnostics *diagnostics)
{
    diagnostics->error(symbol.getLine(), reason, symbol.getName().data());
}

class ValidateNoncoherentQualifierTraverser : public TIntermTraverser
{
  public:
    ValidateNoncoherentQualifierTraverser(const TExtensionBehavior &extBehavior);

    void validate(TDiagnostics *diagnostics) const;

    void visitSymbol(TIntermSymbol *node) override;

  private:
    bool mIsShaderFramebufferFetchNoncoherentEnabled;

    const TIntermSymbol *mLastFragData;
};

ValidateNoncoherentQualifierTraverser::ValidateNoncoherentQualifierTraverser(
    const TExtensionBehavior &extBehavior)
    : TIntermTraverser(true, false, false),
      mIsShaderFramebufferFetchNoncoherentEnabled(
          IsExtensionEnabled(extBehavior, TExtension::EXT_shader_framebuffer_fetch_non_coherent)),
      mLastFragData(nullptr)
{}

void ValidateNoncoherentQualifierTraverser::visitSymbol(TIntermSymbol *symbol)
{
    const TVariable &variable = symbol->variable();

    if (variable.symbolType() == SymbolType::Empty)
    {
        return;
    }

    if (mLastFragData != nullptr)
    {
        return;
    }

    if (variable.name() == "gl_LastFragData" &&
        !variable.getType().getLayoutQualifier().noncoherent)
    {
        mLastFragData = symbol;
    }
}

void ValidateNoncoherentQualifierTraverser::validate(TDiagnostics *diagnostics) const
{
    ASSERT(diagnostics);

    if (mIsShaderFramebufferFetchNoncoherentEnabled)
    {
        if (mLastFragData != nullptr)
        {
            std::stringstream strstr = sh::InitializeStream<std::stringstream>();
            strstr << "gl_LastFragData should be redeclared with noncoherent qualifier when using "
                      "EXT_shader_framebuffer_fetch_non_coherent extension";
            error(*mLastFragData, strstr.str().c_str(), diagnostics);
        }
    }
}

}  // anonymous namespace

bool ValidateNoncoherentQualifier(TIntermBlock *root,
                                  const TExtensionBehavior &extBehavior,
                                  TDiagnostics *diagnostics)
{
    ValidateNoncoherentQualifierTraverser validateNoncoherentQualifer(extBehavior);
    root->traverse(&validateNoncoherentQualifer);
    int numErrorsBefore = diagnostics->numErrors();
    validateNoncoherentQualifer.validate(diagnostics);
    return (diagnostics->numErrors() == numErrorsBefore);
}

}  // namespace sh
