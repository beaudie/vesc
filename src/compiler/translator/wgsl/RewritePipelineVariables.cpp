//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/wgsl/RewritePipelineVariables.h"

#include <string>
#include <utility>

#include "GLES2/gl2.h"
#include "GLSLANG/ShaderVars.h"
#include "anglebase/no_destructor.h"
#include "common/angleutils.h"
#include "common/log_utils.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/OutputTree.h"
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
            // variables? Is it possible for there to be other output variables when using
            // FragColor?
            {"gl_FragColor",
             GlslToWgslBuiltinMapping{"gl_FragColor", LocationAnnotation{0}, IOType::Output,
                                      BuiltInVariable::gl_FragColor()}},
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
    ImmutableStringBuilder newName(strlen(glslBuiltinName) + 1);
    newName << glslBuiltinName << '_';
    return newName;
}

TIntermBinary *AccessFieldByIndex(TIntermTyped &object, int index)
{
    const TType &type = object.getType();
    ASSERT(!type.isArray());
    const TStructure *structure = type.getStruct();
    ASSERT(structure);
    ASSERT(0 <= index);
    ASSERT(static_cast<size_t>(index) < structure->fields().size());

    return new TIntermBinary(
        TOperator::EOpIndexDirectStruct, &object,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
}

TIntermBinary *AccessField(const TVariable &structVarTVariable, const ImmutableString &fieldName)
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

// Records information about a ShaderVariable and how it should be annotated when it is inserted
// into a pipeline (input/output) struct.
struct AddToPipelineStruct
{
    const ShaderVariable &shaderVar;
    PipelineAnnotation pipelineAnnotation;
    ImmutableString newName;
    const TVariable *var;
};

// Given a list of `shaderVars` (as well as `compiler`), computes the fields that should appear in
// the input/output pipeline structs and the annotations that should appear in the WGSL source.
//
// `addToPipelineStruct` will be filled with AddToPipelineStruct entries that map a shaderVar from
// `shaderVars` to the appropriate pipeline annotation (either @builtin(wgslBuiltinName) or
// @location(...)), a new name when the shaderVar is stored into the appropriate pipeline struct,
// and the TVariable `var` that corresponds to the shaderVar.
//
// Finally, `debugString` should describe `shaderVars` (e.g. "input varyings"), and `ioType`
// indicates whether `shaderVars` is meant to be an input or output variable, which is useful for
// debugging asserts.
//
// TODO(anglebug.com/42267100): for now this only deals with builtins, and not user-defined
// inputs/outputs.
[[nodiscard]] bool PrepareToFillPipelineStructs(const std::vector<ShaderVariable> &shaderVars,
                                                TVector<AddToPipelineStruct> *addToPipelineStruct,
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
                // TODO(anglebug.com/42267100): Not sure if this ever actually occurs. Will this
                // TVariable also have a declaration? Are there any gl_ variable that require or
                // even allow declaration?
                varToReplace = static_cast<const TVariable *>(compiler.getSymbolTable().findBuiltIn(
                    ImmutableString(wgslName.glslBuiltinName), compiler.getShaderVersion()));
                if (kOutputVariableUses)
                {
                    std::cout
                        << "Var " << shaderVar.name
                        << " did not have a BuiltIn var but does have a builtin in the symbol "
                           "table";
                }
            }

            ASSERT(ioType == wgslName.ioType);

            AddToPipelineStruct addToPipeline{shaderVar, wgslName.wgslPipelineAnnotation,
                                              CreateNameToReplaceBuiltin(wgslName.glslBuiltinName),
                                              varToReplace};

            addToPipelineStruct->push_back(addToPipeline);
        }
    }

    return true;
}

// Fills the input and output structs with new fields that will be necessary, and prepares a
// VariableReplacementMap to replace old variable accesses with fields accesses of the appropriate
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
             AccessField(*structVariableDeclaration, newAttributeField->name())});
    }
}

}  // namespace

