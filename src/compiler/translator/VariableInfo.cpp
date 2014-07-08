//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "angle_gl.h"
#include "compiler/translator/VariableInfo.h"
#include "compiler/translator/util.h"
#include "common/utilities.h"

template <typename VarT>
static void ExpandUserDefinedVariable(const VarT &variable,
                                      const std::string &name,
                                      const std::string &mappedName,
                                      bool markStaticUse,
                                      std::vector<VarT> *expanded);

// Returns info for an attribute, uniform, or varying.
template <typename VarT>
static void ExpandVariable(const VarT &variable,
                           const std::string &name,
                           const std::string &mappedName,
                           bool markStaticUse,
                           std::vector<VarT> *expanded)
{
    if (variable.isStruct())
    {
        if (variable.isArray())
        {
            for (size_t elementIndex = 0; elementIndex < variable.elementCount(); elementIndex++)
            {
                std::string lname = name + ArrayString(elementIndex);
                std::string lmappedName = mappedName + ArrayString(elementIndex);
                ExpandUserDefinedVariable(variable, lname, lmappedName, markStaticUse, expanded);
            }
        }
        else
        {
            ExpandUserDefinedVariable(variable, name, mappedName, markStaticUse, expanded);
        }
    }
    else
    {
        VarT expandedVar = variable;

        expandedVar.name = name;
        expandedVar.mappedName = mappedName;

        // Mark all expanded fields as used if the parent is used
        if (markStaticUse)
        {
            expandedVar.staticUse = true;
        }

        if (expandedVar.isArray())
        {
            expandedVar.name += "[0]";
            expandedVar.mappedName += "[0]";
        }

        expanded->push_back(expandedVar);
    }
}

template <class VarT>
static void ExpandUserDefinedVariable(const VarT &variable,
                                      const std::string &name,
                                      const std::string &mappedName,
                                      bool markStaticUse,
                                      std::vector<VarT> *expanded)
{
    ASSERT(variable.isStruct());

    const std::vector<VarT> &fields = variable.fields;

    for (size_t fieldIndex = 0; fieldIndex < fields.size(); fieldIndex++)
    {
        const VarT &field = fields[fieldIndex];
        ExpandVariable(field,
                       name + "." + field.name,
                       mappedName + "." + field.mappedName,
                       markStaticUse,
                       expanded);
    }
}

template <class VarT>
static VarT* findVariable(const TString &name,
                          std::vector<VarT> *infoList)
{
    // TODO(zmo): optimize this function.
    for (size_t ii = 0; ii < infoList->size(); ++ii)
    {
        if ((*infoList)[ii].name.c_str() == name)
            return &((*infoList)[ii]);
    }
    return NULL;
}

CollectVariables::CollectVariables(std::vector<sh::Attribute> *attribs,
                                   std::vector<sh::Attribute> *outputVariables,
                                   std::vector<sh::Uniform> *uniforms,
                                   std::vector<sh::Varying> *varyings,
                                   std::vector<sh::InterfaceBlock> *interfaceBlocks,
                                   ShHashFunction64 hashFunction)
    : mAttribs(attribs),
      mOutputVariables(outputVariables),
      mUniforms(uniforms),
      mVaryings(varyings),
      mInterfaceBlocks(interfaceBlocks),
      mPointCoordAdded(false),
      mFrontFacingAdded(false),
      mFragCoordAdded(false),
      mHashFunction(hashFunction)
{
}

// We want to check whether a uniform/varying is statically used
// because we only count the used ones in packing computing.
// Also, gl_FragCoord, gl_PointCoord, and gl_FrontFacing count
// toward varying counting if they are statically used in a fragment
// shader.
void CollectVariables::visitSymbol(TIntermSymbol *symbol)
{
    ASSERT(symbol != NULL);
    sh::ShaderVariable *var = NULL;

    if (sh::IsVarying(symbol->getQualifier()))
    {
        var = findVariable(symbol->getSymbol(), mVaryings);
    }
    else if (symbol->getType().getBasicType() == EbtInterfaceBlock)
    {
        sh::InterfaceBlock *interfaceBlock = findVariable(symbol->getSymbol(), mInterfaceBlocks);
        if (interfaceBlock)
        {
            interfaceBlock->staticUse = true;
        }
    }
    else
    {
        switch (symbol->getQualifier())
        {
          case EvqFragmentOut:
            var = findVariable(symbol->getSymbol(), mOutputVariables);
            break;
          case EvqUniform:
            var = findVariable(symbol->getSymbol(), mUniforms);
            break;
          case EvqFragCoord:
            if (!mFragCoordAdded)
            {
                sh::Varying info;
                info.name = "gl_FragCoord";
                info.mappedName = "gl_FragCoord";
                info.type = GL_FLOAT_VEC4;
                info.arraySize = 0;
                info.precision = GL_MEDIUM_FLOAT;  // Use mediump as it doesn't really matter.
                info.staticUse = true;
                mVaryings->push_back(info);
                mFragCoordAdded = true;
            }
            return;
          case EvqFrontFacing:
            if (!mFrontFacingAdded)
            {
                sh::Varying info;
                info.name = "gl_FrontFacing";
                info.mappedName = "gl_FrontFacing";
                info.type = GL_BOOL;
                info.arraySize = 0;
                info.precision = GL_NONE;
                info.staticUse = true;
                mVaryings->push_back(info);
                mFrontFacingAdded = true;
            }
            return;
          case EvqPointCoord:
            if (!mPointCoordAdded)
            {
                sh::Varying info;
                info.name = "gl_PointCoord";
                info.mappedName = "gl_PointCoord";
                info.type = GL_FLOAT_VEC2;
                info.arraySize = 0;
                info.precision = GL_MEDIUM_FLOAT;  // Use mediump as it doesn't really matter.
                info.staticUse = true;
                mVaryings->push_back(info);
                mPointCoordAdded = true;
            }
            return;
          default:
            break;
        }
    }
    if (var)
    {
        var->staticUse = true;
    }
}

