//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceForShaderFramebufferFetch.h: Find any references to gl_LastFragData, and replace it with
// ANGLELastFragData.
//

#include "compiler/translator/tree_ops/ReplaceForShaderFramebufferFetch.h"

#include "common/bitset_utils.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"
#include "compiler/translator/util.h"

#include "compiler/translator/OutputTree.h"

namespace sh
{
namespace
{

using InputAttachmentIdxSet = angle::BitSet<32>;
using MapForReplacement     = const std::map<const TVariable *, const TIntermTyped *>;

enum class InputType
{
    SubpassInput = 0,
    SubpassInputMS,
    ISubpassInput,
    ISubpassInputMS,
    USubpassInput,
    USubpassInputMS,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

class InputAttachmentReferenceTraverser : public TIntermTraverser
{
  public:
    InputAttachmentReferenceTraverser(std::map<unsigned int, TIntermSymbol *> *declaredSymOut,
                                      unsigned int *maxInputAttachmentIndex,
                                      InputAttachmentIdxSet *constIndicesOut)
        : TIntermTraverser(true, false, false),
          mDeclaredSym(declaredSymOut),
          mMaxInputAttachmentIndex(maxInputAttachmentIndex),
          mConstInputAttachmentIndices(constIndicesOut),
          mTemporaryDeclared(nullptr)
    {
        mDeclaredSym->clear();
        *mMaxInputAttachmentIndex = 0;
        mConstInputAttachmentIndices->reset();
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;

  private:
    std::map<unsigned int, TIntermSymbol *> *mDeclaredSym;
    unsigned int *mMaxInputAttachmentIndex;
    InputAttachmentIdxSet *mConstInputAttachmentIndices;

    // This is for storing the redeclaration symbol of gl_LastFragData
    TIntermSymbol *mTemporaryDeclared;
};

class ReplaceVariableTraverser : public TIntermTraverser
{
  public:
    ReplaceVariableTraverser(const std::map<unsigned int, const TVariable *> &toBeReplaced,
                             const std::map<unsigned int, const TIntermTyped *> &replacement,
                             const std::set<const TIntermSymbol *> &declared)
        : TIntermTraverser(true, false, false),
          mDeclared(declared),
          mToBeReplaced(toBeReplaced),
          mReplacement(replacement)
    {
        ASSERT(mToBeReplaced.size() == mReplacement.size());
    }

    ReplaceVariableTraverser(const TVariable *toBeReplaced,
                             const TIntermTyped *replacement,
                             const TIntermSymbol *declared)
        : TIntermTraverser(true, false, false),
          mDeclared({declared}),
          mToBeReplaced({{0, toBeReplaced}}),
          mReplacement({{0, replacement}})
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    void visitSymbol(TIntermSymbol *node) override;

  private:
    const std::set<const TIntermSymbol *> mDeclared;
    const std::map<unsigned int, const TVariable *> mToBeReplaced;
    const std::map<unsigned int, const TIntermTyped *> mReplacement;
};

bool InputAttachmentReferenceTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());

    if (sequence.size() != 1)
    {
        return true;
    }

    TIntermTyped *variable = sequence.front()->getAsTyped();
    TIntermSymbol *symbol  = variable->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getName() == "gl_LastFragData")
    {
        mTemporaryDeclared = variable->getAsSymbolNode();
    }
    else if (symbol->getType().getQualifier() == EvqFragmentInOut)
    {
        unsigned int inputAttachmentIdx = symbol->getType().getLayoutQualifier().location;
        ASSERT(inputAttachmentIdx < mConstInputAttachmentIndices->size());
        mConstInputAttachmentIndices->set(inputAttachmentIdx);
        *mMaxInputAttachmentIndex = std::max(*mMaxInputAttachmentIndex, inputAttachmentIdx);

        mDeclaredSym->emplace(inputAttachmentIdx, symbol);
    }

    return true;
}

bool InputAttachmentReferenceTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    TOperator op = node->getOp();
    if (op != EOpIndexDirect && op != EOpIndexIndirect)
    {
        return true;
    }

