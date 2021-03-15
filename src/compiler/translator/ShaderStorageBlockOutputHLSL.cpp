//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockOutputHLSL: A traverser to translate a ssbo_access_chain to an offset of
// RWByteAddressBuffer.
//     //EOpIndexDirectInterfaceBlock
//     ssbo_variable :=
//       | the name of the SSBO
//       | the name of a variable in an SSBO backed interface block

//     // EOpIndexInDirect
//     // EOpIndexDirect
//     ssbo_array_indexing := ssbo_access_chain[expr_no_ssbo]

//     // EOpIndexDirectStruct
//     ssbo_structure_access := ssbo_access_chain.identifier

//     ssbo_access_chain :=
//       | ssbo_variable
//       | ssbo_array_indexing
//       | ssbo_structure_access
//

#include "compiler/translator/ShaderStorageBlockOutputHLSL.h"

#include "compiler/translator/ResourcesHLSL.h"
#include "compiler/translator/blocklayoutHLSL.h"
#include "compiler/translator/tree_util\IntermNode_util.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

void GetBlockLayoutInfo(TIntermTyped *node,
                        bool rowMajorAlreadyAssigned,
                        TLayoutBlockStorage *storage,
                        bool *rowMajor)
{
    TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
    if (swizzleNode)
    {
        return GetBlockLayoutInfo(swizzleNode->getOperand(), rowMajorAlreadyAssigned, storage,
                                  rowMajor);
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirectInterfaceBlock:
            {
                // The column_major/row_major qualifier of a field member overrides the interface
                // block's row_major/column_major. So we can assign rowMajor here and don't need to
                // assign it again. But we still need to call recursively to get the storage's
                // value.
                const TType &type = node->getType();
                *rowMajor         = type.getLayoutQualifier().matrixPacking == EmpRowMajor;
                return GetBlockLayoutInfo(binaryNode->getLeft(), true, storage, rowMajor);
            }
            case EOpIndexIndirect:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
                return GetBlockLayoutInfo(binaryNode->getLeft(), rowMajorAlreadyAssigned, storage,
                                          rowMajor);
            default:
                UNREACHABLE();
                return;
        }
    }

    const TType &type = node->getType();
    ASSERT(type.getQualifier() == EvqBuffer);
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    ASSERT(interfaceBlock);
    *storage = interfaceBlock->blockStorage();
    // If the block doesn't have an instance name, rowMajorAlreadyAssigned will be false. In
    // this situation, we still need to set rowMajor's value.
    if (!rowMajorAlreadyAssigned)
    {
        *rowMajor = type.getLayoutQualifier().matrixPacking == EmpRowMajor;
    }
}

// It's possible that the current type has lost the original layout information. So we should pass
// the right layout information to GetBlockMemberInfoByType.
const BlockMemberInfo GetBlockMemberInfoByType(const TType &type,
                                               TLayoutBlockStorage storage,
                                               bool rowMajor)
{
    sh::Std140BlockEncoder std140Encoder;
    sh::Std430BlockEncoder std430Encoder;
    sh::HLSLBlockEncoder hlslEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED, false);
    sh::BlockLayoutEncoder *encoder = nullptr;

    if (storage == EbsStd140)
    {
        encoder = &std140Encoder;
    }
    else if (storage == EbsStd430)
    {
        encoder = &std430Encoder;
    }
    else
    {
        encoder = &hlslEncoder;
    }

    std::vector<unsigned int> arraySizes;
    const TSpan<const unsigned int> &typeArraySizes = type.getArraySizes();
    if (!typeArraySizes.empty())
    {
        arraySizes.assign(typeArraySizes.begin(), typeArraySizes.end());
    }
    return encoder->encodeType(GLVariableType(type), arraySizes, rowMajor);
}

int GetArrayStrideByType(const TType &type,
                         TLayoutBlockStorage storage,
                         bool rowMajor)
{
    sh::Std140BlockEncoder std140Encoder;
    sh::Std430BlockEncoder std430Encoder;
    sh::HLSLBlockEncoder hlslEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED, false);
    sh::BlockLayoutEncoder *encoder = nullptr;

    if (storage == EbsStd140)
    {
        encoder = &std140Encoder;
    }
    else if (storage == EbsStd430)
    {
        encoder = &std430Encoder;
    }
    else
    {
        encoder = &hlslEncoder;
    }

    std::vector<unsigned int> arraySizes;
    const TSpan<const unsigned int> &typeArraySizes = type.getArraySizes();
    if (!typeArraySizes.empty())
    {
        arraySizes.assign(typeArraySizes.begin(), typeArraySizes.end());
    }
    const TStructure* tStruct = type.getStruct();
    if (tStruct) {
        int arrayStride = 0;
        for (TField* field : tStruct->fields()) {
            arrayStride += encoder->encodeType(GLVariableType(*field->type()), arraySizes, rowMajor).arrayStride;
        }
        return arrayStride;
    } else {
       return encoder->encodeType(GLVariableType(type), arraySizes, rowMajor).arrayStride;
    }
}

