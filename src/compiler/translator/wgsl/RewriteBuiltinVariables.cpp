//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/wgsl/RewriteBuiltinVariables.h"

#include <string>
#include <utility>

#include "GLES2/gl2.h"
#include "GLSLANG/ShaderVars.h"
#include "anglebase/no_destructor.h"
#include "common/angleutils.h"
#include "common/log_utils.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolUniqueId.h"
#include "compiler/translator/Types.h"
#include "compiler/translator/tree_util/BuiltIn_complete_autogen.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{

namespace
{

const bool kOutputBuiltinVariableUses = true;

enum class IOType
{
    Input,
    Output
};

struct GlslToWgslBuiltinMapping
{
    const char *glslBuiltinName;
    const char *wgslBuiltinName;
    IOType ioType;
    const TVariable *builtinVar;
};

bool GetWgslBuiltinName(std::string glslBuiltinName,
                        GLenum shaderType,
                        GlslToWgslBuiltinMapping *outMapping)
{
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinVertex(
            {{"gl_VertexID", GlslToWgslBuiltinMapping{"gl_VertexID", "vertex_index", IOType::Input,
                                                      BuiltInVariable::gl_VertexID()}},
             {"gl_InstanceID",
              GlslToWgslBuiltinMapping{"gl_InstanceID", "instance_index", IOType::Input,
                                       BuiltInVariable::gl_InstanceID()}},
             {"gl_Position", GlslToWgslBuiltinMapping{"gl_Position", "position", IOType::Output,
                                                      BuiltInVariable::gl_Position()}},
             // TODO(anglebug.com/42267100): might have to emulate clip_distances, see
             // Metal's
             // https://source.chromium.org/chromium/chromium/src/+/main:third_party/angle/src/compiler/translator/msl/TranslatorMSL.cpp?q=symbol%3A%5Cbsh%3A%3AEmulateClipDistanceVaryings%5Cb%20case%3Ayes
             {"gl_ClipDistance", GlslToWgslBuiltinMapping{"gl_ClipDistance", "clip_distances",
                                                          IOType::Output, nullptr}}});
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinFragment({
            {"gl_FragCoord", GlslToWgslBuiltinMapping{"gl_FragCoord", "position", IOType::Input,
                                                      BuiltInVariable::gl_FragCoord()}},
            {"gl_FrontFacing",
             GlslToWgslBuiltinMapping{"gl_FrontFacing", "front_facing", IOType::Input,
                                      BuiltInVariable::gl_FrontFacing()}},
            {"gl_SampleID", GlslToWgslBuiltinMapping{"gl_SampleID", "sample_index", IOType::Input,
                                                     BuiltInVariable::gl_SampleID()}},
            // TODO(anglebug.com/42267100): gl_SampleMask is GLSL 4.00 or ARB_sample_shading and
            // requires some special handling (see Metal).
            {"gl_SampleMaskIn",
             GlslToWgslBuiltinMapping{"gl_SampleMaskIn", "sample_mask", IOType::Input, nullptr}},
            {"gl_SampleMask",
             GlslToWgslBuiltinMapping{"gl_SampleMask", "sample_mask", IOType::Output, nullptr}},
            {"gl_FragDepth", GlslToWgslBuiltinMapping{"gl_FragDepth", "frag_depth", IOType::Output,
                                                      BuiltInVariable::gl_FragDepth()}},
        });
    // TODO(anglebug.com/42267100): gl_FragColor and gl_FragData need to be emulated. Need something
    // like spir-v's
    // third_party/angle/src/compiler/translator/tree_ops/spirv/EmulateFragColorData.h.

    if (shaderType == GL_VERTEX_SHADER)
    {
        auto it = kGlslBuiltinToWgslBuiltinVertex->find(glslBuiltinName);
        if (it == kGlslBuiltinToWgslBuiltinVertex->end())
        {
            return false;
        }
        *outMapping = it->second;
        return true;
    }
    else if (shaderType == GL_FRAGMENT_SHADER)
    {
        auto it = kGlslBuiltinToWgslBuiltinFragment->find(glslBuiltinName);
        if (it == kGlslBuiltinToWgslBuiltinFragment->end())
        {
            return false;
        }
        *outMapping = it->second;
        return true;
    }
    else
    {
        UNREACHABLE();
        return false;
    }
}

ImmutableString createNameToReplaceBuiltin(const char *glslBuiltinName)
{
    return std::string(glslBuiltinName) + "_";
}

TIntermBinary &AccessFieldByIndex(TIntermTyped &object, int index)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const TType &type = object.getType();
    ASSERT(!type.isArray());
    const TStructure *structure = type.getStruct();
    ASSERT(structure);
    ASSERT(0 <= index);
    ASSERT(static_cast<size_t>(index) < structure->fields().size());
#endif