    TIntermSymbol *left = node->getLeft()->getAsSymbolNode();
    if (!left)
    {
        return true;
    }
    else if (left->getName() != "gl_LastFragData")
    {
        return true;
    }

    const TConstantUnion *constIdx = node->getRight()->getConstantValue();
    if (!constIdx)
    {
        return true;
    }
    else
    {
        unsigned int idx = 0;
        switch (constIdx->getType())
        {
            case EbtInt:
                idx = constIdx->getIConst();
                break;
            case EbtUInt:
                idx = constIdx->getUConst();
                break;
            case EbtFloat:
                idx = static_cast<unsigned int>(constIdx->getFConst());
                break;
            case EbtBool:
                idx = constIdx->getBConst() ? 1 : 0;
                break;
            default:
                UNREACHABLE();
                break;
        }
        ASSERT(idx < mConstInputAttachmentIndices->size());
        mConstInputAttachmentIndices->set(idx);

        *mMaxInputAttachmentIndex = std::max(*mMaxInputAttachmentIndex, idx);

        mDeclaredSym->emplace(idx, mTemporaryDeclared);
    }

    return true;
}

bool ReplaceVariableTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());
    if (sequence.size() != 1)
    {
        return true;
    }

    TIntermTyped *variable = sequence.front()->getAsTyped();
    TIntermSymbol *symbol  = variable->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (mDeclared.find(symbol) != mDeclared.end())
    {
        TIntermSequence emptyReplacement;
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, emptyReplacement);

        return true;
    }

    return true;
}