const TField *GetFieldMemberInShaderStorageBlock(const TInterfaceBlock *interfaceBlock,
                                                 const ImmutableString &variableName)
{
    for (const TField *field : interfaceBlock->fields())
    {
        if (field->name() == variableName)
        {
            return field;
        }
    }
    return nullptr;
}

const InterfaceBlock *FindInterfaceBlock(const TInterfaceBlock *needle,
                                         const std::vector<InterfaceBlock> &haystack)
{
    for (const InterfaceBlock &block : haystack)
    {
        if (strcmp(block.name.c_str(), needle->name().data()) == 0)
        {
            ASSERT(block.fields.size() == needle->fields().size());
            return &block;
        }
    }

    UNREACHABLE();
    return nullptr;
}

std::string StripArrayIndices(const std::string &nameIn)
{
    std::string name = nameIn;
    size_t pos       = name.find('[');
    while (pos != std::string::npos)
    {
        size_t closePos = name.find(']', pos);
        ASSERT(closePos != std::string::npos);
        name.erase(pos, closePos - pos + 1);
        pos = name.find('[', pos);
    }
    ASSERT(name.find(']') == std::string::npos);
    return name;
}

// Does not include any array indices.
void MapVariableToField(const ShaderVariable &variable,
                        const TField *field,
                        std::string currentName,
                        ShaderVarToFieldMap *shaderVarToFieldMap)
{
    ASSERT((field->type()->getStruct() == nullptr) == variable.fields.empty());
    (*shaderVarToFieldMap)[currentName] = field;

    if (!variable.fields.empty())
    {
        const TStructure *subStruct = field->type()->getStruct();
        ASSERT(variable.fields.size() == subStruct->fields().size());

        for (size_t index = 0; index < variable.fields.size(); ++index)
        {
            const TField *subField            = subStruct->fields()[index];
            const ShaderVariable &subVariable = variable.fields[index];
            std::string subName               = currentName + "." + subVariable.name;
            MapVariableToField(subVariable, subField, subName, shaderVarToFieldMap);
        }
    }
}

class BlockInfoVisitor final : public BlockEncoderVisitor
{
  public:
    BlockInfoVisitor(const std::string &prefix,
                     TLayoutBlockStorage storage,
                     const ShaderVarToFieldMap &shaderVarToFieldMap,
                     BlockMemberInfoMap *blockInfoOut)
        : BlockEncoderVisitor(prefix, "", getEncoder(storage)),
          mShaderVarToFieldMap(shaderVarToFieldMap),
          mBlockInfoOut(blockInfoOut),
          mHLSLEncoder(HLSLBlockEncoder::ENCODE_PACKED, false),
          mStorage(storage)
    {}

    BlockLayoutEncoder *getEncoder(TLayoutBlockStorage storage)
    {
        switch (storage)
        {
            case EbsStd140:
                return &mStd140Encoder;
            case EbsStd430:
                return &mStd430Encoder;
            default:
                return &mHLSLEncoder;
        }
    }

    void enterStructAccess(const ShaderVariable &structVar, bool isRowMajor) override
    {
        BlockEncoderVisitor::enterStructAccess(structVar, isRowMajor);

        std::string variableName = StripArrayIndices(collapseNameStack());

        // Remove the trailing "."
        variableName.pop_back();

        BlockInfoVisitor childVisitor(variableName, mStorage, mShaderVarToFieldMap, mBlockInfoOut);
        childVisitor.getEncoder(mStorage)->enterAggregateType(structVar);
        TraverseShaderVariables(structVar.fields, isRowMajor, &childVisitor);
        childVisitor.getEncoder(mStorage)->exitAggregateType(structVar);

        int offset      = static_cast<int>(getEncoder(mStorage)->getCurrentOffset());
        int arrayStride = static_cast<int>(childVisitor.getEncoder(mStorage)->getCurrentOffset());

        auto iter = mShaderVarToFieldMap.find(variableName);
        if (iter == mShaderVarToFieldMap.end())
            return;

        const TField *structField = iter->second;
        if (mBlockInfoOut->count(structField) == 0)
        {
            mBlockInfoOut->emplace(structField, BlockMemberInfo(offset, arrayStride, -1, false));
        }
    }

