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
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

const bool kOutputVariableUses = true;

enum class IOType
{
    Input,
    Output
};

struct GlslToWgslBuiltinMapping
{
    const char *glslBuiltinName;
    PipelineAnnotation wgslPipelineAnnotation;
    IOType ioType;
    const TVariable *builtinVar;
};

bool GetWgslBuiltinName(std::string glslBuiltinName,
                        GLenum shaderType,
                        GlslToWgslBuiltinMapping *outMapping)
{
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinVertex(
            {{"gl_VertexID",
              GlslToWgslBuiltinMapping{"gl_VertexID",
                                       BuiltinAnnotation{ImmutableString("vertex_index")},
                                       IOType::Input, BuiltInVariable::gl_VertexID()}},
             {"gl_InstanceID",
              GlslToWgslBuiltinMapping{"gl_InstanceID",
                                       BuiltinAnnotation{ImmutableString("instance_index")},
                                       IOType::Input, BuiltInVariable::gl_InstanceID()}},
             {"gl_Position",
              GlslToWgslBuiltinMapping{"gl_Position",
                                       BuiltinAnnotation{ImmutableString("position")},
                                       IOType::Output, BuiltInVariable::gl_Position()}},
             // TODO(anglebug.com/42267100): might have to emulate clip_distances, see
             // Metal's
             // https://source.chromium.org/chromium/chromium/src/+/main:third_party/angle/src/compiler/translator/msl/TranslatorMSL.cpp?q=symbol%3A%5Cbsh%3A%3AEmulateClipDistanceVaryings%5Cb%20case%3Ayes
             {"gl_ClipDistance",
              GlslToWgslBuiltinMapping{"gl_ClipDistance",
                                       BuiltinAnnotation{ImmutableString("clip_distances")},
                                       IOType::Output, nullptr}}});
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinFragment({
            {"gl_FragCoord",
             GlslToWgslBuiltinMapping{"gl_FragCoord",
                                      BuiltinAnnotation{ImmutableString("position")}, IOType::Input,
                                      BuiltInVariable::gl_FragCoord()}},
            {"gl_FrontFacing",
             GlslToWgslBuiltinMapping{"gl_FrontFacing",
                                      BuiltinAnnotation{ImmutableString("front_facing")},
                                      IOType::Input, BuiltInVariable::gl_FrontFacing()}},
            {"gl_SampleID",
             GlslToWgslBuiltinMapping{"gl_SampleID",
                                      BuiltinAnnotation{ImmutableString("sample_index")},
                                      IOType::Input, BuiltInVariable::gl_SampleID()}},
            // TODO(anglebug.com/42267100): gl_SampleMask is GLSL 4.00 or ARB_sample_shading and
            // requires some special handling (see Metal).
            {"gl_SampleMaskIn",
             GlslToWgslBuiltinMapping{"gl_SampleMaskIn",
                                      BuiltinAnnotation{ImmutableString("sample_mask")},
                                      IOType::Input, nullptr}},
            // Just translate FragColor into a location = 0 out variable.
            // TODO(anglebug.com/42267100): maybe ASSERT that there are no user-defined output
            // variables? Is it possible for there to be other output variables?
            {"gl_FragColor",
             GlslToWgslBuiltinMapping{"gl_FragColor", AttributeLocationAnnotation{0},
                                      IOType::Output, BuiltInVariable::gl_FragColor()}},
            {"gl_SampleMask",
             GlslToWgslBuiltinMapping{"gl_SampleMask",
                                      BuiltinAnnotation{ImmutableString("sample_mask")},
                                      IOType::Output, nullptr}},
            {"gl_FragDepth",
             GlslToWgslBuiltinMapping{"gl_FragDepth",
                                      BuiltinAnnotation{ImmutableString("frag_depth")},
                                      IOType::Output, BuiltInVariable::gl_FragDepth()}},
        });
    // TODO(anglebug.com/42267100): gl_FragData needs to be emulated. Need something
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

ImmutableString CreateNameToReplaceBuiltin(const char *glslBuiltinName)
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

using GlobalVars = TMap<ImmutableString, TIntermDeclaration *>;

GlobalVars FindGlobalVars(TIntermBlock &root)
{
    GlobalVars globals;
    for (TIntermNode *node : *root.getSequence())
    {
        if (TIntermDeclaration *declNode = node->getAsDeclarationNode())
        {
            Declaration decl = ViewDeclaration(*declNode);
            globals.insert({decl.symbol.variable().name(), declNode});
        }
    }
    return globals;
}

// TODO comment
struct AddToPipelineStruct
{
    const ShaderVariable &shaderVar;
    PipelineAnnotation pipelineAnnotation;
    ImmutableString newName;
    const TVariable *var;
};

// TODO comment
[[nodiscard]] bool PrepareToFillPipelineStructs(const std::vector<ShaderVariable> &shaderVars,
                                                TVector<AddToPipelineStruct> *addToPipelineStruct,
                                                TVector<TIntermDeclaration *> *varDeclsToDelete,
                                                const GlobalVars &globalVars,
                                                TCompiler &compiler,
                                                IOType ioType,
                                                std::string debugString)
{
    for (const ShaderVariable &shaderVar : shaderVars)
    {
        if (shaderVar.name == "gl_FragData" || shaderVar.name == "gl_SecondaryFragColorEXT" ||
            shaderVar.name == "gl_SecondaryFragDataEXT")
        {
            // TODO(anglebug.com/42267100): declare gl_FragData as multiple variables.
            UNIMPLEMENTED();
            return false;
        }

        if (kOutputVariableUses)
        {
            std::cout << "Use of " << (shaderVar.isBuiltIn() ? "builtin " : "") << debugString
                      << ": " << shaderVar.name << std::endl;
        }

        if (shaderVar.isBuiltIn())
        {
            GlslToWgslBuiltinMapping wgslName;
            if (!GetWgslBuiltinName(shaderVar.name, compiler.getShaderType(), &wgslName))
            {
                return false;
            }

            const TVariable *varToReplace = wgslName.builtinVar;

            if (varToReplace == nullptr)
            {
                // Should be declared somewhere as a symbol.
                varToReplace = static_cast<const TVariable *>(compiler.getSymbolTable().findBuiltIn(
                    ImmutableString(wgslName.glslBuiltinName), compiler.getShaderVersion()));
                if (kOutputVariableUses)
                {
                    std::cout
                        << "Var " << shaderVar.name
                        << " did not have a BuiltIn var but does have a builtin in the symbol "
                           "table";  // TODO will this also have a declaration? Are there any gl_
                                     // variable that require declaration?
                }
            }

            ASSERT(ioType == wgslName.ioType);

            AddToPipelineStruct addToPipeline{shaderVar, wgslName.wgslPipelineAnnotation,
                                              CreateNameToReplaceBuiltin(wgslName.glslBuiltinName),
                                              varToReplace};

            addToPipelineStruct->push_back(addToPipeline);
        }
        else
        {
            TIntermDeclaration *declNode = globalVars.find(shaderVar.name)->second;
            const TVariable *astVar      = &ViewDeclaration(*declNode).symbol.variable();
            varDeclsToDelete->push_back(declNode);

            AddToPipelineStruct addToPipeline{shaderVar, AttributeLocationAnnotation(),
                                              shaderVar.name, astVar};

            addToPipelineStruct->push_back(addToPipeline);
        }
    }

    return true;
}

// Fills the input and output structs with new fields that will be necessary, and prepares a
// VariableReplacementMap to replace old variable accesses with fields accesses of the appropriae
// structs.
void CreatePipelineStructFields(TVector<AddToPipelineStruct> varsToBecomeFields,
                                TFieldList *fieldList,
                                TVariable *structVariableDeclaration,
                                PipelineAnnotationsMap &pipelineAnnotationsOut,
                                VariableReplacementMap *variableReplacementMap)
{
    for (const AddToPipelineStruct &varToBecomeField : varsToBecomeFields)
    {

        TType *type = new TType(varToBecomeField.var->getType());
        TField *newAttributeField =
            new TField(type, varToBecomeField.newName, {0, 0, 0, 0}, SymbolType::AngleInternal);
        fieldList->push_back(newAttributeField);
        // Tell the next stage of the WGSL outputter that this field requires a location()
        // attribute.
        pipelineAnnotationsOut.insert({newAttributeField, varToBecomeField.pipelineAnnotation});

        variableReplacementMap->insert(
            {varToBecomeField.var,
             &AccessField(*structVariableDeclaration, newAttributeField->name())});
    }
}

}  // namespace

