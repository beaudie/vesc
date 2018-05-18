//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockOutputHLSL: A traverser to translate a buffer variable of shader storage block
// to an offset of RWByteAddressBuffer.
//

#include "compiler/translator/ShaderStorageBlockOutputHLSL.h"

#include "compiler/translator/UniformHLSL.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/blocklayoutHLSL.h"

namespace sh
{

namespace
{

const TField *GetFieldMemberInShaderStorageBlock(const TInterfaceBlock *interfaceBlock,
                                                 const ImmutableString &varableName)
{
    for (const TField *field : interfaceBlock->fields())
    {
        if (field->name() == varableName)
        {
            return field;
        }
    }
    return nullptr;
}

void SetShaderStorageBlockFieldMemberInfo(const TFieldList &fields,
                                          sh::BlockLayoutEncoder *encoder,
                                          bool inRowMajorLayout)
{
    for (TField *field : fields)
    {
        const TType &fieldType      = *field->type();
        const bool isRowMajorLayout = (fieldType.getLayoutQualifier().matrixPacking == EmpRowMajor);
        bool rowMajorLayout         = (inRowMajorLayout || isRowMajorLayout);
        const TStructure *fieldStruct = fieldType.getStruct();
        if (fieldStruct)
        {
            // TODO(jiajia.qin@intel.com): Add structure field member support.
        }
        else if (fieldType.isArrayOfArrays())
        {
            // TODO(jiajia.qin@intel.com): Add array of array field member support.
        }
        else
        {
            std::vector<unsigned int> fieldarraySizes;
            if (auto *arraySizes = fieldType.getArraySizes())
            {
                fieldarraySizes.assign(arraySizes->begin(), arraySizes->end());
            }
            const BlockMemberInfo &memeberInfo = encoder->encodeType(
                GLVariableType(fieldType), fieldarraySizes, rowMajorLayout && fieldType.isMatrix());
            field->setOffset(memeberInfo.offset);
            field->setArrayStride(memeberInfo.arrayStride);
        }
    }
}

void SetShaderStorageBlockMembersOffset(const TInterfaceBlock *interfaceBlock)
{
    sh::Std140BlockEncoder std140Encoder;
    sh::HLSLBlockEncoder hlslEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED, false);
    sh::BlockLayoutEncoder *encoder = nullptr;

    if (interfaceBlock->blockStorage() == EbsStd140)
    {
        encoder = &std140Encoder;
    }
    else
    {
        encoder = &hlslEncoder;
    }

    SetShaderStorageBlockFieldMemberInfo(interfaceBlock->fields(), encoder, false);
}

}  // anonymous namespace

ShaderStorageBlockOutputHLSL::ShaderStorageBlockOutputHLSL(OutputHLSL *outputHLSL,
                                                           TSymbolTable *symbolTable,
                                                           UniformHLSL *uniformHLSL)
    : TIntermTraverser(true, true, true, symbolTable),
      mBufferVariableInAssignLeft(false),
      mOutputHLSL(outputHLSL),
      mUniformHLSL(uniformHLSL)
{
    mSSBOFunctionHLSL = new ShaderStorageBlockFunctionHLSL;
}

ShaderStorageBlockOutputHLSL::~ShaderStorageBlockOutputHLSL()
{
    SafeDelete(mSSBOFunctionHLSL);
}

void ShaderStorageBlockOutputHLSL::setAssignLeft(bool inLeft)
{
    mBufferVariableInAssignLeft = inLeft;
}

TString ShaderStorageBlockOutputHLSL::shaderStorageBlocksHeader()
{
    return mUniformHLSL->shaderStorageBlocksHeader(mReferencedShaderStorageBlocks);
}

bool ShaderStorageBlockOutputHLSL::isLastNodeInSSBO()
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