    void encodeVariable(const ShaderVariable &variable,
                        const BlockMemberInfo &variableInfo,
                        const std::string &name,
                        const std::string &mappedName) override
    {
        auto iter = mShaderVarToFieldMap.find(StripArrayIndices(name));
        if (iter == mShaderVarToFieldMap.end())
            return;

        const TField *field = iter->second;
        if (mBlockInfoOut->count(field) == 0)
        {
            mBlockInfoOut->emplace(field, variableInfo);
        }
    }

  private:
    const ShaderVarToFieldMap &mShaderVarToFieldMap;
    BlockMemberInfoMap *mBlockInfoOut;
    Std140BlockEncoder mStd140Encoder;
    Std430BlockEncoder mStd430Encoder;
    HLSLBlockEncoder mHLSLEncoder;
    TLayoutBlockStorage mStorage;
};

void GetShaderStorageBlockMembersInfo(const TInterfaceBlock *interfaceBlock,
                                      const std::vector<InterfaceBlock> &shaderStorageBlocks,
                                      BlockMemberInfoMap *blockInfoOut)
{
    // Find the sh::InterfaceBlock.
    const InterfaceBlock *block = FindInterfaceBlock(interfaceBlock, shaderStorageBlocks);
    ASSERT(block);

    // Map ShaderVariable to TField.
    ShaderVarToFieldMap shaderVarToFieldMap;
    for (size_t index = 0; index < block->fields.size(); ++index)
    {
        const TField *field            = interfaceBlock->fields()[index];
        const ShaderVariable &variable = block->fields[index];
        MapVariableToField(variable, field, variable.name, &shaderVarToFieldMap);
    }

    BlockInfoVisitor visitor("", interfaceBlock->blockStorage(), shaderVarToFieldMap, blockInfoOut);
    TraverseShaderVariables(block->fields, false, &visitor);
}

bool IsInArrayOfArraysChain(TIntermTyped *node)
{
    if (node->getType().isArrayOfArrays())
        return true;
    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        if (binaryNode->getLeft()->getType().isArrayOfArrays())
            return true;
    }

    return false;
}
}  // anonymous namespace

ShaderStorageBlockOutputHLSL::ShaderStorageBlockOutputHLSL(
    OutputHLSL *outputHLSL,
    TSymbolTable *symbolTable,
    ResourcesHLSL *resourcesHLSL,
    const std::vector<InterfaceBlock> &shaderStorageBlocks)
    : TIntermTraverser(true, true, true, symbolTable),
      mMatrixStride(0),
      mRowMajor(false),
      mLocationAsTheLastArgument(false),
      mOutputHLSL(outputHLSL),
      mResourcesHLSL(resourcesHLSL),
      mShaderStorageBlocks(shaderStorageBlocks)
{
    mSSBOFunctionHLSL = new ShaderStorageBlockFunctionHLSL;
}

ShaderStorageBlockOutputHLSL::~ShaderStorageBlockOutputHLSL()
{
    SafeDelete(mSSBOFunctionHLSL);
}

void ShaderStorageBlockOutputHLSL::outputStoreFunctionCall(TIntermBinary *node)
{
    TInfoSinkBase &out         = mOutputHLSL->getInfoSink();
    mLocationAsTheLastArgument = false;
    traverseSSBOAccess(node->getLeft(), SSBOMethod::STORE);
    out << ", ";
    if (IsInShaderStorageBlock(node->getRight()))
    {
        outputLoadFunctionCall(node->getRight());
    }
    else
    {
        node->getRight()->traverse(mOutputHLSL);
    }
    out << ")";
}

void ShaderStorageBlockOutputHLSL::outputLoadFunctionCall(TIntermTyped *node)
{
    TInfoSinkBase &out         = mOutputHLSL->getInfoSink();
    mLocationAsTheLastArgument = true;
    traverseSSBOAccess(node, SSBOMethod::LOAD );
    out << ")";
}