bool PartiallyRewriteBuiltinVariables(TCompiler &compiler,
                                      TIntermBlock &root,
                                      PipelineAnnotationsMap &pipelineAnnotationsOut,
                                      bool *needsInputStructOut,
                                      bool *needsOutputStructOut)
{
    // Track all of the uses of GLSL's builtin variables so we can use the corresponding WGSL
    // builtin values in the translated WGSL code.
    TVector<AddToPipelineStruct> addToInputStruct;
    TVector<AddToPipelineStruct> addToOutputStruct;
    // Inputs and outputs must also be declared in WGSL's input and output structs with location(#)
    // specifiers.

    // Global variable declarations can be deleted once they are moved into the pipeline structs.
    TVector<TIntermDeclaration *> varDeclsToDelete;

    GlobalVars globalVars = FindGlobalVars(root);

    if (!PrepareToFillPipelineStructs(compiler.getInputVaryings(), &addToInputStruct,
                                      &varDeclsToDelete, globalVars, compiler, IOType::Input,
                                      "input varyings") ||
        !PrepareToFillPipelineStructs(compiler.getAttributes(), &addToInputStruct,
                                      &varDeclsToDelete, globalVars, compiler, IOType::Input,
                                      "input attributes") ||
        !PrepareToFillPipelineStructs(compiler.getOutputVaryings(), &addToOutputStruct,
                                      &varDeclsToDelete, globalVars, compiler, IOType::Output,
                                      "output varyings") ||
        !PrepareToFillPipelineStructs(compiler.getOutputVariables(), &addToOutputStruct,
                                      &varDeclsToDelete, globalVars, compiler, IOType::Output,
                                      "output variables"))
    {
        return false;
    }

    TSymbolTable *symbolTable = &compiler.getSymbolTable();

    // Two TFieldLists used to build the input/output structs.
    TFieldList &inputFieldList  = *new TFieldList;
    TFieldList &outputFieldList = *new TFieldList;

    // Add the two structs, and declarations of one instance each, to the AST.
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

    // All references to the old declared variables will be replaced by field accesses into the
    // pipeline structs.
    VariableReplacementMap variableReplacementMap;

    // TODO(mpdenton): file comment about location() variables.
    CreatePipelineStructFields(addToInputStruct, &inputFieldList, &inputVariableDeclaration,
                               pipelineAnnotationsOut, &variableReplacementMap);
    CreatePipelineStructFields(addToOutputStruct, &outputFieldList, &outputVariableDeclaration,
                               pipelineAnnotationsOut, &variableReplacementMap);

    // Put in function and use on separate input/output lists.
    //  for (var : list) {
    //      var = old var (see `type`)
    //      type = type of new field (globalVars.find(shaderVar.name), or could be a builtin
    //      variable, or done using findBuiltin on the SymbolTable (TODO find out which vars
    //      match that pattern)) create field
    //          requires a name (builtin var name + "_" or same as original attribute)
    //          use `type`
    //      insert field -> pipelineAnnotation into `pipelineAnnotationsOut` (pipeline
    //      annotation is either a location or a builtin string) Add to variableReplacementMap[]
    //  }
    //
    // This:
    //  Loop through all the collected variables and construct the two lists.
    //  Call the function above on the two lists.
    //  Add the input and output structs if they are needed, respectively.
    //  Delete all the declarations here (so that we minimize the amount of time the AST is
    //  invalid) Replace all variables with the variable replacement map
    //

    // Insert the input/output struct declarations and their associated variables into the AST.

    if (outputFieldList.size() == 0)
    {
        *needsOutputStructOut = false;
    }
    else
    {
        root.insertStatement(0, &outputStructTypeDeclaration);
        root.insertStatement(1, &outputVariableDeclarationAstDecl);
        *needsOutputStructOut = true;
    }

    if (inputFieldList.size() == 0)
    {
        *needsInputStructOut = false;
    }
    else
    {
        root.insertStatement(0, &inputStructTypeDeclaration);
        root.insertStatement(1, &inputVariableDeclarationAstDecl);
        *needsInputStructOut = true;
    }

    for (TIntermDeclaration *declNode : varDeclsToDelete)
    {
        // TODO(mpdenton): just add a delete method?
        auto emptySequence = new TIntermSequence;
        root.replaceChildNodeWithMultiple(declNode, *emptySequence);
    }

    if (!ReplaceVariables(&compiler, &root, variableReplacementMap))
    {
        return false;
    }

    return true;
}
}  // namespace sh