    return *new TIntermBinary(
        TOperator::EOpIndexDirectStruct, &object,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
}

TIntermBinary &AccessField(const TVariable &structVarTVariable, const ImmutableString &fieldName)
{
    const TStructure *structure = structVarTVariable.getType().getStruct();
    ASSERT(structure);
    const TFieldList &fieldList    = structure->fields();
    TIntermSymbol &structVarSymbol = *new TIntermSymbol(&structVarTVariable);
    for (int i = 0; i < static_cast<int>(fieldList.size()); ++i)
    {
        if (fieldList[i]->name() == fieldName)
        {
            return AccessFieldByIndex(structVarSymbol, i);
        }
    }
    UNREACHABLE();
    return AccessFieldByIndex(structVarSymbol, -1);
}

}  // namespace

bool PartiallyRewriteBuiltinVariables(TCompiler &compiler,
                                      TIntermBlock &root,
                                      BuiltinAnnotationsMap &builtinAnnotationsOut)
{
    // Track all of the uses of GLSL's builtin variables so we can use the corresponding WGSL
    // builtin values in the translated WGSL code.
    std::vector<GlslToWgslBuiltinMapping> addToInputStruct;
    std::vector<GlslToWgslBuiltinMapping> addToOutputStruct;

    for (const ShaderVariable &inputVarying : compiler.getInputVaryings())
    {
        if (!inputVarying.isBuiltIn())
        {
            continue;
        }
        if (kOutputBuiltinVariableUses)
        {
            std::cout << "Use of builtin input varying: " << inputVarying.name << std::endl;
        }
        GlslToWgslBuiltinMapping wgslName;
        if (!GetWgslBuiltinName(inputVarying.name, compiler.getShaderType(), &wgslName))
        {
            return false;
        }
        ASSERT(wgslName.ioType == IOType::Input);

        addToInputStruct.push_back(wgslName);
    }
    for (const ShaderVariable &attribute : compiler.getAttributes())
    {

        if (!attribute.isBuiltIn())
        {
            continue;
        }
        if (kOutputBuiltinVariableUses)
        {
            std::cout << "Use of builtin attribute: " << attribute.name << std::endl;
        }
        GlslToWgslBuiltinMapping wgslName;
        if (!GetWgslBuiltinName(attribute.name, compiler.getShaderType(), &wgslName))
        {
            return false;
        }

        switch (wgslName.ioType)
        {
            case IOType::Input:
                addToInputStruct.push_back(wgslName);
                break;
            case IOType::Output:
                addToOutputStruct.push_back(wgslName);
                break;
        }
    }
    for (const ShaderVariable &outputVarying : compiler.getOutputVaryings())
    {
        if (!outputVarying.isBuiltIn())
        {
            continue;
        }
        if (kOutputBuiltinVariableUses)
        {
            std::cout << "Use of builtin output varying: " << outputVarying.name << std::endl;
        }
        GlslToWgslBuiltinMapping wgslName;
        if (!GetWgslBuiltinName(outputVarying.name, compiler.getShaderType(), &wgslName))
        {
            return false;
        }
        ASSERT(wgslName.ioType == IOType::Output);
        addToOutputStruct.push_back(wgslName);
    }

    for (const ShaderVariable &outputVariable : compiler.getOutputVariables())
    {
        if (!outputVariable.isBuiltIn())
        {
            continue;
        }
        if (kOutputBuiltinVariableUses)
        {
            std::cout << "Use of builtin output variable: " << outputVariable.name << std::endl;
        }
        GlslToWgslBuiltinMapping wgslName;
        if (!GetWgslBuiltinName(outputVariable.name, compiler.getShaderType(), &wgslName))
        {
            return false;
        }
        ASSERT(wgslName.ioType == IOType::Output);
        addToOutputStruct.push_back(wgslName);
    }

    TSymbolTable *symbolTable = &compiler.getSymbolTable();

    // Two TFieldLists used to build the input/output structs.
    TFieldList &inputFieldList  = *new TFieldList;
    TFieldList &outputFieldList = *new TFieldList;

    // Add the two structs, and declarations of one instance each, to the beginning of the AST,
    TStructure &inputStructType =
        *new TStructure(symbolTable, ImmutableString(kBuiltinInputStructType), &inputFieldList,
                        SymbolType::AngleInternal);
    TStructure &outputStructType =
        *new TStructure(symbolTable, ImmutableString(kBuiltinOutputStructType), &outputFieldList,
                        SymbolType::AngleInternal);
    TType &outputStructTypeAstType     = *new TType(&inputStructType, /*isStructSpecifier=*/true);
    TType &inputStructTypeAstType      = *new TType(&outputStructType, /*isStructSpecifier=*/true);
    TVariable &inputStructTypeVariable = *new TVariable(
        symbolTable, kEmptyImmutableString, &outputStructTypeAstType, SymbolType::Empty);
    TVariable &outputStructTypeVariable = *new TVariable(
        symbolTable, kEmptyImmutableString, &inputStructTypeAstType, SymbolType::Empty);
    TIntermDeclaration &inputStructTypeDeclaration =
        *new TIntermDeclaration({&inputStructTypeVariable});
    TIntermDeclaration &outputStructTypeDeclaration =
        *new TIntermDeclaration({&outputStructTypeVariable});
    TVariable &inputVariableDeclaration = *new TVariable(
        symbolTable, ImmutableString(kBuiltinInputStructName),
        new TType(&inputStructType, /*isStructSpecifier=*/false), SymbolType::AngleInternal);
    TVariable &outputVariableDeclaration = *new TVariable(
        symbolTable, ImmutableString(kBuiltinOutputStructName),
        new TType(&outputStructType, /*isStructSpecifier=*/false), SymbolType::AngleInternal);
    TIntermDeclaration &inputVariableDeclarationAstDecl =
        *new TIntermDeclaration({&inputVariableDeclaration});
    TIntermDeclaration &outputVariableDeclarationAstDecl =
        *new TIntermDeclaration({&outputVariableDeclaration});

    root.insertStatement(0, &inputStructTypeDeclaration);
    root.insertStatement(1, &inputVariableDeclarationAstDecl);
    root.insertStatement(2, &outputStructTypeDeclaration);
    root.insertStatement(3, &outputVariableDeclarationAstDecl);

    // In a loop adding one field at a time:
    //   Edit one of the structs to add a field.
    //   Ensure that the mapping in `builtinAnnotationsOut` of TField to the struct is correct.
    //   Replace all accesses of the builtin variable with a field access of the struct.

    for (GlslToWgslBuiltinMapping &wgslBuiltin : addToInputStruct)
    {
        if (wgslBuiltin.builtinVar == nullptr)
        {
            // Should be declared somewhere as a symbol.
            wgslBuiltin.builtinVar =
                static_cast<const TVariable *>(compiler.getSymbolTable().findBuiltIn(
                    ImmutableString(wgslBuiltin.glslBuiltinName), compiler.getShaderVersion()));
        }
        TType *type = new TType;
        *type       = wgslBuiltin.builtinVar->getType();
        TField *newBuiltinField =
            new TField(type, createNameToReplaceBuiltin(wgslBuiltin.glslBuiltinName), {0, 0, 0, 0},
                       SymbolType::AngleInternal);
        inputFieldList.push_back(newBuiltinField);

        builtinAnnotationsOut.insert(
            {newBuiltinField, ImmutableString(wgslBuiltin.wgslBuiltinName)});

        // TODO(anglebug.com/42267100): just replace all the variables in one swoop
        if (!ReplaceVariableWithTyped(
                &compiler, &root, wgslBuiltin.builtinVar,
                &AccessField(inputVariableDeclaration, newBuiltinField->name())))
        {
            return false;
        }
    }

    for (GlslToWgslBuiltinMapping &wgslBuiltin : addToOutputStruct)
    {
        if (wgslBuiltin.builtinVar == nullptr)
        {
            // Should be declared somewhere as a symbol.
            wgslBuiltin.builtinVar =
                static_cast<const TVariable *>(compiler.getSymbolTable().findBuiltIn(
                    ImmutableString(wgslBuiltin.glslBuiltinName), compiler.getShaderVersion()));
        }
        TType *type = new TType;
        *type       = wgslBuiltin.builtinVar->getType();
        TField *newBuiltinField =
            new TField(type, createNameToReplaceBuiltin(wgslBuiltin.glslBuiltinName), {0, 0, 0, 0},
                       SymbolType::AngleInternal);
        outputFieldList.push_back(newBuiltinField);

        builtinAnnotationsOut.insert(
            {newBuiltinField, ImmutableString(wgslBuiltin.wgslBuiltinName)});
        if (!ReplaceVariableWithTyped(
                &compiler, &root, wgslBuiltin.builtinVar,
                &AccessField(outputVariableDeclaration, newBuiltinField->name())))
        {
            return false;
        }
    }

    return true;
}
}  // namespace sh
