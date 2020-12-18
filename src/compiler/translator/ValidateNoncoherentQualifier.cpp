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
    bool visitDeclaration(Visit visit, TIntermDeclaration *symbol) override;

  private:
    bool mIsShaderFramebufferFetchNoncoherentEnabled;

    const TIntermSymbol *mLastFragData;
    const TIntermSymbol *mUseLastFragData;
};

ValidateNoncoherentQualifierTraverser::ValidateNoncoherentQualifierTraverser(
    const TExtensionBehavior &extBehavior)
    : TIntermTraverser(true, false, false),
      mIsShaderFramebufferFetchNoncoherentEnabled(
          IsExtensionEnabled(extBehavior, TExtension::EXT_shader_framebuffer_fetch_non_coherent)),
      mLastFragData(nullptr),
      mUseLastFragData(nullptr)
{}

void ValidateNoncoherentQualifierTraverser::visitSymbol(TIntermSymbol *symbol)
{
    if (symbol->variable().symbolType() == SymbolType::Empty)
        return;

    if (mUseLastFragData == nullptr || (mLastFragData != nullptr && mLastFragData != symbol))
    {
        mUseLastFragData = symbol;
    }
}

bool ValidateNoncoherentQualifierTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());

    if (sequence.size() != 1)
    {
        return true;
    }

    const TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getName() == "gl_LastFragData")
    {
        mLastFragData = symbol;
    }

    return true;
}

void ValidateNoncoherentQualifierTraverser::validate(TDiagnostics *diagnostics) const
{
    ASSERT(diagnostics);

    if (mIsShaderFramebufferFetchNoncoherentEnabled)
    {
        if (mLastFragData == nullptr || !mLastFragData->getType().getLayoutQualifier().noncoherent)
        {
            std::stringstream strstr = sh::InitializeStream<std::stringstream>();
            strstr << "gl_LastFragData should be redeclared with noncoherent qualifier when using "
                      "EXT_shader_framebuffer_fetch_non_coherent extension";
            error(*mUseLastFragData, strstr.str().c_str(), diagnostics);
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