bool PartiallyRewritePipelineVariables(TCompiler &compiler,
                                       TIntermBlock &root,
                                       PipelineAnnotationsMap &pipelineAnnotationsOut,
                                       bool *needsInputStructOut,
                                       bool *needsOutputStructOut)
{
    // Track all of the uses of GLSL's input/output variables so we can add them all to input/output
    // pipeline structs for WGSL's main function to understand.
    // TODO(anglebug.com/42267100): for now this only deals with builtins, and not user-defined
    // inputs/outputs.
    TVector<AddToPipelineStruct> addToInputStruct;
    TVector<AddToPipelineStruct> addToOutputStruct;

    if (!PrepareToFillPipelineStructs(compiler.getInputVaryings(), &addToInputStruct, compiler,
                                      IOType::Input, "input varyings") ||
        !PrepareToFillPipelineStructs(compiler.getAttributes(), &addToInputStruct, compiler,
                                      IOType::Input, "input attributes") ||
        !PrepareToFillPipelineStructs(compiler.getOutputVaryings(), &addToOutputStruct, compiler,
                                      IOType::Output, "output varyings") ||
        !PrepareToFillPipelineStructs(compiler.getOutputVariables(), &addToOutputStruct, compiler,
                                      IOType::Output, "output variables"))
    {
        return false;
    }

    TSymbolTable *symbolTable = &compiler.getSymbolTable();

    // Two TFieldLists used to build the input/output structs.
    TFieldList *inputFieldList  = new TFieldList;
    TFieldList *outputFieldList = new TFieldList;

    // Construct AST nodes that declare the two structs, and declare of one instance each, to
    // prepare to replace input/output variable references to references to fields of the pipeline
    // structs.
    TStructure *inputStructType =
        new TStructure(symbolTable, ImmutableString(kBuiltinInputStructType), inputFieldList,
                       SymbolType::AngleInternal);
    TStructure *outputStructType =
        new TStructure(symbolTable, ImmutableString(kBuiltinOutputStructType), outputFieldList,
                       SymbolType::AngleInternal);
    TType *outputStructTypeAstType      = new TType(inputStructType, /*isStructSpecifier=*/true);
    TType *inputStructTypeAstType       = new TType(outputStructType, /*isStructSpecifier=*/true);
    TVariable *inputStructTypeVariable  = new TVariable(symbolTable, kEmptyImmutableString,
                                                        outputStructTypeAstType, SymbolType::Empty);
    TVariable *outputStructTypeVariable = new TVariable(symbolTable, kEmptyImmutableString,
                                                        inputStructTypeAstType, SymbolType::Empty);
    TIntermDeclaration *inputStructTypeDeclaration =
        new TIntermDeclaration({inputStructTypeVariable});
    TIntermDeclaration *outputStructTypeDeclaration =
        new TIntermDeclaration({outputStructTypeVariable});
    TVariable *inputVariableDeclaration = new TVariable(
        symbolTable, ImmutableString(kBuiltinInputStructName),
        new TType(inputStructType, /*isStructSpecifier=*/false), SymbolType::AngleInternal);
    TVariable *outputVariableDeclaration = new TVariable(
        symbolTable, ImmutableString(kBuiltinOutputStructName),
        new TType(outputStructType, /*isStructSpecifier=*/false), SymbolType::AngleInternal);
    TIntermDeclaration *inputVariableDeclarationAstDecl =
        new TIntermDeclaration({inputVariableDeclaration});
    TIntermDeclaration *outputVariableDeclarationAstDecl =
        new TIntermDeclaration({outputVariableDeclaration});

    // All references to the old declared variables will be replaced by field accesses into the
    // pipeline structs. Build this VariableReplacementMap while the pipeline structs are built.
    VariableReplacementMap variableReplacementMap;
    // Fill the input/output pipeline structs with fields that correspond to the shader's input and
    // output variables.
    CreatePipelineStructFields(addToInputStruct, inputFieldList, inputVariableDeclaration,
                               pipelineAnnotationsOut, &variableReplacementMap);
    CreatePipelineStructFields(addToOutputStruct, outputFieldList, outputVariableDeclaration,
                               pipelineAnnotationsOut, &variableReplacementMap);

    // Insert the input/output struct declarations and their associated variables into the AST.
    if (outputFieldList->size() == 0)
    {
        *needsOutputStructOut = false;
    }
    else
    {
        root.insertStatement(0, outputStructTypeDeclaration);
        root.insertStatement(1, outputVariableDeclarationAstDecl);
        *needsOutputStructOut = true;
    }

    if (inputFieldList->size() == 0)
    {
        *needsInputStructOut = false;
    }
    else
    {
        root.insertStatement(0, inputStructTypeDeclaration);
        root.insertStatement(1, inputVariableDeclarationAstDecl);
        *needsInputStructOut = true;
    }

    OutputTree(&root, compiler.getInfoSink().info);

    // Finally, replace the variable accesses with field accesses.
    if (!ReplaceVariables(&compiler, &root, variableReplacementMap))
    {
        return false;
    }

    return true;
}
}  // namespace sh
