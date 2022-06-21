//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright 2022 Rive
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

// Returns true if the shader has any top-level PLS declarations.
// Generates errors at PLS declaration locations if shaderType is not GL_FRAGMENT_SHADER.
bool IsPixelLocalStorageDeclared(TIntermBlock *root, GLenum shaderType, TDiagnostics *diagnostics)
{
    const auto &sequence = *root->getSequence();
    bool foundPLS        = false;
    for (size_t index = 0; index < sequence.size(); ++index)
    {
        if (auto *decl = sequence[index]->getAsDeclarationNode())
        {
            auto *declVariable = decl->getSequence()->front()->getAsTyped();
            ASSERT(declVariable);
            if (!IsPixelLocal(declVariable->getBasicType()))
            {
                continue;
            }
            if (shaderType == GL_FRAGMENT_SHADER)
            {
                return true;
            }
            diagnostics->error(declVariable->getLine(),
                               "undefined use of pixel local storage outside a fragment shader",
                               getBasicString(declVariable->getBasicType()));
            foundPLS = true;
        }
    }
    return foundPLS;
}

class PLSErrorTraverser : public TLValueTrackingTraverser
{
  public:
    PLSErrorTraverser(TSymbolTable *symbolTable, TDiagnostics *diagnostics)
        : TLValueTrackingTraverser(true, false, true, symbolTable), mDiagnostics(diagnostics)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *decl) override
    {
        if (visit != Visit::PreVisit)
        {
            return true;
        }

        TIntermTyped *declVariable = (decl->getSequence())->front()->getAsTyped();
        ASSERT(declVariable);

        const TType &declType = declVariable->getType();
        if (!IsPixelLocal(declType.getBasicType()))
        {
            return true;
        }

        // This visitDeclaration doesn't get called for function arguments, and opaque types can
        // otherwise only be uniforms.
        ASSERT(declType.getQualifier() == EvqUniform);

        int binding = declType.getLayoutQualifier().binding;
        ASSERT(!declType.isArray());  // PLS is not allowed in arrays.
        if (mPLSBindings.contains(binding))
        {
            // PLS handles cannot have duplicate binding indices.
            error(declVariable->getLine(), "duplicate pixel local storage binding index",
                  std::to_string(binding).c_str());
        }
        mPLSBindings.insert(binding);
        return true;
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *prototype) override
    {
        auto *fn = prototype->getFunction();
        for (size_t i = 0; i < fn->getParamCount(); ++i)
        {
            auto *param = fn->getParam(i);
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

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *function) override
    {
        mVisitingMain = visit == Visit::PreVisit ? function->getFunction()->isMain() : false;
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
                // Discard is not allowed when pixel local storage has been declared. The shader
                // image polyfill requires early_fragment_tests, which causes discard to interact
                // differently with the depth and stencil tests.
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
            // The shader image polyfill requires early_fragment_tests, which causes
            // gl_FragDepth(EXT) and gl_SampleMask to be ignored.
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

PixelLocalStorageUsage ValidatePixelLocalStorage(TIntermBlock *root,
                                                 GLenum shaderType,
                                                 TSymbolTable *symbolTable,
                                                 TDiagnostics *diagnostics)
{
    if (!IsPixelLocalStorageDeclared(root, shaderType, diagnostics))
    {
        // The extra PLS restrctions only apply if PLS variables have been declared.
        return PixelLocalStorageUsage::Unused;
    }
    if (shaderType != GL_FRAGMENT_SHADER)
    {
        // IsPixelLocalStorageDeclared already generated errors.
        return PixelLocalStorageUsage::Invalid;
    }
    PLSErrorTraverser traverser(symbolTable, diagnostics);
    root->traverse(&traverser);
    return traverser.hasErrors() ? PixelLocalStorageUsage::Invalid : PixelLocalStorageUsage::Valid;
}

}  // namespace sh