void ShaderStorageBlockOutputHLSL::outputLengthFunctionCall(TIntermTyped *node)
{
    TInfoSinkBase &out         = mOutputHLSL->getInfoSink();
    mLocationAsTheLastArgument = true;
    traverseSSBOAccess(node, SSBOMethod::LENGTH);
    out << ")";
}

void ShaderStorageBlockOutputHLSL::outputAtomicMemoryFunctionCallPrefix(TIntermTyped *node,
                                                                        TOperator op)
{
    mLocationAsTheLastArgument = false;

    switch (op)
    {
        case EOpAtomicAdd:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_ADD);
            break;
        case EOpAtomicMin:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_MIN);
            break;
        case EOpAtomicMax:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_MAX);
            break;
        case EOpAtomicAnd:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_AND);
            break;
        case EOpAtomicOr:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_OR);
            break;
        case EOpAtomicXor:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_XOR);
            break;
        case EOpAtomicExchange:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_EXCHANGE);
            break;
        case EOpAtomicCompSwap:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_COMPSWAP);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// Note that we must calculate the matrix stride here instead of ShaderStorageBlockFunctionHLSL.
// It's because that if the current node's type is a vector which comes from a matrix, we will
// lose the matrix type info once we enter ShaderStorageBlockFunctionHLSL.
void ShaderStorageBlockOutputHLSL::setMatrixStride(TIntermTyped *node,
                                                   TLayoutBlockStorage storage,
                                                   bool rowMajor)
{
    if (node->getType().isMatrix())
    {
        mMatrixStride = GetBlockMemberInfoByType(node->getType(), storage, rowMajor).matrixStride;
        mRowMajor     = rowMajor;
        return;
    }

    if (node->getType().isVector())
    {
        TIntermBinary *binaryNode = node->getAsBinaryNode();
        if (binaryNode)
        {
            return setMatrixStride(binaryNode->getLeft(), storage, rowMajor);
        }
        else
        {
            TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
            if (swizzleNode)
            {
                return setMatrixStride(swizzleNode->getOperand(), storage, rowMajor);
            }
        }
    }
}

void ShaderStorageBlockOutputHLSL::traverseSSBOAccess(TIntermTyped *node, SSBOMethod method)
{
    mMatrixStride = 0;
    mRowMajor     = false;

    // Note that we don't have correct BlockMemberInfo from mBlockMemberInfoMap at the current
    // point. But we must use those information to generate the right function name. So here we have
    // to calculate them again.
    TLayoutBlockStorage storage;
    bool rowMajor;
    GetBlockLayoutInfo(node, false, &storage, &rowMajor);
    int unsizedArrayStride = 0;
    if (node->getType().isUnsizedArray())
    {
        unsizedArrayStride = GetArrayStrideByType(node->getType(), storage, rowMajor);
    }
    setMatrixStride(node, storage, rowMajor);

    const TString &functionName = mSSBOFunctionHLSL->registerShaderStorageBlockFunction(
        node->getType(), method, storage, mRowMajor, mMatrixStride, unsizedArrayStride,
        node->getAsSwizzleNode());
    TIntermSymbol *base  = findBase(node);
    referenceBufferSymbol(base);
    TIntermNode *loc   = createByteAddressExpression(node, method);
    TInfoSinkBase &out = mOutputHLSL->getInfoSink();
    out << functionName;
    out << "(";
    writeBufferSymbol(out, node);
    out << ", ";
    if (loc)
    {
        loc->traverse(mOutputHLSL);
    }
    else
    {
        out << "0";
    }
}

void ShaderStorageBlockOutputHLSL::writeShaderStorageBlocksHeader(TInfoSinkBase &out) const
{
    out << mResourcesHLSL->shaderStorageBlocksHeader(mReferencedShaderStorageBlocks);
    mSSBOFunctionHLSL->shaderStorageBlockFunctionHeader(out);
}

// Check if the current node is the end of the SSBO access chain. If true, we should output ')' for
// Load method.
bool ShaderStorageBlockOutputHLSL::isEndOfSSBOAccessChain()
{
    TIntermNode *parent = getParentNode();
    if (parent)
    {
        TIntermBinary *parentBinary = parent->getAsBinaryNode();
        if (parentBinary != nullptr)
        {
            switch (parentBinary->getOp())
            {
                case EOpIndexDirectStruct:
                case EOpIndexDirect:
                case EOpIndexIndirect:
                {
                    return false;
                }
                default:
                    return true;
            }
        }

        const TIntermSwizzle *parentSwizzle = parent->getAsSwizzleNode();
        if (parentSwizzle)
        {
            return false;
        }
    }
    return true;
}

