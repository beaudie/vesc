//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidatePixelLocalStorage.h"

#include <GLES2/gl2.h>
#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class PLSErrorTraverser : public TLValueTrackingTraverser
{
  public:
    PLSErrorTraverser(TSymbolTable *symbolTable, TDiagnostics *diagnostics)
        : TLValueTrackingTraverser(true, false, true, symbolTable), mDiagnostics(diagnostics)
    {}

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *function) override
    {
        mVisitingMain = visit == Visit::PreVisit && function->getFunction()->isMain();
        return true;
    }

    bool visitBranch(Visit visit, TIntermBranch *branch) override
    {
        if (visit != Visit::PreVisit)
        {
            return true;
        }

        switch (branch->getFlowOp())
        {
            case EOpKill:
                // Discard is not allowed when pixel local storage has been declared. When
                // polyfilled with shader images, pixel local storage requires early_fragment_tests,
                // which causes discard to interact differently with the depth and stencil tests.
                //
                // To ensure identical behavior across all backends (some of which may not have
                // access to early_fragment_tests), we disallow discard if pixel local storage has
                // been declared.
                error(branch->getLine(), "illegal discard when pixel local storage is declared",
                      "discard");
                break;

            case EOpReturn:
                // Returning from main isn't allowed when pixel local storage has been declared.
                // (ARB_fragment_shader_interlock isn't allowed after return from main.)
                if (mVisitingMain)
                {
                    error(branch->getLine(),
                          "illegal return from main when pixel local storage is declared",
                          "return");
                }
                break;

            default:
                break;
        }

        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (isLValueRequiredHere())
        {
            // When polyfilled with shader images, pixel local storage requires
            // early_fragment_tests, which causes assignments to gl_FragDepth(EXT) and gl_SampleMask
            // to be ignored.
            //
            // To ensure identical behavior across all backends, we disallow assignment to these
            // values if pixel local storage has been declared.
            if (symbol->getName().beginsWith("gl_FragDepth") ||  // gl_FragDepth, gl_FragDepthEXT,
                symbol->getName().beginsWith("gl_SampleMask"))
            {
                error(symbol->getLine(),
                      "value not assignable when pixel local storage is declared",
                      symbol->getName().data());
            }
        }
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *prototype) override
    {
        const TFunction *fn = prototype->getFunction();
        for (size_t i = 0; i < fn->getParamCount(); ++i)
        {
            const TVariable *param = fn->getParam(i);
            if (IsPixelLocal(param->getType().getBasicType()))
            {
                // PLS handles aren't allowed as function arguments.
                // TODO(anglebug.com/7484): Consider allowing this once it is properly supported for
                // images.
                error(prototype->getLine(),
                      "function arguments cannot be pixel local storage handles",
                      param->name().data());
            }
        }
    }

    bool hasErrors() const { return mHasErrors; }

  private:
    void error(const TSourceLoc &loc, const char *reason, const char *token)
    {
        mDiagnostics->error(loc, reason, token);
        mHasErrors = true;
    }

    TDiagnostics *mDiagnostics;
    angle::HashSet<int> mPLSBindings;
    bool mHasErrors    = false;
    bool mVisitingMain = false;
};

}  // namespace

bool ValidatePixelLocalStorage(TIntermBlock *root,
                               TSymbolTable *symbolTable,
                               TDiagnostics *diagnostics)
{
    PLSErrorTraverser traverser(symbolTable, diagnostics);
    root->traverse(&traverser);
    return !traverser.hasErrors();
}

}  // namespace sh