const TType &ShaderStorageBlockOutputHLSL::ancestorEvaluatesBufferVariableType(const TType &type)
{
    TIntermBinary *preAncestorBinary = nullptr;
    for (unsigned int n = 0u; getAncestorNode(n) != nullptr; ++n)
    {
        TIntermNode *ancestor         = getAncestorNode(n);
        TIntermBinary *ancestorBinary = ancestor->getAsBinaryNode();
        if (ancestorBinary != nullptr)
        {
            switch (ancestorBinary->getOp())
            {
                case EOpIndexDirectStruct:
                case EOpIndexDirect:
                case EOpIndexIndirect:
                {
                    preAncestorBinary = ancestorBinary;
                    continue;
                }
                default:
                    break;
            }
            if (preAncestorBinary)
            {
                return preAncestorBinary->getType();
            }
        }

        const TIntermSwizzle *ancestorSwizzle = ancestor->getAsSwizzleNode();
        if (ancestorSwizzle)
        {
            return ancestorSwizzle->getType();
        }
        else
        {
            break;
        }
    }

    if (preAncestorBinary)
    {
        return preAncestorBinary->getType();
    }

    return type;
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
            SetShaderStorageBlockMembersOffset(interfaceBlock);
        }
        if (variableType.isInterfaceBlock())
        {
            out << DecorateVariableIfNeeded(variable);
        }
        else
        {
            SSBOMethod method;
            if (mBufferVariableInAssignLeft)
            {
                method = SSBOMethod::STORE;
            }
            else
            {
                method = SSBOMethod::LOAD;
            }
            const TType &type = ancestorEvaluatesBufferVariableType(variableType);
            const TString &functionName =
                mSSBOFunctionHLSL->registerShaderStorageBlockFunction(type, method);

            out << functionName;
            out << "(";
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
    TInfoSinkBase &out = mOutputHLSL->getInfoSink();
    mOutputHLSL->writeConstantUnion(out, node->getType(), node->getConstantValue());
}

bool ShaderStorageBlockOutputHLSL::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    TInfoSinkBase &out = mOutputHLSL->getInfoSink();
    if (visit == PostVisit)
    {
        // TODO(jiajia.qin@intel.com): add swizzle process.
        if (!mBufferVariableInAssignLeft)
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
                        SetShaderStorageBlockMembersOffset(interfaceBlock);
                    }

                    const int arrayIndex = node->getRight()->getAsConstantUnion()->getIConst(0);
                    out << mUniformHLSL->InterfaceBlockInstanceString(
                        instanceArraySymbol->getName(), arrayIndex);
                    return false;
                }
            }
            else
            {
                writeEOpIndexDirectOrIndirectOutput(out, visit, node);
            }
        }
        break;
        case EOpIndexIndirect:
        {
            // We do not currently support indirect references to interface blocks
            ASSERT(node->getLeft()->getBasicType() != EbtInterfaceBlock);
            writeEOpIndexDirectOrIndirectOutput(out, visit, node);
        }
        break;
        case EOpIndexDirectStruct:
        {
            if (visit == InVisit)
            {
                ASSERT(IsInShaderStorageBlock(node->getLeft()));
                const TStructure *structure       = node->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = structure->fields()[index->getIConst(0)];
                writeDotOperatorOutput(out, field);
                return false;
            }
        }
        break;
        case EOpIndexDirectInterfaceBlock:
        {
            ASSERT(IsInShaderStorageBlock(node->getLeft()));
            if (visit == PreVisit)
            {
                SSBOMethod method;
                if (mBufferVariableInAssignLeft)
                {
                    method = SSBOMethod::STORE;
                }
                else
                {
                    method = SSBOMethod::LOAD;
                }
                const TType &type = ancestorEvaluatesBufferVariableType(node->getType());
                const TString &functionName =
                    mSSBOFunctionHLSL->registerShaderStorageBlockFunction(type, method);

                out << functionName;
                out << "(";
            }
            else if (visit == InVisit)
            {
                out << ", ";
                const TInterfaceBlock *interfaceBlock =
                    node->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                writeDotOperatorOutput(out, field);
                return false;
            }
            break;
        }
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
        if (node->getType().isVector() && type.isMatrix())
        {
            int matrixStride =
                BlockLayoutEncoder::ComponentsPerRegister * BlockLayoutEncoder::BytesPerComponent;
            out << " + " << str(matrixStride);
        }
        else if (node->getType().isScalar() && !type.isArray())
        {
            int scalarStride = BlockLayoutEncoder::BytesPerComponent;
            out << " + " << str(scalarStride);
        }

        if (type.isUnsizedArray())
        {
            out << " + ";
        }
        else
        {
            out << " * ";
        }
    }
    else if (visit == PostVisit && !mBufferVariableInAssignLeft && isLastNodeInSSBO())
    {
        out << ")";
    }
}

void ShaderStorageBlockOutputHLSL::writeDotOperatorOutput(TInfoSinkBase &out, const TField *field)
{
    out << str(field->getOffset());

    const TType &fieldType = *field->type();
    if (fieldType.isArray() && !fieldType.isUnsizedArray() && !isLastNodeInSSBO())
    {
        out << " + ";
        out << field->getArrayStride();
    }
    if (!mBufferVariableInAssignLeft && isLastNodeInSSBO())
    {
        out << ")";
    }
}

}  // namespace sh