void ShaderStorageBlockOutputHLSL::referenceBufferSymbol(TIntermSymbol *node)
{
    const TVariable &variable = node->variable();

    const TType &variableType             = variable.getType();
    const TInterfaceBlock *interfaceBlock = variableType.getInterfaceBlock();
    ASSERT(interfaceBlock);
    if (mReferencedShaderStorageBlocks.count(interfaceBlock->uniqueId().get()) == 0)
    {
        const TVariable *instanceVariable = nullptr;
        if (variableType.isInterfaceBlock())
        {
            instanceVariable = &variable;
        }
        mReferencedShaderStorageBlocks[interfaceBlock->uniqueId().get()] =
            new TReferencedBlock(interfaceBlock, instanceVariable);
        GetShaderStorageBlockMembersInfo(interfaceBlock, mShaderStorageBlocks,
                                            &mBlockMemberInfoMap);
    }
}

void ShaderStorageBlockOutputHLSL::visitSymbol(TIntermSymbol *node)
{
    TInfoSinkBase &out        = mOutputHLSL->getInfoSink();
    const TVariable &variable = node->variable();
    TQualifier qualifier      = variable.getType().getQualifier();

    if (qualifier == EvqBuffer)
    {
        const TType &variableType             = variable.getType();
        const TInterfaceBlock *interfaceBlock = variableType.getInterfaceBlock();
        ASSERT(interfaceBlock);
        if (mReferencedShaderStorageBlocks.count(interfaceBlock->uniqueId().get()) == 0)
        {
            const TVariable *instanceVariable = nullptr;
            if (variableType.isInterfaceBlock())
            {
                instanceVariable = &variable;
            }
            mReferencedShaderStorageBlocks[interfaceBlock->uniqueId().get()] =
                new TReferencedBlock(interfaceBlock, instanceVariable);
            GetShaderStorageBlockMembersInfo(interfaceBlock, mShaderStorageBlocks,
                                             &mBlockMemberInfoMap);
        }
        if (variableType.isInterfaceBlock())
        {
            out << DecorateVariableIfNeeded(variable);
        }
        else
        {
            out << Decorate(interfaceBlock->name());
            out << ", ";

            const TField *field =
                GetFieldMemberInShaderStorageBlock(interfaceBlock, variable.name());
            writeDotOperatorOutput(out, field);
        }
    }
    else
    {
        return mOutputHLSL->visitSymbol(node);
    }
}

void ShaderStorageBlockOutputHLSL::visitConstantUnion(TIntermConstantUnion *node)
{
    mOutputHLSL->visitConstantUnion(node);
}

bool ShaderStorageBlockOutputHLSL::visitAggregate(Visit visit, TIntermAggregate *node)
{
    return mOutputHLSL->visitAggregate(visit, node);
}

bool ShaderStorageBlockOutputHLSL::visitTernary(Visit visit, TIntermTernary *node)
{
    return mOutputHLSL->visitTernary(visit, node);
}

bool ShaderStorageBlockOutputHLSL::visitUnary(Visit visit, TIntermUnary *node)
{
    return mOutputHLSL->visitUnary(visit, node);
}

bool ShaderStorageBlockOutputHLSL::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    if (visit == PostVisit)
    {
        if (!IsInShaderStorageBlock(node))
        {
            return mOutputHLSL->visitSwizzle(visit, node);
        }
        TInfoSinkBase &out = mOutputHLSL->getInfoSink();
        // TODO(jiajia.qin@intel.com): add swizzle process if the swizzle node is not the last node
        // of ssbo access chain. Such as, data.xy[0]
        if (mLocationAsTheLastArgument && isEndOfSSBOAccessChain())
        {
            out << ")";
        }
    }
    return true;
}