void ReplaceVariableTraverser::visitSymbol(TIntermSymbol *node)
{
    for (unsigned int i = 0; i < mToBeReplaced.size(); i++)
    {
        if (&node->variable() == mToBeReplaced.at(i) && mDeclared.find(node) == mDeclared.end())
        {
            queueReplacement(mReplacement.at(i)->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }
}

void AddInputAttachmentUniform(std::vector<ShaderVariable> *uniforms,
                               const std::map<unsigned int, TVariable *> &inputAttachmentVarArray,
                               const unsigned int &maxInputAttachmentIndex,
                               const InputAttachmentIdxSet &constIndices)
{
    for (unsigned int index = 0; index <= maxInputAttachmentIndex; index++)
    {
        if (constIndices.test(index))
        {
            const TVariable *inputAttachmentVar = inputAttachmentVarArray.at(index);

            ShaderVariable inputAttachmentUniform;
            inputAttachmentUniform.active    = true;
            inputAttachmentUniform.staticUse = true;
            inputAttachmentUniform.name.assign(inputAttachmentVar->name().data(),
                                               inputAttachmentVar->name().length());
            inputAttachmentUniform.mappedName.assign(inputAttachmentUniform.name);
            inputAttachmentUniform.inputAttachmentIndex =
                inputAttachmentVar->getType().getLayoutQualifier().inputAttachmentIndex;
            uniforms->push_back(inputAttachmentUniform);
        }
    }
}

InputType GetInputTypeOfSubpassInput(const TBasicType &basicType)
{
    switch (basicType)
    {
        case TBasicType::EbtSubpassInput:
            return InputType::SubpassInput;
        case TBasicType::EbtSubpassInputMS:
            return InputType::SubpassInputMS;
        case TBasicType::EbtISubpassInput:
            return InputType::ISubpassInput;
        case TBasicType::EbtISubpassInputMS:
            return InputType::ISubpassInputMS;
        case TBasicType::EbtUSubpassInput:
            return InputType::USubpassInput;
        case TBasicType::EbtUSubpassInputMS:
            return InputType::USubpassInputMS;
        default:
            UNREACHABLE();
            return InputType::InvalidEnum;
    }
}

TBasicType GetBasicTypeOfSubpassInput(const InputType &inputType)
{
    switch (inputType)
    {
        case InputType::SubpassInput:
            return EbtSubpassInput;
        case InputType::SubpassInputMS:
            return EbtSubpassInputMS;
        case InputType::ISubpassInput:
            return EbtISubpassInput;
        case InputType::ISubpassInputMS:
            return EbtISubpassInputMS;
        case InputType::USubpassInput:
            return EbtUSubpassInput;
        case InputType::USubpassInputMS:
            return EbtUSubpassInputMS;
        default:
            UNREACHABLE();
            return TBasicType::EbtVoid;
    }
}

TBasicType GetBasicTypeForSubpassInput(const TBasicType &inputType)
{
    switch (inputType)
    {
        case EbtFloat:
            return EbtSubpassInput;
        case EbtInt:
            return EbtISubpassInput;
        case EbtUInt:
            return EbtUSubpassInput;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

TBasicType GetBasicTypeForSubpassInput(const TIntermSymbol *originSymbol)
{
    if (originSymbol->getName().beginsWith("gl_LastFragData"))
    {
        return GetBasicTypeForSubpassInput(EbtFloat);
    }

    return GetBasicTypeForSubpassInput(originSymbol->getBasicType());
}

ImmutableString GetTypeNameOfSubpassInput(const InputType &inputType)
{
    switch (inputType)
    {
        case InputType::SubpassInput:
            return ImmutableString("subpassInput");
        case InputType::SubpassInputMS:
            return ImmutableString("subpassInputMS");
        case InputType::ISubpassInput:
            return ImmutableString("isubpassInput");
        case InputType::ISubpassInputMS:
            return ImmutableString("isubpassInputMS");
        case InputType::USubpassInput:
            return ImmutableString("usubpassInput");
        case InputType::USubpassInputMS:
            return ImmutableString("usubpassInputMS");
        default:
            UNREACHABLE();
            return kEmptyImmutableString;
    }
}

ImmutableString GetFunctionNameOfSubpassLoad(const InputType &inputType)
{
    switch (inputType)
    {
        case InputType::SubpassInput:
            return ImmutableString("subpassLoad");
        case InputType::SubpassInputMS:
            return ImmutableString("subpassLoadMS");
        case InputType::ISubpassInput:
            return ImmutableString("isubpassLoad");
        case InputType::ISubpassInputMS:
            return ImmutableString("isubpassLoadMS");
        case InputType::USubpassInput:
            return ImmutableString("usubpassLoad");
        case InputType::USubpassInputMS:
            return ImmutableString("usubpassLoadMS");
        default:
            UNREACHABLE();
            return kEmptyImmutableString;
    }
}

bool DeclareSubpassInputVariables(TSymbolTable *symbolTable,
                                  TIntermSequence *declareVariables,
                                  std::map<unsigned int, TVariable *> *inputAttachmentVarArray,
                                  const unsigned int &maxInputAttachmentIndex,
                                  const InputAttachmentIdxSet &constIndices,
                                  const std::map<unsigned int, TIntermSymbol *> &declaredVarVec)
{
    bool isDeclared = false;

    for (unsigned int index = 0; index <= maxInputAttachmentIndex; index++)
    {
        if (constIndices.test(index))
        {
            TBasicType subpassInputType = GetBasicTypeForSubpassInput(declaredVarVec.at(index));
            if (subpassInputType == EbtVoid)
            {
                return false;
            }

            constexpr unsigned int numSize = 2;
            ImmutableString defaultVarName(vk::kInputAttachmentName);
            ImmutableStringBuilder varNameBuilder(defaultVarName.length() + numSize);
            varNameBuilder << defaultVarName;
            varNameBuilder.appendDecimal(index);

            TVariable **inputAttachmentVar = &((*inputAttachmentVarArray)[index]);
            TType *inputAttachmentType = new TType(subpassInputType, EbpUndefined, EvqUniform, 1);

            TLayoutQualifier inputAttachmentQualifier = inputAttachmentType->getLayoutQualifier();
            inputAttachmentQualifier.inputAttachmentIndex = index;
            inputAttachmentType->setLayoutQualifier(inputAttachmentQualifier);

            (*inputAttachmentVar) = new TVariable(symbolTable, varNameBuilder, inputAttachmentType,
                                                  SymbolType::AngleInternal);
            TIntermSymbol *inputAttachmentDeclarator = new TIntermSymbol(*inputAttachmentVar);

            TIntermDeclaration *inputAttachmentDecl = new TIntermDeclaration;
            inputAttachmentDecl->appendDeclarator(inputAttachmentDeclarator);

            declareVariables->push_back(inputAttachmentDecl);

            isDeclared = true;
        }
    }

    return isDeclared;
}

TIntermAggregate *CreateSubpassLoadFuncCall(TSymbolTable *symbolTable,
                                            std::map<InputType, TFunction *> *functionMap,
                                            const InputType &inputType,
                                            TIntermSequence *arguments)
{
    TBasicType subpassInputType = GetBasicTypeOfSubpassInput(inputType);
    ASSERT(subpassInputType != TBasicType::EbtVoid);

    TFunction **currentFunc = &(*functionMap)[inputType];
    if (*currentFunc == nullptr)
    {
        TType *inputAttachmentType = new TType(subpassInputType, EbpUndefined, EvqUniform, 1);
        *currentFunc = new TFunction(symbolTable, GetFunctionNameOfSubpassLoad(inputType),
                                     SymbolType::AngleInternal,
                                     new TType(EbtFloat, EbpUndefined, EvqGlobal, 4, 1), true);
        (*currentFunc)
            ->addParameter(new TVariable(symbolTable, GetTypeNameOfSubpassInput(inputType),
                                         inputAttachmentType, SymbolType::AngleInternal));
    }

    return TIntermAggregate::CreateFunctionCall(**currentFunc, arguments);
}

}  // anonymous namespace

ANGLE_NO_DISCARD bool ReplaceLastFragData(TCompiler *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable,
                                          std::vector<ShaderVariable> *uniforms)
{
    // Common variables
    InputAttachmentIdxSet constIndices;
    std::map<unsigned int, TIntermSymbol *> redeclaredGLLastFragDataMap;
    unsigned int maxInputAttachmentIndex = 0;

    // Get informations for gl_LastFragData
    InputAttachmentReferenceTraverser informationTraverser(&redeclaredGLLastFragDataMap,
                                                           &maxInputAttachmentIndex, &constIndices);
    root->traverse(&informationTraverser);
    if (constIndices.none())
    {
        // No references of gl_LastFragData
        return true;
    }

    // Declare subpassInput uniform variables
    TIntermSequence *declareVariables = new TIntermSequence();
    std::map<unsigned int, TVariable *> inputAttachmentVarArray;
    if (!DeclareSubpassInputVariables(symbolTable, declareVariables, &inputAttachmentVarArray,
                                      maxInputAttachmentIndex, constIndices,
                                      redeclaredGLLastFragDataMap))
    {
        return false;
    }

    AddInputAttachmentUniform(uniforms, inputAttachmentVarArray, maxInputAttachmentIndex,
                              constIndices);

    // Declare the variables which store the result of subpassLoad function
    const TVariable *glLastFragDataVar = nullptr;
    if (redeclaredGLLastFragDataMap.size() > 0)
    {
        // gl_LastFragData can be redelcared only once
        ASSERT(redeclaredGLLastFragDataMap.size() == 1);
        glLastFragDataVar = &redeclaredGLLastFragDataMap.begin()->second->variable();
    }
    else
    {
        glLastFragDataVar = static_cast<const TVariable *>(
            symbolTable->findBuiltIn(ImmutableString("gl_LastFragData"), 100));
    }
    if (!glLastFragDataVar)
    {
        return false;
    }

    const TBasicType loadVarBasicType = glLastFragDataVar->getType().getBasicType();
    const TPrecision loadVarPrecision = glLastFragDataVar->getType().getPrecision();
    const unsigned int loadVarVecSize = glLastFragDataVar->getType().getNominalSize();
    const int loadVarArraySize        = glLastFragDataVar->getType().getOutermostArraySize();

    TIntermDeclaration *loadVarDecl = new TIntermDeclaration;

    ImmutableString loadVarName("ANGLELastFragData");
    TType *loadVarType = new TType(loadVarBasicType, loadVarPrecision, EvqGlobal,
                                   static_cast<unsigned char>(loadVarVecSize));
    loadVarType->makeArray(loadVarArraySize);

    TVariable *loadVar =
        new TVariable(symbolTable, loadVarName, loadVarType, SymbolType::AngleInternal);
    TIntermSymbol *loadVarDeclarator = new TIntermSymbol(loadVar);
    loadVarDecl->appendDeclarator(loadVarDeclarator);
    declareVariables->push_back(loadVarDecl);

    for (unsigned int index = 0; index < declareVariables->size(); index++)
    {
        root->insertStatement(index, declareVariables->at(index));
    }

    // 3) Add the routine for reading InputAttachment data
    std::map<InputType, TFunction *> functionMap;
    TIntermBlock *loadInputAttachmentBlock = new TIntermBlock;
    for (unsigned int index = 0; index <= maxInputAttachmentIndex; index++)
    {
        if (constIndices.test(index))
        {
            TIntermSymbol *loadInputAttachmentDataSymbol = new TIntermSymbol(loadVar);
            TIntermSymbol *inputAttachmentSymbol =
                new TIntermSymbol(inputAttachmentVarArray[index]);

            TIntermBinary *left = new TIntermBinary(
                EOpIndexDirect, loadInputAttachmentDataSymbol->deepCopy(), CreateIndexNode(index));

            TIntermSequence *subpassArguments = new TIntermSequence();
            subpassArguments->push_back(inputAttachmentSymbol);
            TIntermAggregate *subpassLoadFuncCall = CreateSubpassLoadFuncCall(
                symbolTable, &functionMap,
                GetInputTypeOfSubpassInput(inputAttachmentSymbol->getBasicType()),
                subpassArguments);

            TIntermBinary *assignment = new TIntermBinary(EOpAssign, left, subpassLoadFuncCall);

            loadInputAttachmentBlock->appendStatement(assignment);
        }
    }

    ASSERT(loadInputAttachmentBlock->getChildCount() > 0);
    if (!RunAtTheBeginningOfShader(compiler, root, loadInputAttachmentBlock))
    {
        return false;
    }

    // 4) Replace gl_LastFragData with ANGLELastFragData
    const TIntermSymbol *redeclaredGLLastFragData = nullptr;
    if (redeclaredGLLastFragDataMap.size() > 0)
    {
        redeclaredGLLastFragData = redeclaredGLLastFragDataMap.begin()->second;
    }

    ReplaceVariableTraverser replaceTraverser(glLastFragDataVar, new TIntermSymbol(loadVar),
                                              redeclaredGLLastFragData);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    return true;
}

ANGLE_NO_DISCARD bool ReplaceInOutVariables(TCompiler *compiler,
                                            TIntermBlock *root,
                                            TSymbolTable *symbolTable,
                                            std::vector<ShaderVariable> *uniforms)
{
    // Common variables
    InputAttachmentIdxSet constIndices;
    std::map<unsigned int, TIntermSymbol *> declaredInOutVarMap;
    unsigned int maxInputAttachmentIndex = 0;

    // Get informations for gl_LastFragData
    InputAttachmentReferenceTraverser informationTraverser(&declaredInOutVarMap,
                                                           &maxInputAttachmentIndex, &constIndices);
    root->traverse(&informationTraverser);
    if (declaredInOutVarMap.size() == 0)
    {
        // No references of the variable decorated with a inout qualifier
        return true;
    }

    // Declare subpassInput uniform variables
    TIntermSequence *declareVariables = new TIntermSequence();
    std::map<unsigned int, TVariable *> inputAttachmentVarArray;
    if (!DeclareSubpassInputVariables(symbolTable, declareVariables, &inputAttachmentVarArray,
                                      maxInputAttachmentIndex, constIndices, declaredInOutVarMap))
    {
        return false;
    }

    AddInputAttachmentUniform(uniforms, inputAttachmentVarArray, maxInputAttachmentIndex,
                              constIndices);

    std::map<unsigned int, const TVariable *> toBeReplaced;
    std::map<unsigned int, const TVariable *> newOutVarArray;
    for (auto originInOutVarIter : declaredInOutVarMap)
    {
        const unsigned int inputAttachmentIndex = originInOutVarIter.first;
        const TIntermSymbol *originInOutVar     = originInOutVarIter.second;

        const TBasicType loadVarBasicType = originInOutVar->getType().getBasicType();
        const TPrecision loadVarPrecision = originInOutVar->getType().getPrecision();
        const unsigned int loadVarVecSize = originInOutVar->getType().getNominalSize();

        TType *newOutVarType = new TType(loadVarBasicType, loadVarPrecision, EvqGlobal,
                                         static_cast<unsigned char>(loadVarVecSize));

        // We just want to use the original variable decorated with a inout qualifier, except the
        // qualifier itself. The qualifier will be changed from inout to out.
        newOutVarType->setQualifier(TQualifier::EvqFragmentOut);

        TIntermDeclaration *newOutVarDecl = new TIntermDeclaration;
        TVariable *newOutVar = new TVariable(symbolTable, originInOutVar->getName(), newOutVarType,
                                             SymbolType::UserDefined);
        newOutVarArray[inputAttachmentIndex] = newOutVar;

        TIntermSymbol *newOutVarDeclarator = new TIntermSymbol(newOutVar);
        newOutVarDecl->appendDeclarator(newOutVarDeclarator);
        declareVariables->push_back(newOutVarDecl);

        toBeReplaced[inputAttachmentIndex] = &originInOutVar->variable();
    }

    for (unsigned int index = 0; index < declareVariables->size(); index++)
    {
        root->insertStatement(index, declareVariables->at(index));
    }

    // 3) Add the routine for reading InputAttachment data
    std::map<InputType, TFunction *> functionMap;
    TIntermBlock *loadInputAttachmentBlock = new TIntermBlock;
    for (unsigned int index = 0; index <= maxInputAttachmentIndex; index++)
    {
        if (constIndices.test(index))
        {
            TIntermSymbol *loadInputAttachmentDataSymbol = new TIntermSymbol(newOutVarArray[index]);
            TIntermSymbol *inputAttachmentSymbol =
                new TIntermSymbol(inputAttachmentVarArray[index]);

            TIntermSequence *subpassArguments = new TIntermSequence();
            subpassArguments->push_back(inputAttachmentSymbol);
            TIntermAggregate *subpassLoadFuncCall = CreateSubpassLoadFuncCall(
                symbolTable, &functionMap,
                GetInputTypeOfSubpassInput(inputAttachmentSymbol->getBasicType()),
                subpassArguments);

            const int targetVecSize = loadInputAttachmentDataSymbol->getType().getNominalSize();
            TVector<int> fieldOffsets(targetVecSize);
            for (int i = 0; i < targetVecSize; i++)
            {
                fieldOffsets[i] = i;
            }

            TIntermTyped *right = new TIntermSwizzle(subpassLoadFuncCall, fieldOffsets);
            TIntermBinary *assignment =
                new TIntermBinary(EOpAssign, loadInputAttachmentDataSymbol, right);

            loadInputAttachmentBlock->appendStatement(assignment);
        }
    }

    ASSERT(loadInputAttachmentBlock->getChildCount() > 0);
    if (!RunAtTheBeginningOfShader(compiler, root, loadInputAttachmentBlock))
    {
        return false;
    }

    std::set<const TIntermSymbol *> declared;
    for (auto declareInOutVarIter : declaredInOutVarMap)
    {
        declared.emplace(declareInOutVarIter.second);
    }

    std::map<unsigned int, const TIntermTyped *> replacement;
    for (auto newOutVar = newOutVarArray.begin(); newOutVar != newOutVarArray.end(); newOutVar++)
    {
        const unsigned int inputAttachmentIndex = newOutVar->first;
        replacement[inputAttachmentIndex]       = new TIntermSymbol(newOutVar->second);
    }

    // 4) Replace previous 'inout' variable with newly created 'inout' variable
    ReplaceVariableTraverser replaceTraverser(toBeReplaced, replacement, declared);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    return true;
}

}  // namespace sh