struct NameHashCallback
{
  public:
    NameHashCallback(ShHashFunction64 hashFunction)
        : mHashFunction(hashFunction)
    {}

    void operator()(sh::ShaderVariable &variable)
    {
        TString stringName = TString(variable.name.c_str());
        variable.mappedName = TIntermTraverser::hash(stringName, mHashFunction).c_str();
    }

  private:
    ShHashFunction64 mHashFunction;
};

template <>
void CollectVariables::visitVariable(const TIntermSymbol* variable,
                                     std::vector<sh::Attribute> *infoList) const
{
    sh::Attribute attribute = sh::GetSingleVariableInfo<sh::Attribute>(variable->getType(),
                                                                       variable->getSymbol());
    attribute.mappedName = TIntermTraverser::hash(variable->getSymbol(), mHashFunction).c_str();
    attribute.location = variable->getType().getLayoutQualifier().location;
    infoList->push_back(attribute);
}

template <>
void CollectVariables::visitVariable(const TIntermSymbol* variable,
                                     std::vector<sh::InterfaceBlock> *infoList) const
{
    sh::InterfaceBlock interfaceBlock;
    const TInterfaceBlock *blockType = variable->getType().getInterfaceBlock();

    interfaceBlock.name = variable->getSymbol().c_str();
    interfaceBlock.mappedName = TIntermTraverser::hash(variable->getSymbol(), mHashFunction).c_str();
    interfaceBlock.arraySize = variable->getArraySize();
    interfaceBlock.isRowMajorLayout = (blockType->matrixPacking() == EmpRowMajor);
    interfaceBlock.layout = sh::GetBlockLayoutType(blockType->blockStorage());

    ASSERT(blockType);
    const TFieldList &blockFields = blockType->fields();
    sh::SetMatrixPackingCallback callback(interfaceBlock.isRowMajorLayout);

    for (size_t fieldIndex = 0; fieldIndex < blockFields.size(); fieldIndex++)
    {
        const TField *field = blockFields[fieldIndex];
        ASSERT(field);
        sh::GetVariableInfo(*field->type(), field->name(), &interfaceBlock.fields, &callback);
    }
}

template <typename VarT>
void CollectVariables::visitVariable(const TIntermSymbol *variable,
                                     std::vector<VarT> *infoList) const
{
    NameHashCallback callback(mHashFunction);
    sh::GetVariableInfo(variable->getType(), variable->getSymbol(), infoList, &callback);
}

template <typename VarT>
void CollectVariables::visitInfoList(const TIntermSequence &sequence,
                                     std::vector<VarT> *infoList) const
{
    for (size_t seqIndex = 0; seqIndex < sequence.size(); seqIndex++)
    {
        const TIntermSymbol *variable = sequence[seqIndex]->getAsSymbolNode();
        // The only case in which the sequence will not contain a
        // TIntermSymbol node is initialization. It will contain a
        // TInterBinary node in that case. Since attributes, uniforms,
        // and varyings cannot be initialized in a shader, we must have
        // only TIntermSymbol nodes in the sequence.
        ASSERT(variable != NULL);
        visitVariable(variable, infoList);
    }
}

bool CollectVariables::visitAggregate(Visit, TIntermAggregate *node)
{
    bool visitChildren = true;

    switch (node->getOp())
    {
      case EOpDeclaration:
        {
            const TIntermSequence &sequence = node->getSequence();
            const TIntermTyped &typedNode = *sequence.front()->getAsTyped();
            TQualifier qualifier = typedNode.getQualifier();

            if (typedNode.getBasicType() == EbtInterfaceBlock)
            {
                visitInfoList(sequence, mInterfaceBlocks);
            }
            else if (qualifier == EvqAttribute || qualifier == EvqVertexIn ||
                     qualifier == EvqFragmentOut || qualifier == EvqUniform ||
                     sh::IsVarying(qualifier))
            {
                switch (qualifier)
                {
                  case EvqAttribute:
                  case EvqVertexIn:
                    visitInfoList(sequence, mAttribs);
                    break;
                  case EvqFragmentOut:
                    visitInfoList(sequence, mOutputVariables);
                    break;
                  case EvqUniform:
                    visitInfoList(sequence, mUniforms);
                    break;
                  default:
                    visitInfoList(sequence, mVaryings);
                    break;
                }

                if (!sequence.empty())
                {
                    visitChildren = false;
                }
            }
            break;
        }
      default: break;
    }

    return visitChildren;
}

template <typename VarT>
void ExpandVariables(const std::vector<VarT> &compact, std::vector<VarT> *expanded)
{
    for (size_t variableIndex = 0; variableIndex < compact.size(); variableIndex++)
    {
        const VarT &variable = compact[variableIndex];
        ExpandVariable(variable, variable.name, variable.mappedName, variable.staticUse, expanded);
    }
}

template void ExpandVariables(const std::vector<sh::Uniform> &, std::vector<sh::Uniform> *);
template void ExpandVariables(const std::vector<sh::Varying> &, std::vector<sh::Varying> *);
