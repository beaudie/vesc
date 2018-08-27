//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateAST.h"

#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class ValidateAST : public TIntermTraverser
{
  public:
    static bool validate(TIntermNode *root, const ValidateASTOptions &options);

    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitIfElse(Visit visit, TIntermIfElse *node) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitCase(Visit visit, TIntermCase *node) override;
    void visitFunctionPrototype(TIntermFunctionPrototype *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;
    bool visitInvariantDeclaration(Visit visit, TIntermInvariantDeclaration *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitLoop(Visit visit, TIntermLoop *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    void visitPreprocessorDirective(TIntermPreprocessorDirective *node) override;

  private:
    ValidateAST(TIntermNode *root, const ValidateASTOptions &options);

    // Visit as a generic node
    void visitNode(Visit visit, TIntermNode *node);

    void expectNonNullChildren(Visit visit, size_t least_count);

    bool validateInternal();

    ValidateASTOptions mOptions;

    // For validateSingleParent:
    std::map<TIntermNode *, TIntermNode *> mParent;
    bool mSingleParentFailed = false;

    // For validateNullNodes
    bool mNullNodesFailed = false;

    // TODO: For other options
};

bool ValidateAST::validate(TIntermNode *root, const ValidateASTOptions &options)
{
    ValidateAST validate(root, options);
    root->traverse(&validate);
    return validate.validateInternal();
}

ValidateAST::ValidateAST(TIntermNode *root, const ValidateASTOptions &options)
    : TIntermTraverser(true, false, true, nullptr)
{
    if (mOptions.validateSingleParent)
    {
        mParent[root] = nullptr;
    }
}

void ValidateAST::visitNode(Visit, TIntermSymbol *node)
{
    if (visit == PreVisit && mOptions.validateSingleParent)
    {
        size_t childCount = node->getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            TIntermNode *child = node->getChildNode(i);
            if (mParent.find(child) != mParent.end())
            {
                // If child is visited twice but through the same parent, the problem is in one of
                // the ancestors.
                if (mParent[child] != node)
                {
                    mDiagnostics->error(getLine(), "Found child with two parents",
                                        "<validateSingleParent>");
                    mSingleParentFailed = true;
                }
            }

            mParent[child] = node;
        }
    }
}

void ValidateAST::expectNonNullChildren(Visit visit, size_t least_count)
{
    if (visit == PreVisit && mOptions.validateNullNodes)
    {
        size_t childCount = node->getChildCount();
        if (childCount < least_count)
        {
            mDiagnostics->error(getLine(), "Too few children", "<validateNullNodes>");
            mNullNodesFailed = true;
        }

        for (size_t i = 0; i < childCount; ++i)
        {
            if (node->getChildNode(i) == nullptr)
            {
                mDiagnostics->error(getLine(), "Found nullptr child", "<validateNullNodes>");
                mNullNodesFailed = true;
            }
        }
    }
}

void ValidateAST::visitSymbol(TIntermSymbol *node)
{
    visitNode(visit, node);
    // TODO
}

void ValidateAST::visitConstantUnion(TIntermConstantUnion *node)
{
    visitNode(visit, node);
    // TODO
}

bool ValidateAST::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitBinary(Visit visit, TIntermBinary *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitUnary(Visit visit, TIntermUnary *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitTernary(Visit visit, TIntermTernary *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitIfElse(Visit visit, TIntermIfElse *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitSwitch(Visit visit, TIntermSwitch *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitCase(Visit visit, TIntermCase *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

void ValidateAST::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    visitNode(visit, node);
    // TODO
}

bool ValidateAST::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitAggregate(Visit visit, TIntermAggregate *node)
{
    visitNode(visit, node);
    expectNonNullChildren(visit, 1);
    // TODO
    return true;
}

bool ValidateAST::visitBlock(Visit visit, TIntermBlock *node)
{
    visitNode(visit, node);
    expectNonNullChildren(visit, 0);
    // TODO
    return true;
}

bool ValidateAST::visitInvariantDeclaration(Visit visit, TIntermInvariantDeclaration *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    visitNode(visit, node);
    expectNonNullChildren(visit, 1);
    // TODO
    return true;
}

bool ValidateAST::visitLoop(Visit visit, TIntermLoop *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

bool ValidateAST::visitBranch(Visit visit, TIntermBranch *node)
{
    visitNode(visit, node);
    // TODO
    return true;
}

void ValidateAST::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    visitNode(visit, node);
    // TODO
}

bool ValidateAST::validateInternal(const TSourceLoc &loc)
{
    return !mSingleParentFailed && !mNullNodesFailed;
}

}  // anonymous namespace

bool ValidateAST(TIntermNode *root, const ValidateASTOptions &options);
{
    return ValidateAST::validate(root, options);
}

}  // namespace sh
