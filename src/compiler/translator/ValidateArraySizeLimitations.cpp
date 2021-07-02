//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateArraySizeLimitations.h"

#include "angle_gl.h"
#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

// Arbitrarily enforce that arrays - even local variables - declared
// with a size in bytes of over 2 GB will cause compilation failure.
constexpr size_t kMaxArraySizeInBytes = static_cast<size_t>(2) * 1024 * 1024 * 1024;

// Traverses intermediate tree to ensure that the shader does not
// exceed certain implementation-defined limits on array sizes.
class ValidateArraySizeLimitationsTraverser : public TIntermTraverser
{
  public:
    ValidateArraySizeLimitationsTraverser(TSymbolTable *symbolTable, TDiagnostics *diagnostics)
        : TIntermTraverser(true, false, false, symbolTable), mDiagnostics(diagnostics)
    {
        ASSERT(diagnostics);
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        for (TIntermNode *variableNode : sequence)
        {
            // See CollectVariablesTraverser::visitDeclaration for a
            // deeper analysis of the AST structures that might be
            // encountered.
            TIntermSymbol *asSymbol = variableNode->getAsSymbolNode();
            TIntermBinary *asBinary = variableNode->getAsBinaryNode();

            if (asBinary != nullptr)
            {
                ASSERT(asBinary->getOp() == EOpInitialize);
                asSymbol = asBinary->getLeft()->getAsSymbolNode();
            }

            ASSERT(asSymbol);

            const TVariable &variable = asSymbol->variable();
            if (variable.symbolType() == SymbolType::AngleInternal)
            {
                // Ignore internal variables.
                continue;
            }

            const TType &variableType = asSymbol->getType();
            if (!variableType.isArray())
            {
                // Only look at array-typed variables.
                continue;
            }

            // Create a ShaderVariable from which to compute
            // (conservative) sizing information.
            ShaderVariable shaderVar;
            setCommonVariableProperties(variableType, variable, &shaderVar);

            // Compute the std140 layout of this array variable,
            // assuming it's a member of a block (which it might not be).
            Std140BlockEncoder layoutEncoder;
            BlockEncoderVisitor visitor("", "", &layoutEncoder);
            // Row-major layout won't affect layout size.
            bool isRowMajorLayout = false;
            TraverseShaderVariable(shaderVar, isRowMajorLayout, &visitor);
            if (layoutEncoder.getCurrentOffset() > kMaxArraySizeInBytes)
            {
                error(asSymbol->getLine(),
                      "Size of array variable exceeds implementation-defined limit",
                      asSymbol->getName());
                return false;
            }
        }

        return true;
    }

  private:
    void error(TSourceLoc loc, const char *reason, const ImmutableString &token)
    {
        mDiagnostics->error(loc, reason, token.data());
    }

    void setFieldOrVariableProperties(const TType &type,
                                      bool staticUse,
                                      bool isShaderIOBlock,
                                      bool isPatch,
                                      ShaderVariable *variableOut) const
    {
        ASSERT(variableOut);

        variableOut->staticUse       = staticUse;
        variableOut->isShaderIOBlock = isShaderIOBlock;
        variableOut->isPatch         = isPatch;

        const TStructure *structure           = type.getStruct();
        const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
        if (structure)
        {
            // Structures use a NONE type that isn't exposed outside ANGLE.
            variableOut->type = GL_NONE;
            if (structure->symbolType() != SymbolType::Empty)
            {
                variableOut->structOrBlockName = structure->name().data();
            }

            const TFieldList &fields = structure->fields();

            for (const TField *field : fields)
            {
                // Regardless of the variable type (uniform, in/out etc.) its fields are always
                // plain ShaderVariable objects.
                ShaderVariable fieldVariable;
                setFieldProperties(*field->type(), field->name(), staticUse, isShaderIOBlock,
                                   isPatch, &fieldVariable);
                variableOut->fields.push_back(fieldVariable);
            }
        }
        else if (interfaceBlock && isShaderIOBlock)
        {
            variableOut->type = GL_NONE;
            if (interfaceBlock->symbolType() != SymbolType::Empty)
            {
                variableOut->structOrBlockName = interfaceBlock->name().data();
            }
            const TFieldList &fields = interfaceBlock->fields();
            for (const TField *field : fields)
            {
                ShaderVariable fieldVariable;
                setFieldProperties(*field->type(), field->name(), staticUse, true, isPatch,
                                   &fieldVariable);
                fieldVariable.isShaderIOBlock = true;
                variableOut->fields.push_back(fieldVariable);
            }
        }
        else
        {
            variableOut->type      = GLVariableType(type);
            variableOut->precision = GLVariablePrecision(type);
        }

        const TSpan<const unsigned int> &arraySizes = type.getArraySizes();
        if (!arraySizes.empty())
        {
            variableOut->arraySizes.assign(arraySizes.begin(), arraySizes.end());
            // WebGL does not support tessellation shaders; removed
            // code specific to that shader type.
        }
    }

    void setFieldProperties(const TType &type,
                            const ImmutableString &name,
                            bool staticUse,
                            bool isShaderIOBlock,
                            bool isPatch,
                            ShaderVariable *variableOut) const
    {
        ASSERT(variableOut);
        setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);
        variableOut->name.assign(name.data(), name.length());
    }

    void setCommonVariableProperties(const TType &type,
                                     const TVariable &variable,
                                     ShaderVariable *variableOut) const
    {
        ASSERT(variableOut);

        const bool staticUse       = mSymbolTable->isStaticallyUsed(variable);
        const bool isShaderIOBlock = type.getInterfaceBlock() != nullptr;
        const bool isPatch =
            type.getQualifier() == EvqPatchIn || type.getQualifier() == EvqPatchOut;

        setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);

        const bool isNamed = variable.symbolType() != SymbolType::Empty;

        ASSERT(isNamed || isShaderIOBlock);
        if (isNamed)
        {
            variableOut->name.assign(variable.name().data(), variable.name().length());
        }

        // For I/O blocks, additionally store the name of the block as blockName.  If the variable
        // is unnamed, this name will be used instead for the purpose of interface matching.
        if (isShaderIOBlock)
        {
            const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
            ASSERT(interfaceBlock);

            variableOut->structOrBlockName.assign(interfaceBlock->name().data(),
                                                  interfaceBlock->name().length());
            variableOut->isShaderIOBlock = true;
        }
    }

    TDiagnostics *mDiagnostics;
    std::vector<int> mLoopSymbolIds;
};

}  // namespace

bool ValidateArraySizeLimitations(TIntermNode *root,
                                  TSymbolTable *symbolTable,
                                  TDiagnostics *diagnostics)
{
    ValidateArraySizeLimitationsTraverser validate(symbolTable, diagnostics);
    root->traverse(&validate);
    return diagnostics->numErrors() == 0;
}

}  // namespace sh