bool ShaderStorageBlockOutputHLSL::visitBinary(Visit visit, TIntermBinary *node)
{
    TInfoSinkBase &out = mOutputHLSL->getInfoSink();

    switch (node->getOp())
    {
        case EOpIndexDirect:
        {
            if (!IsInShaderStorageBlock(node->getLeft()))
            {
                return mOutputHLSL->visitBinary(visit, node);
            }

            const TType &leftType = node->getLeft()->getType();
            if (leftType.isInterfaceBlock())
            {
                if (visit == PreVisit)
                {
                    ASSERT(leftType.getQualifier() == EvqBuffer);
                    TIntermSymbol *instanceArraySymbol    = node->getLeft()->getAsSymbolNode();
                    const TInterfaceBlock *interfaceBlock = leftType.getInterfaceBlock();

                    if (mReferencedShaderStorageBlocks.count(interfaceBlock->uniqueId().get()) == 0)
                    {
                        mReferencedShaderStorageBlocks[interfaceBlock->uniqueId().get()] =
                            new TReferencedBlock(interfaceBlock, &instanceArraySymbol->variable());
                        GetShaderStorageBlockMembersInfo(interfaceBlock, mShaderStorageBlocks,
                                                         &mBlockMemberInfoMap);
                    }

                    const int arrayIndex = node->getRight()->getAsConstantUnion()->getIConst(0);
                    out << mResourcesHLSL->InterfaceBlockInstanceString(
                        instanceArraySymbol->getName(), arrayIndex);
                    return false;
                }
            }
            else
            {
                writeEOpIndexDirectOrIndirectOutput(out, visit, node);
            }
            break;
        }
        case EOpIndexIndirect:
        {
            if (!IsInShaderStorageBlock(node->getLeft()))
            {
                return mOutputHLSL->visitBinary(visit, node);
            }

            // We do not currently support indirect references to interface blocks
            ASSERT(node->getLeft()->getBasicType() != EbtInterfaceBlock);
            writeEOpIndexDirectOrIndirectOutput(out, visit, node);
            break;
        }
        case EOpIndexDirectStruct:
        {
            if (!IsInShaderStorageBlock(node->getLeft()))
            {
                return mOutputHLSL->visitBinary(visit, node);
            }

            if (visit == InVisit)
            {
                ASSERT(IsInShaderStorageBlock(node->getLeft()));
                const TStructure *structure       = node->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = structure->fields()[index->getIConst(0)];
                out << " + ";
                writeDotOperatorOutput(out, field);
                return false;
            }
            break;
        }
        case EOpIndexDirectInterfaceBlock:
            if (!IsInShaderStorageBlock(node->getLeft()))
            {
                return mOutputHLSL->visitBinary(visit, node);
            }

            if (visit == InVisit)
            {
                ASSERT(IsInShaderStorageBlock(node->getLeft()));
                out << ", ";
                const TInterfaceBlock *interfaceBlock =
                    node->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                writeDotOperatorOutput(out, field);
                return false;
            }
            break;
        default:
            // It may have other operators in EOpIndexIndirect. Such as buffer.attribs[(y * gridSize
            // + x) * 6u + 0u]
            return mOutputHLSL->visitBinary(visit, node);
    }

    return true;
}

void ShaderStorageBlockOutputHLSL::writeEOpIndexDirectOrIndirectOutput(TInfoSinkBase &out,
                                                                       Visit visit,
                                                                       TIntermBinary *node)
{
    ASSERT(IsInShaderStorageBlock(node->getLeft()));
    if (visit == InVisit)
    {
        const TType &type = node->getLeft()->getType();
        // For array of arrays, we calculate the offset using the formula below:
        // elementStride * (a3 * a2 * a1 * i0 + a3 * a2 * i1 + a3 * i2 + i3)
        // Note: assume that there are 4 dimensions.
        //       a0, a1, a2, a3 is the size of the array in each dimension. (S s[a0][a1][a2][a3])
        //       i0, i1, i2, i3 is the index of the array in each dimension. (s[i0][i1][i2][i3])
        if (IsInArrayOfArraysChain(node->getLeft()))
        {
            if (type.isArrayOfArrays())
            {
                const TSpan<const unsigned int> &arraySizes = type.getArraySizes();
                // Don't need to concern the tail comma which will be used to multiply the index.
                for (unsigned int i = 0; i < (arraySizes.size() - 1); i++)
                {
                    out << arraySizes[i];
                    out << " * ";
                }
            }
        }
        else
        {
            if (node->getType().isVector() && type.isMatrix())
            {
                if (mRowMajor)
                {
                    out << " + " << str(BlockLayoutEncoder::kBytesPerComponent);
                }
                else
                {
                    out << " + " << str(mMatrixStride);
                }
            }
            else if (node->getType().isScalar() && !type.isArray())
            {
                if (mRowMajor)
                {
                    out << " + " << str(mMatrixStride);
                }
                else
                {
                    out << " + " << str(BlockLayoutEncoder::kBytesPerComponent);
                }
            }

            out << " * ";
        }
    }
    else if (visit == PostVisit)
    {
        // This is used to output the '+' in the array of arrays formula in above.
        if (node->getType().isArray() && !isEndOfSSBOAccessChain())
        {
            out << " + ";
        }
        // This corresponds to '(' in writeDotOperatorOutput when fieldType.isArrayOfArrays() is
        // true.
        if (IsInArrayOfArraysChain(node->getLeft()) && !node->getType().isArray())
        {
            out << ")";
        }
        if (mLocationAsTheLastArgument && isEndOfSSBOAccessChain())
        {
            out << ")";
        }
    }
}

