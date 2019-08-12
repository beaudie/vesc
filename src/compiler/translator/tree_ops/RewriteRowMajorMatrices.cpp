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
bool IsIndexNode(TIntermNode *node, TIntermNode *child)
{
    if (node->getAsSwizzleNode())
    {
        return true;
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode == nullptr || child != binaryNode->getLeft())
    {
        return false;
    }

    TOperator op = binaryNode->getOp();

    return op == EOpIndexDirect || op == EOpIndexDirectInterfaceBlock ||
           op == EOpIndexDirectStruct || op == EOpIndexIndirect;
}

TOperator GetIndex(TIntermNode *node, TIntermSequence *indices)
{
    // Swizzle nodes are converted EOpIndexDirect for simplicity, with one index per swizzle
    // channel.
    TIntermSwizzle *asSwizzle = node->getAsSwizzleNode();
    if (asSwizzle)
    {
        for (int channel : asSwizzle->getSwizzleOffsets())
        {
            indices->push_back(CreateIndexNode(channel));
        }
        return EOpIndexDirect;
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    ASSERT(binaryNode);

    TOperator op = binaryNode->getOp();
    ASSERT(op == EOpIndexDirect || op == EOpIndexDirectInterfaceBlock ||
           op == EOpIndexDirectStruct || op == EOpIndexIndirect);

    indices->push_back(binaryNode->getRight());
    return op;
}

TIntermTyped *ReplicateIndexNode(TIntermNode *node, TIntermTyped *lhs)
{
    TIntermSwizzle *asSwizzle = node->getAsSwizzleNode();
    if (asSwizzle)
    {
        return new TIntermSwizzle(lhs, asSwizzle->getSwizzleOffsets());
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    ASSERT(binaryNode);

    return new TIntermBinary(binaryNode->getOp(), lhs, binaryNode->getRight());
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
//            (matrix reads through struct are transformed similarly to M)
//            (matrix arrays are similarly transformed)
//
// 4. When written to, the following transformations are applied:
//
//      M = exp       -> Mt = transpose(exp)
//      M[c] = exp    -> temp = exp
//                       Mt[0][c] = temp[0]
//                       Mt[1][c] = temp[1]
//                       ...
//                       Mt[N-1][c] = temp[N-1]
//      M[c][r] = exp -> Mt[r][c] = exp
//      M[c].yz = exp -> temp = exp
//                       Mt[1][c] = temp[0]
//                       Mt[2][c] = temp[1]
//      s = exp       -> st = copy_S_to_St(exp)
//      (matrix writes through struct are transformed similarly to M)
//      (matrix arrays are similarly transformed)
//
// 5. If any of the above is passed to an `inout` parameter, both transformations are applied:
//
//            f(M[c]) -> temp = gvecN(Mt[0][c], Mt[1][c], ..., Mt[N-1][c])
//                       f(temp)
//                       Mt[0][c] = temp[0]
//                       Mt[1][c] = temp[1]
//                       ...
//                       Mt[N-1][c] = temp[N-1]
//
//               f(s) -> temp = copy_St_to_S(st)
//                       f(temp)
//                       st = copy_S_to_St(temp)
//
//    If passed to an `out` parameter, the `temp` parameter is simply not initialized.
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

        transformExpression(symbol, isRead, isWrite, &prependStatements, &appendStatements);

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

    void determineAccess(TIntermNode *expression,
                         TIntermNode *accessor bool *isReadOut,
                         bool *isWriteOut)
    {
        // If passing to a function, look at whether the parameter is in, out or inout.
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
        if (assignment && IsAssignment(assignment->getOp()))
        {
            // If it's on the right of assignment, it's being read from.
            *isReadOut = assignment->getRight() == expression;
            // If expression is on the left of assignment, it's being written to.
            *isWriteOut = assignment->getLeft() == expression;
            return;
        }

        // Any other usage is a read.
        *isReadOut  = true;
        *isWriteOut = false;
    }

    void transformExpression(TIntermSymbol *symbol)
    {
        // Walk up the parent chain while the nodes are EOpIndex* (whether array indexing or struct
        // field selection) or swizzle and construct the replacement expression.  This traversal can
        // lead to one of the following possibilities:
        //
        // - a.b[N].etc.s (struct): copy function should be declared and used,
        // - a.b[N].etc.M (matrix): transpose() should be used,
        // - a.b[N].etc.M[c] (a column): each element in column needs to be handled separately,
        // - a.b[N].etc.M[c].yz (multiple elements): similar to whole column, but a subset of
        //   elements,
        // - a.b[N].etc.M[c][r] (an element): single element to handle.
        // - a.b[N].etc.x (not struct or matrix): not modified
        //
        // primaryIndex will contain c, if any.  secondaryIndices will contain {0, ..., R-1}
        // (if no [r] or swizzle), {r} (if [r]), or {1, 2} (corresponding to .yz) if any.
        //
        // In all cases, the base symbol is replaced.  |baseExpression| will contain everything up
        // to (and not including) the last index/swizzle operations, i.e. a.b[N].etc.s/M/x.
        //
        TIntermTyped *baseExpression = new TIntermSymbol(mInterfaceBlockMap[symbol]);
        bool isStruct                = false;
        bool isMatrix                = false;
        const TStructure *structure  = nullptr;

        TIntermNode *primaryIndex = nullptr;
        TIntermSequence secondaryIndices;

        uint32_t accessorIndex        = 0;
        TIntermNode *previousAncestor = symbol;
        while (IsIndexNode(getAncestorNode(accessorIndex), previousAncestor))
        {
            TIntermTyped *ancestor = getAncestorNode(accessorIndex)->getAsTyped();
            ASSERT(ancestor);

            const TType &ancestorType = ancestor->getType();
            bool ancestorIsStruct     = ancestorType.isStructSpecifier();
            bool ancestorIsMatrix     = ancestorType.isMatrix();

            TIntermSequence indices;
            TOperation op = GetIndex(ancestor, &indices);

            // TODO: add a test with a struct that's row-major but has a vec4 in it.  Make sure the
            // vec4 is not transformed here.

            // If it's a direct index in a matrix, it's the primary index.
            bool isMatrixPrimarySubscript =
                op == EOpIndexDirect && !ancestorType.isArray() && ancestorIsMatrix;
            ASSERT(!isMatrixPrimarySubscript ||
                   (primaryIndex == nullptr && secondaryIndices.empty()));
            // If primary index is seen and the ancestor is still an index, it must be a direct
            // index as the secondary one.  Note that if primaryIndex is set, there can only ever be
            // one more parent of interest, and that's subscripting the second dimension.
            bool isMatrixSecondarySubscript = primaryIndex != nullptr;
            ASSERT(!isMatrixSecondarySubscript ||
                   (op == EOpIndexDirect && !ancestorType.isArray()));

            if (isMatrixPrimarySubscript)
            {
                ASSERT(indices.size() == 1);
                primaryIndex = indices.front();

                // Default the secondary indices to include every row.  If there's a secondary
                // subscript provided, it will override this.
                int rows = ancestorType.getRows();
                for (int r = 0; r < rows; ++r)
                {
                    secondaryIndices.push_back(CreateIndexNode(r));
                }
            }
            else if (isMatrixSecondarySubscript)
            {
                secondaryIndices = indices;
            }
            else
            {
                // Replicate the expression otherwise.
                baseExpression = ReplicateIndexNode(ancestor, baseExpression);

                isMatrix  = ancestorIsMatrix;
                isStruct  = ancestorIsStruct;
                structure = ancestorType.getStruct();
            }

            previousAncestor = ancestor;
            ++accessorIndex;
        }

        TIntermNode *originalExpression =
            accessorIndex == 0 ? symbol : getAncestorNode(accessorIndex - 1);
        TIntermNode *accessor = getAncestorNode(accessorIndex);

        // TODO: these asserts should hold.  In that case, no need to take that from the loop and
        // can be retrieved here.  If they don't hold, need to pass isMatrix/isStruct to the
        // transform helper functions.
        ASSERT(isMatrix == baseExpression->getType().isMatrix());
        ASSERT(isStruct == baseExpression->getType().isStructSpecifier());

        // If it's neither a struct or matrix, no modification is necessary.  In this case,
        // |baseExpression| contains all of the original expression.
        bool requiresTransformation = isMatrix || isStruct;

        // If it's a struct that doesn't have matrices, again there's no transformation necessary.
        if (requiresTransformation && isStruct)
        {
            if (!baseExpression->getType().isStructureContainingMatrices())
            {
                requiresTransformation = false;
            }
        }

        if (!requiresTransformation)
        {
            ASSERT(primaryIndex == nullptr);
            queueReplacementWithParent(accessor, originalExpression, baseExpression,
                                       OriginalNode::IS_DROPPED);
            return;
        }

        ASSERT(isMatrix || primaryIndex == nullptr);

        // At the end, we can determine if the expression is being read from or written to (or both,
        // if sent as an inout parameter to a function).  For the sake of the transformation, the
        // left-hand side of operations like += can be treated as "written to", without necessarily
        // "read from".
        bool isRead  = false;
        bool isWrite = false;

        determineAccess(originalExpression, accessor, &isRead, &isWrite);

        ASSERT(isRead || isWrite);

        // In some cases, it is necessary to prepend or append statements.  Those are captured in
        // |prependStatements| and |appendStatements|.
        TIntermSequence prependStatements;
        TIntermSequence appendStatements;

        TIntermTyped *readExpression = nullptr;
        if (isRead)
        {
            readExpression =
                transformReadExpression(baseExpression, primaryIndex, &secondaryIndices, structure);

            // If both read from and written to (i.e. passed to inout parameter), store the
            // expression in a temp variable and pass that to the function.
            if (isWrite)
            {
                TVariable *temp = CreateTempVariable(mSymbolTable, &readExpression->getType());
                TIntermDeclaration *tempDecl = CreateTempInitDeclarationNode(temp, *readExpression);
                prependStatements.push_back(tempDecl);

                readExpression = new TIntermSymbol(temp);
            }

            // Replace the original expression with the transformed one.  Read transformations
            // always generate a single expression that can be used in place of the original (as
            // oppposed to write transformations that can generate multiple statements).
            queueReplacementWithParent(accessor, originalExpression, readExpression,
                                       OriginalNode::IS_DROPPED);
        }

        TIntermSequence *writeStatements = &appendStatements;
        bool isWriteWithAssign           = false;

        if (isWrite)
        {
            TIntermTyped *writeExpression = nullptr;
            TIntermTyped *valueExpression = readExpression;

            if (!valueExpression)
            {
                // If there's already a read expression, this was an inout parameter and
                // |valueExpression| will contain the temp variable that was passed to the function
                // instead.
                //
                // If not, then the modification is either through being passed as an out parameter
                // to a function, or an assignment.  In the former case, create a temp variable to
                // be passed to the function.  In the latter case, create a temp variable that holds
                // the right hand side expression.
                //
                // In either case, use that temp value as the value to assign to |writeExpression|.

                TVariable *temp = CreateTempVariable(mSymbolTable, &originalExpression->getType());
                TIntermDeclaration *tempDecl = nullptr;

                TIntermNode *assignment = accessor->getAsBinaryNode();
                if (assignment)
                {
                    ASSERT(IsAssignment(assignment->getOp()));
                    tempDecl = CreateTempInitDeclarationNode(temp, *assignment->getRight());

                    // Replace the whole assingment expression with the temp value.  If the result
                    // of the assignment is used, it will use temp.  For example, this transforms:
                    //
                    //     if ((M = exp) == X)
                    //     {
                    //         // use M
                    //     }
                    //
                    // to:
                    //
                    //     temp = exp;
                    //     M = transform(temp);
                    //     if (temp == X)
                    //     {
                    //         // use M
                    //     }
                    //
                    // Note that in this case the assignment to M must be prepended in the parent
                    // block.  In contrast, when sent to a function, the assignment to M should be
                    // done after the current function call is done.
                    //
                    writeStatements = &prependStatements;
                }
                else
                {
                    tempDecl = CreateTempDeclarationNode(temp);
                }
                prependStatements.push_back(tempDecl);

                valueExpression = new TIntermSymbol(temp);
                queueReplacementWithParent(accessor, originalExpression, valueExpression,
                                           OriginalNode::IS_DROPPED);
            }

            transformWriteExpression(baseExpression, primaryIndex, &secondaryIndices, structure,
                                     valueExpression, writeStatements);
        }

        insertStatementsInParentBlock(prependStatements, appendStatements);
    }

    TIntermTyped *transformReadExpression(TIntermTyped *baseExpression,
                                          TIntermNode *primaryIndex,
                                          TIntermSequence *secondaryIndices,
                                          const TStructure *structure)
    {
        if (structure)
        {
            ASSERT(primaryIndex == nullptr && secondaryIndices->empty());
            ASSERT(mStructMap.count(structure) != 0);
            ASSERT(mStructMap[structure].convertedStruct != nullptr);

            // Declare copy from original to converted struct function (if not already).
            declareStructCopyFrom(structure);

            // The result is simply a call to this function with the base expression.
            const TFunction *copyFrom = &mStructMap[structure].copyFrom->getFunction();
            return TIntermAggregate::CreateFunctionCall(*copyFrom,
                                                        new TIntermSequence({baseExpression}));
        }

        // If not indexed, the result is transpose(exp)
        if (primaryIndex == nullptr)
        {
            ASSERT(secondaryIndices->empty());

            return CreateBuiltInFunctionCallNode("transpose", baseExpression);
        }

        // If indexed the result is a vector (or just one element) where the primary and secondary
        // indices are swapped.
        ASSERT(!secondaryIndices->empty());
        TType *vecType =
            new TType(baseExpression->getType().getBasicType(), secondaryIndices->size());
        // TODO: from here
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
