//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteRowMajorMatrices: Rewrite row-major matrices as column-major.
//
// TODO: make sure to write tests for and support mixed packing for default uniforms.  If it's not
// already handled, probably just transposing data when assigning uniform values should take care of
// it (as well as ignoring row-major-ness when calculating default uniform packing).  If for some
// reason that doesn't work, the transformation here can be applied after the default uniform is
// turned to a UBO in the Vulkan translator.
//

#include "compiler/translator/tree_ops/RewriteRowMajorMatrices.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
// Only structs with matrices are tracked.  If layout(row_major) is applied to a struct that doesn't
// have matrices, it's silently dropped.  This is also used to avoid creating duplicates for inner
// structs that don't have matrices.
struct StructConversionData
{
    // The converted struct with every matrix transposed.
    TStructure *convertedStruct = nullptr;

    // The copy-from and copy-to functions copying from a struct to its converted version and back.
    TIntermFunctionDefinition *copyFrom = nullptr;
    TIntermFunctionDefinition *copyTo   = nullptr;
};

bool IsFieldRowMajor(TField *field, bool isBlockRowMajor)
{
    TLayoutMatrixPacking matrixPacking = field->type()->getLayoutQualifier().matrixPacking;

    // The field is row major if either explicitly specified as such, or if it inherits it from the
    // block layout qualifier.
    return matrixPacking == EmpRowMajor || (matrixPacking == EmpUnspecified && isBlockRowMajor);
}

TType *TransposeMatrix(TType *type)
{
    TType *newType = new TType(*type);

    TLayoutQualifier layoutQualifier = newType->getLayoutQualifier();
    layoutQualifier.matrixPacking    = EmpUnspecified;

    newType->setPrimarySize(fieldType->getRows());
    newType->setSecondarySize(fieldType->getCols());

    return newType;
}

void CopyArraySizes(const TType *from, TType *to)
{
    if (from->isArray())
    {
        to->makeArray(*from->getArraySizes());
    }
}

// Determine if the node is an index node (array index or struct field selection).  For the purposes
// of this transformation, swizzle nodes are considered index nodes too.
bool IsIndexNode(TIntermNode *node)
{
    if (node->getAsSwizzleNode())
    {
        return true;
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode == nullptr)
    {
        return false;
    }

    TOperator op = binaryNode->getOp();

    return op == EOpIndexDirect || op == EOpIndexDirectInterfaceBlock ||
           op == EOpIndexDirectStruct || op == EOpIndexIndirect;
}