void ShaderStorageBlockOutputHLSL::writeDotOperatorOutput(TInfoSinkBase &out, const TField *field)
{
    auto fieldInfoIter = mBlockMemberInfoMap.find(field);
    ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
    const BlockMemberInfo &memberInfo = fieldInfoIter->second;
    mMatrixStride                     = memberInfo.matrixStride;
    mRowMajor                         = memberInfo.isRowMajorMatrix;
    out << memberInfo.offset;

    const TType &fieldType = *field->type();
    if (fieldType.isArray() && !isEndOfSSBOAccessChain())
    {
        out << " + ";
        out << memberInfo.arrayStride;
        if (fieldType.isArrayOfArrays())
        {
            out << " * (";
        }
    }
    if (mLocationAsTheLastArgument && isEndOfSSBOAccessChain())
    {
        out << ")";
    }
}

TIntermSymbol *ShaderStorageBlockOutputHLSL::findBase(TIntermNode *node) const
{
    if (TIntermSymbol *symbolNode = node->getAsSymbolNode())
    {
        return symbolNode;
    }
    else if (TIntermSwizzle *swizzleNode = node->getAsSwizzleNode())
    {
        return findBase(swizzleNode->getOperand());
    }
    else if (TIntermBinary *binaryNode = node->getAsBinaryNode())
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
                return findBase(binaryNode->getLeft());
            default:
                return nullptr;
        }
    }
    return nullptr;
}

void ShaderStorageBlockOutputHLSL::writeBufferSymbol(TInfoSinkBase &out, TIntermNode *node)
{
    if (TIntermSymbol *symbolNode = node->getAsSymbolNode())
    {
        const TType &type = symbolNode->variable().getType();
        if (type.isInterfaceBlock())
        {
            out << DecorateVariableIfNeeded(symbolNode->variable());
        }
        else
        {
            out << Decorate(type.getInterfaceBlock()->name());
        }
    }
    else if (TIntermSwizzle *swizzleNode = node->getAsSwizzleNode())
    {
        writeBufferSymbol(out, swizzleNode->getOperand());
    }
    else if (TIntermBinary *binaryNode = node->getAsBinaryNode())
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirect:
            {
                const TType &leftType = binaryNode->getLeft()->getType();
                if (leftType.isInterfaceBlock())
                {
                    TIntermSymbol *instanceArraySymbol = binaryNode->getLeft()->getAsSymbolNode();
                    const int arrayIndex =
                        binaryNode->getRight()->getAsConstantUnion()->getIConst(0);
                    out << mResourcesHLSL->InterfaceBlockInstanceString(
                        instanceArraySymbol->getName(), arrayIndex);
                }
                else
                {
                    writeBufferSymbol(out, binaryNode->getLeft());
                }
                break;
            }
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
            case EOpIndexDirectStruct:
                writeBufferSymbol(out, binaryNode->getLeft());
                break;
            default:
                break;
        }
    }
}

TIntermTyped *ShaderStorageBlockOutputHLSL::createByteAddressExpression(TIntermTyped *node, SSBOMethod method)
{
    if (TIntermSwizzle *swizzleNode = node->getAsSwizzleNode())
    {
        // FIXME: should the swizzle offsets be applied to the byte address?
        return createByteAddressExpression(swizzleNode->getOperand(), method);
    }
    else if (TIntermBinary *binaryNode = node->getAsBinaryNode())
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirect:
            {
                const TType &leftType = binaryNode->getLeft()->getType();
                if (leftType.isInterfaceBlock())
                {
                    ASSERT(leftType.getQualifier() == EvqBuffer);
                    TIntermSymbol *instanceArraySymbol    = binaryNode->getLeft()->getAsSymbolNode();
                    const TInterfaceBlock *interfaceBlock = leftType.getInterfaceBlock();

                    if (mReferencedShaderStorageBlocks.count(interfaceBlock->uniqueId().get()) == 0)
                    {
                        mReferencedShaderStorageBlocks[interfaceBlock->uniqueId().get()] =
                            new TReferencedBlock(interfaceBlock, &instanceArraySymbol->variable());
                        GetShaderStorageBlockMembersInfo(interfaceBlock, mShaderStorageBlocks,
                                                         &mBlockMemberInfoMap);
                    }
                }
                else
                {
                    return convertEOpIndexDirectOrIndirectOutput(binaryNode, method);
                }
                break;
            }
            case EOpIndexIndirect:
            {
                // We do not currently support indirect references to interface blocks
                ASSERT(binaryNode->getLeft()->getBasicType() != EbtInterfaceBlock);
                return convertEOpIndexDirectOrIndirectOutput(binaryNode, method);
                break;
            }
            case EOpIndexDirectStruct:
            {
                // We do not currently support direct references to interface blocks
                ASSERT(binaryNode->getLeft()->getBasicType() != EbtInterfaceBlock);
                TIntermTyped *left = createByteAddressExpression(binaryNode->getLeft(), method);
                const TStructure *structure       = binaryNode->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index = binaryNode->getRight()->getAsConstantUnion();
                const TField *field               = structure->fields()[index->getIConst(0)];
                return addFieldOffset(field, left);
                break;
            }
            case EOpIndexDirectInterfaceBlock:
            {
                // FIXME: COMMA HERE
                ASSERT(IsInShaderStorageBlock(binaryNode->getLeft()));
                const TInterfaceBlock *interfaceBlock = binaryNode->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = binaryNode->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                auto fieldInfoIter = mBlockMemberInfoMap.find(field);
                ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
                const BlockMemberInfo &memberInfo = fieldInfoIter->second;
                return CreateUIntNode(memberInfo.offset);
                break;
            }
            default:
                return nullptr;
        }
    }
    return nullptr;
}
TIntermTyped *ShaderStorageBlockOutputHLSL::convertEOpIndexDirectOrIndirectOutput(TIntermBinary *node, SSBOMethod method)
{
    ASSERT(IsInShaderStorageBlock(node->getLeft()));
    TIntermTyped *left = createByteAddressExpression(node->getLeft(), method);
    TIntermTyped *right = node->getRight()->deepCopy();
    const TType &type  = node->getLeft()->getType();

    if (node->getLeft()->getType().isUnsizedArray())
    {
        TLayoutBlockStorage storage;
        bool rowMajor;
        GetBlockLayoutInfo(node, false, &storage, &rowMajor);
        int stride = GetArrayStrideByType(node->getLeft()->getType(), storage, rowMajor);
        right      = new TIntermBinary(EOpMul, CreateUIntNode(stride), right);
    }
    else if (node->getLeft()->getType().isArray())
    {
        const TSpan<const unsigned int> &arraySizes = type.getArraySizes();
        for (unsigned int i = 0; i < arraySizes.size(); i++)
        {
            right = new TIntermBinary(EOpMul, CreateUIntNode(arraySizes[i]), right);
        }
    }
    else if (node->getLeft()->getType().isVector() && type.isMatrix())
    {
        if (mRowMajor)
        {
            right = new TIntermBinary(EOpAdd, CreateUIntNode(BlockLayoutEncoder::kBytesPerComponent), right);
        }
        else
        {
            right = new TIntermBinary(EOpAdd, CreateUIntNode(mMatrixStride), right);
        }
    }
    else if (node->getLeft()->getType().isScalar() && !type.isArray())
    {
        if (mRowMajor)
        {
            right = new TIntermBinary(EOpAdd, CreateUIntNode(mMatrixStride), right);
        }
        else
        {
            right = new TIntermBinary(EOpAdd, CreateUIntNode(BlockLayoutEncoder::kBytesPerComponent), right);
        }
    }
    return left ? new TIntermBinary(EOpAdd, left, right) : right;
}

TIntermTyped *ShaderStorageBlockOutputHLSL::addFieldOffset(const TField *field, TIntermTyped* node)
{
    auto fieldInfoIter = mBlockMemberInfoMap.find(field);
    ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
    const BlockMemberInfo &memberInfo = fieldInfoIter->second;

    return new TIntermBinary(EOpAdd, CreateUIntNode(memberInfo.offset), node);
}

}  // namespace sh