// Traverser that:
//
// 1. Converts |layout(row_major) matCxR M| to |layout(column_major) matRxC Mt|.
// 2. Converts |layout(row_major) S s| to |layout(column_major) St st|, where S is a struct that
//    contains matrices, and St is a new struct with the transformation in 1 applied to matrix
//    members (recursively).
// 3. When read from, the following transformations are applied:
//
//            M       -> transpose(Mt)
//            M[c]    -> gvecN(Mt[0][c], Mt[1][c], ..., Mt[N-1][c])
//            M[c][r] -> Mt[r][c]
//            M[c].yz -> gvec2(Mt[1][c], Mt[2][c])
//            s       -> copy_St_to_S(st)
//            (matrix reads through struct transformed similarly to M)
//            (matrix arrays similarly transformed)
//
// 4. When written to, the following transformations are applied:
//
//      M = exp       -> Mt = transpose(exp)
//      M[c] = exp    -> temp = exp
//                       Mt[0][c] = exp[0]
//                       Mt[1][c] = exp[1]
//                       ...
//                       Mt[N-1][c] = exp[N-1]
//      M[c][r] = exp -> Mt[r][c] = exp
//      M[c].yz = exp -> temp = exp
//                       Mt[1][c] = exp[0]
//                       Mt[2][c] = exp[1]
//      s = exp       -> st = copy_S_to_St(exp)
//      (matrix writes through struct transformed similarly to M)
//      (matrix arrays similarly transformed)
//
// 5. If any of the above is passed to an `inout` parameter, both transformations are applied:
//
//            f(M[c]) -> temp = gvecN(Mt[0][c], Mt[1][c], ..., Mt[N-1][c])
//                       f(temp)
//                       Mt[0][c] = exp[0]
//                       Mt[1][c] = exp[1]
//                       ...
//                       Mt[N-1][c] = exp[N-1]
//
//               f(s) -> temp = copy_St_to_S(st)
//                       f(temp)
//                       st = copy_S_to_St(temp)
//
class RewriteRowMajorMatricesTraverser : public TIntermTraverser
{
  public:
    RewriteRowMajorMatricesTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, true, true, symbolTable)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit != PreVisit)
        {
            return true;
        }

        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();

        // If it's a struct declaration that has matrices, remember it.  If a row-major instance
        // of it is created, it will have to be converted.
        if (type.isStructSpecifier() && type.isStructureContainingMatrices())
        {
            TStructure *structure = type.getStruct();
            ASSERT(structure);

            ASSERT(mStructMap.count(structure) == 0);

            StructConversionData structData;
            mStructMap[structure] = structData;

            return false;
        }

        // If it's an interface block, it may have to be converted if it contains any row-major
        // fields.
        if (type.isInterfaceBlock() && type.getInterfaceBlock()->containsMatrices())
        {
            TInterfaceBlock *block = type.getInterfaceBlock();
            ASSERT(block);
            bool isBlockRowMajor = type.getLayoutQualifier().matrixPacking == EmpRowMajor;

            const TFieldList &fields = block->fields();
            bool anyRowMajor         = false;

            for (const TField *field : fields)
            {
                if (IsFieldRowMajor(field, isBlockRowMajor))
                {
                    anyRowMajor = true;
                    break;
                }
            }

            if (!anyRowMajor)
            {
                return true;
            }

            convertInterfaceBlock(node);
        }

        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        const TVariable *symbolVariable = &symbol->variable();
        const TType *type               = symbol->getType();

        // If the symbol doesn't need to be replaced, there's nothing to do.
        if (mInterfaceBlockMap.count(symbolVariable) == 0)
        {
            return;
        }

        // Walk up the parent chain while the nodes are EOpIndex* (whether array indexing or struct
        // field selection).  At the end, we can determine if the expression is being read from or
        // written to (or both, if sent as an inout parameter to a function).
        bool isRead  = false;
        bool isWrite = false;
        prepareExpressionTransform(symbol, &isRead, &isWrite);

        ASSERT(isRead || isWrite);

        // TODO: Walk up the parent chain similarly again and construct the transformed
        // expression(s).

        // TODO: if reading, create a temporary of the expression, then transform it according to
        // the rules.  Replace this with the transformed temporary.
        // TODO: if writing, create a temporary of the expression that's being assigned, then
        // transform/assign it with the write rules

        // TODO: walk up the parents until you find the symbol that starts the chain of indexing.
        // This could be either array or struct indexing.  If only array indexing is seen, declare
        // and use copy functions to read from or write to this expression.
        //
        // If struct indexing is seen, follow it to see how it ends.  If it ends in another struct
        // that has matrices, again, declare and use copy functions for that struct.
        //
        // If it ends in a matrix, go through the possible matrix transformations.
    }

    TIntermSequence *getStructCopyFunctions()
    {
        TIntermSequence *functionDeclarations = new TIntermSequence;

        for (const auto &iter : mStructMap)
        {
            const StructConversionData structData = iter.second;
            if (structData.copyFrom)
            {
                functionDeclarations.push_back(structData.copyFrom);
            }
            if (structData.copyTo)
            {
                functionDeclarations.push_back(structData.copyTo);
            }
        }

        return functionDeclarations;
    }

  private:
    void convertInterfaceBlock(TIntermDeclaration *node)
    {
        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        TInterfaceBlock *block = type.getInterfaceBlock();
        ASSERT(block);

        bool isBlockRowMajor = type.getLayoutQualifier().matrixPacking == EmpRowMajor;

        TIntermSequence newDeclarations;

        TFieldList *newFields = new TFieldList;
        for (const TField *field : block->fields())
        {
            TField *newField = field;

            if (IsFieldRowMajor(field, isBlockRowMajor))
            {
                newField = convertField(field, &newDeclarations);
            }

            newFields.push_back(newField);
        }

        // Create a new interface block with these fields.
        TLayoutQualifier blockLayoutQualifier = type.getLayoutQualifier();
        blockLayoutQualifier.matrixPacking    = EmpColumnMajor;

        TInterfaceBlock *newInterfaceBlock =
            new TInterfaceBlock(mSymbolTable, block->name(), newFields, blockLayoutQualifier,
                                block->symbolType(), block->extension());

        // For every variable in the declaration, create a new one with the new type.
        // TODO: check to see if matrixPacking should be set in blockLayoutQualifier or in
        // the layout qualifier passed to the type constructor here!
        TType *newInterfaceBlockType =
            new TType(newInterfaceBlock, type.getQualifier(), type.getLayoutQualifier());

        TIntermDeclaration *newDeclaration = new TIntermDeclaration;
        for (TIntermNode *variableNode : sequence)
        {
            const TVariable *variable = &variableNode->getAsSymbolNode().variable();
            const TType *variableType = variable->type();

            const TType *newType = newInterfaceBlockType;
            if (variableType->isArray())
            {
                // TODO: at least temporarily, make the test have an array of some UBO, and multiple
                // instances in the same declaration to make sure these translate correctly.
                TType *newArrayType = new TType(*newType);
                CopyArraySizes(variableType, newArrayType);
                newType = newArrayType;
            }

            TVariable *newVariable = new TVariable(mSymbolTable, variable.name(), newType,
                                                   variable.symbolType(), variable.extension());

            newDeclaration->appendDeclarator(new TIntermSymbol(newVariable));

            mInterfaceBlockMap[variable] = newVariable;
        }

        newDeclarations.push_back(newDeclaration);

        // Replace the interface block definition with the new one, prepending any new struct
        // definitions.
        mMultiReplacements.emplace_back(getParentNode(), argument, newDeclarations);
    }

    void convertStruct(const TStructure *structure, TIntermSequence *declarations)
    {
        ASSERT(mStructMap.count(structure) != 0);
        StructConversionData *structData = &mStructMap[structure];

        if (structData->convertedStruct)
        {
            return;
        }

        TFieldList *newFields = new TFieldList;
        for (const TField *field : structure->fields())
        {
            newFields.push_back(convertField(field, &newDeclarations));
        }

        TStructure *newStruct =
            new TStructure(mSymbolTable, structure->name(), newFields, structure->symbolType());
        TType *newType = new TType(newStruct, true);
        TVariable *newStructVar =
            new TVariable(mSymbolTable, kEmptyImmutableString, newType, SymbolType::Empty);

        TIntermDeclaration *structDecl = new TIntermDeclaration;
        structDecl->appendDeclarator(new TIntermSymbol(newStructVar));

        declarations->push_back(structDecl);

        structData->convertedStruct = newStruct;
    }

    TField *convertField(const TField *field, TIntermDeclaration *declarations)
    {
        TField *newField = field;

        const TType *fieldType = field->type();
        TType *newType         = nullptr;

        if (fieldType->isStructureContainingMatrices())
        {
            // If the field is a struct instance, convert the struct and replace the field
            // with an instance of the new struct.
            const TStructure *fieldTypeStruct = fieldType->getStruct();
            convertStruct(fieldTypeStruct, declarations);

            StructConversionData &structData = mStructMap[fieldTypeStruct];
            newType                          = new TType(structData.convertedStruct, false);
        }
        else if (fieldType->isMatrix())
        {
            // If the field is a matrix, transpose the matrix and replace the field with
            // that, removing the matrix packing qualifier.
            newType = TransposeMatrix(*fieldType);
        }

        if (newType)
        {
            CopyArraySizes(fieldType, newType);
            newField = new TField(newType, field.name(), field.line(), field.symbolType());
        }

        return newField;
    }

    void prepareExpressionTransform(TIntermSymbol *symbol, bool *isReadOut, bool *isWriteOut)
    {
        uint32_t accessorIndex = 0;

        while (IsIndexNode(getAncestorNode(accessorIndex)))
        {
            ++accessorIndex;
        }

        // If passing to a function, look at whether the parameter is in, out or inout.
        TIntermNode *expression = accessorIndex == 0 ? symbol : getAncestorNode(accessorIndex - 1);
        TIntermNode *accessor   = getAncestorNode(accessorIndex);
        TIntermAggregate *functionCall = accessor->getAsAggregate();

        if (functionCall)
        {
            TIntermSequence *arguments = functionCall->getSequence();
            for (size_t argIndex = 0; argIndex < arguments->size(); ++argIndex)
            {
                if ((*arguments)[argIndex] == expression)
                {
                    const TFunction *function = functionCall->getFunction();
                    const TVariable *param    = function->getParam(argIndex);
                    TQualifier qualifier      = param->getType().getQualifier();

                    *isReadOut  = qualifier != EvqOut;
                    *isWriteOut = qualifier == EvqOut || qualifier == EvqInOut;
                    break;
                }
            }
            return;
        }

        TIntermBinary *assignment = accessor->getAsBinaryNode();
        if (assignment && IsAssignment(assignment->getOp))
        {
            // If it's on the right of assignment, it's being read from.
            *isReadOut = assignment->getRight() == expression;
            // If expression is on the left of assignment, it's being written to.
            // TODO: if it's M += b, the same write transformations apply, right?
            *isWriteOut = assignment->getLeft() == expression;
            return;
        }

        // Any other usage is a read.
        *isReadOut  = true;
        *isWriteOut = false;
    }

    // A map from structures with matrices to their converted version.
    std::unordered_map<const TStructure *, StructConversionData> mStructMap;
    // A map from interface block instances with row-major matrices to their converted variable.
    std::unordered_map<const TVariable *, TVariable *> mInterfaceBlockMap;
};

}  // anonymous namespace

void RewriteRowMajorMatrices(TIntermBlock *root, TSymbolTable *symbolTable)
{
    RewriteRowMajorMatricesTraverser traverser(symbolTable);
    root->traverse(&traverser);
    traverser.updateTree();

    size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    root->insertChildNodes(firstFunctionIndex, *traverser.getStructCopyFunctions());
}
}  // namespace sh
