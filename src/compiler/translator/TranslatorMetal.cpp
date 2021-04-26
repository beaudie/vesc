//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorMetal:
//   A GLSL-based translator that outputs shaders that fit GL_KHR_vulkan_glsl.
//   It takes into account some considerations for Metal backend also.
//   The shaders are then fed into glslang to spit out SPIR-V.
//   See: https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
//
//   The SPIR-V will then be translated to Metal Shading Language later in Metal backend.
//

#include "compiler/translator/TranslatorMetal.h"

#include "angle_gl.h"
#include "common/utilities.h"
#include "compiler/translator/OutputVulkanGLSLForMetal.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/tree_ops/InitializeVariables.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/ReplaceArrayOfMatrixVarying.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace mtl
{
/** extern */
const char kCoverageMaskEnabledConstName[]      = "ANGLECoverageMaskEnabled";
const char kRasterizerDiscardEnabledConstName[] = "ANGLERasterizerDisabled";
}  // namespace mtl

namespace
{
// Metal specific driver uniforms
constexpr const char kDiscardWrapperFuncName[] = "DiscardWrapper";
constexpr const char kHalfRenderArea[]         = "halfRenderArea";
constexpr const char kFlipXY[]                 = "flipXY";
constexpr const char kNegFlipXY[]              = "negFlipXY";
constexpr const char kCoverageMask[]           = "coverageMask";

constexpr ImmutableString kSampleMaskWriteFuncName = ImmutableString("ANGLEWriteSampleMask");

// Unlike Vulkan having auto viewport flipping extension, in Metal we have to flip gl_Position.y
// manually.
// This operation performs flipping the gl_Position.y using this expression:
// gl_Position.y = gl_Position.y * negViewportScaleY
ANGLE_NO_DISCARD bool AppendVertexShaderPositionYCorrectionToMain(TCompiler *compiler,
                                                                  TIntermBlock *root,
                                                                  TSymbolTable *symbolTable,
                                                                  TIntermTyped *negFlipY)
{
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.y"
    TVector<int> swizzleOffsetY;
    swizzleOffsetY.push_back(1);
    TIntermSwizzle *positionY = new TIntermSwizzle(positionRef, swizzleOffsetY);

    // Create the expression "gl_Position.y * negFlipY"
    TIntermBinary *inverseY = new TIntermBinary(EOpMul, positionY->deepCopy(), negFlipY);

    // Create the assignment "gl_Position.y = gl_Position.y * negViewportScaleY
    TIntermTyped *positionYLHS = positionY->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionYLHS, inverseY);

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(compiler, root, assignment, symbolTable);
}

ANGLE_NO_DISCARD bool EmulateInstanceID(TCompiler *compiler,
                                        TIntermBlock *root,
                                        TSymbolTable *symbolTable,
                                        const TVariable *driverUniforms)
{
    // emuInstanceID
    TIntermBinary *emuInstanceID =
        CreateDriverUniformRef(driverUniforms, kEmuInstanceIDField.data());

    // Create a symbol reference to "gl_InstanceIndex"
    const TVariable *instanceID = BuiltInVariable::gl_InstanceIndex();

    return ReplaceVariableWithTyped(compiler, root, instanceID, emuInstanceID);
}

// Initialize unused varying outputs.
ANGLE_NO_DISCARD bool InitializeUnusedOutputs(TIntermBlock *root,
                                              TSymbolTable *symbolTable,
                                              const InitVariableList &unusedVars)
{
    if (unusedVars.empty())
    {
        return true;
    }

    TIntermSequence insertSequence;

    for (const sh::ShaderVariable &var : unusedVars)
    {
        ASSERT(!var.active);
        const TIntermSymbol *symbol = FindSymbolNode(root, var.name);
        ASSERT(symbol);

        TIntermSequence initCode;
        CreateInitCode(symbol, false, false, &initCode, symbolTable);

        insertSequence.insert(insertSequence.end(), initCode.begin(), initCode.end());
    }

    if (!insertSequence.empty())
    {
        TIntermFunctionDefinition *main = FindMain(root);
        TIntermSequence *mainSequence   = main->getBody()->getSequence();

        // Insert init code at the start of main()
        mainSequence->insert(mainSequence->begin(), insertSequence.begin(), insertSequence.end());
    }

    return true;
}
}  // anonymous namespace

// class DriverUniformMetal
TFieldList *DriverUniformMetal::createUniformFields(TSymbolTable *symbolTable)
{
    TFieldList *driverFieldList = DriverUniform::createUniformFields(symbolTable);

    constexpr size_t kNumGraphicsDriverUniformsMetal = 4;
    constexpr std::array<const char *, kNumGraphicsDriverUniformsMetal>
        kGraphicsDriverUniformNamesMetal = {{kHalfRenderArea, kFlipXY, kNegFlipXY, kCoverageMask}};

    const std::array<TType *, kNumGraphicsDriverUniformsMetal> kDriverUniformTypesMetal = {{
        new TType(EbtFloat, 2),  // halfRenderArea
        new TType(EbtFloat, 2),  // flipXY
        new TType(EbtFloat, 2),  // negFlipXY
        new TType(EbtUInt),      // kCoverageMask
    }};

    for (size_t uniformIndex = 0; uniformIndex < kNumGraphicsDriverUniformsMetal; ++uniformIndex)
    {
        TField *driverUniformField =
            new TField(kDriverUniformTypesMetal[uniformIndex],
                       ImmutableString(kGraphicsDriverUniformNamesMetal[uniformIndex]),
                       TSourceLoc(), SymbolType::AngleInternal);
        driverFieldList->push_back(driverUniformField);
    }

    return driverFieldList;
}

TIntermBinary *DriverUniformMetal::getHalfRenderAreaRef() const
{
    return createDriverUniformRef(kHalfRenderArea);
}

TIntermBinary *DriverUniformMetal::getFlipXYRef() const
{
    return createDriverUniformRef(kFlipXY);
}

TIntermBinary *DriverUniformMetal::getNegFlipXYRef() const
{
    return createDriverUniformRef(kNegFlipXY);
}

TIntermSwizzle *DriverUniformMetal::getNegFlipYRef() const
{
    // Create a swizzle to "negFlipXY.y"
    TIntermBinary *negFlipXY    = createDriverUniformRef(kNegFlipXY);
    TVector<int> swizzleOffsetY = {1};
    TIntermSwizzle *negFlipY    = new TIntermSwizzle(negFlipXY, swizzleOffsetY);
    return negFlipY;
}

TIntermBinary *DriverUniformMetal::getCoverageMaskFieldRef() const
{
    return createDriverUniformRef(kCoverageMask);
}

TranslatorMetal::TranslatorMetal(sh::GLenum type, ShShaderSpec spec) : TranslatorVulkan(type, spec)
{}

bool TranslatorMetal::translate(TIntermBlock *root,
                                ShCompileOptions compileOptions,
                                PerformanceDiagnostics *perfDiagnostics)
{
    TInfoSinkBase sink;

    SpecConstMetal specConst(&getSymbolTable(), compileOptions, getShaderType());
    DriverUniformMetal driverUniforms;
    if (!TranslatorVulkan::translateImpl(sink, root, compileOptions, perfDiagnostics, &specConst,
                                         &driverUniforms))
    {
        return false;
    }

    // Replace array of matrix varyings
    if (!ReplaceArrayOfMatrixVaryings(this, root, &getSymbolTable()))
    {
        return false;
    }

    if (getShaderType() == GL_VERTEX_SHADER)
    {
        TIntermTyped *negFlipY = driverUniforms.getNegFlipYRef();

        // Append gl_Position.y correction to main
        if (!AppendVertexShaderPositionYCorrectionToMain(this, root, &getSymbolTable(), negFlipY))
        {
            return false;
        }

        // Insert rasterizer discard logic
        if (!insertRasterizerDiscardLogic(sink, root))
        {
            return false;
        }
    }
    else if (getShaderType() == GL_FRAGMENT_SHADER)
    {
        // For non void MSL fragment functions replace discard
        // with ANGLEDiscardWrapper()
        if (mOutputVariables.size() > 0)
        {
            if (!replaceAllMainDiscardUses(root))
            {
                return false;
            }
        }
        if (!insertSampleMaskWritingLogic(sink, root, &driverUniforms))
        {
            return false;
        }
    }

    // Initialize unused varying outputs to avoid spirv-cross dead-code removing them in later
    // stage. Only do this if SH_INIT_OUTPUT_VARIABLES is not specified.
    if ((getShaderType() == GL_VERTEX_SHADER || getShaderType() == GL_GEOMETRY_SHADER_EXT) &&
        (compileOptions & SH_INIT_OUTPUT_VARIABLES) == 0)
    {
        InitVariableList list;
        for (const sh::ShaderVariable &var : mOutputVaryings)
        {
            if (!var.active)
            {
                list.push_back(var);
            }
        }

        if (!InitializeUnusedOutputs(root, &getSymbolTable(), list))
        {
            return false;
        }
    }

    // Write translated shader.
    TOutputVulkanGLSL outputGLSL(sink, getArrayIndexClampingStrategy(), getHashFunction(),
                                 getNameMap(), &getSymbolTable(), getShaderType(),
                                 getShaderVersion(), getOutputType(), false, true, compileOptions);
    root->traverse(&outputGLSL);

    return compileToSpirv(sink);
}

// Metal needs to inverse the depth if depthRange is is reverse order, i.e. depth near > depth far
// This is achieved by multiply the depth value with scale value stored in
// driver uniform's depthRange.reserved
bool TranslatorMetal::transformDepthBeforeCorrection(TIntermBlock *root,
                                                     const DriverUniform *driverUniforms)
{
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.z"
    TVector<int> swizzleOffsetZ = {2};
    TIntermSwizzle *positionZ   = new TIntermSwizzle(positionRef, swizzleOffsetZ);

    // Create a ref to "depthRange.reserved"
    TIntermBinary *viewportZScale = driverUniforms->getDepthRangeReservedFieldRef();

    // Create the expression "gl_Position.z * depthRange.reserved".
    TIntermBinary *zScale = new TIntermBinary(EOpMul, positionZ->deepCopy(), viewportZScale);

    // Create the assignment "gl_Position.z = gl_Position.z * depthRange.reserved"
    TIntermTyped *positionZLHS = positionZ->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionZLHS, zScale);

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(this, root, assignment, &getSymbolTable());
}

void TranslatorMetal::createAdditionalGraphicsDriverUniformFields(std::vector<TField *> *fieldsOut)
{
    // Add coverage mask to driver uniform. Metal doesn't have built-in GL_SAMPLE_COVERAGE_VALUE
    // equivalent functionality, needs to emulate it using fragment shader's [[sample_mask]] output
    // value.
    TField *coverageMaskField =
        new TField(new TType(EbtUInt), kCoverageMaskField, TSourceLoc(), SymbolType::AngleInternal);
    fieldsOut->push_back(coverageMaskField);

    if (mEmulatedInstanceID)
    {
        TField *emuInstanceIDField = new TField(new TType(EbtInt), kEmuInstanceIDField,
                                                TSourceLoc(), SymbolType::AngleInternal);
        fieldsOut->push_back(emuInstanceIDField);
    }
}

// Add sample_mask writing to main, guarded by the specialization constant
// kCoverageMaskEnabledConstName
ANGLE_NO_DISCARD bool TranslatorMetal::insertSampleMaskWritingLogic(
    TInfoSinkBase &sink,
    TIntermBlock *root,
    const DriverUniformMetal *driverUniforms)
{
    // This transformation leaves the tree in an inconsistent state by using a variable that's
    // defined in text, outside of the knowledge of the AST.
    mValidateASTOptions.validateVariableReferences = false;

    TSymbolTable *symbolTable = &getSymbolTable();

    // Insert coverageMaskEnabled specialization constant and sample_mask writing function.
    sink << "layout (constant_id=0) const bool " << mtl::kCoverageMaskEnabledConstName;
    sink << " = false;\n";
    sink << "void " << kSampleMaskWriteFuncName << "(uint mask)\n";
    sink << "{\n";
    sink << "   if (" << mtl::kCoverageMaskEnabledConstName << ")\n";
    sink << "   {\n";
    sink << "       gl_SampleMask[0] = int(mask);\n";
    sink << "   }\n";
    sink << "}\n";

    // Create kCoverageMaskEnabledConstName and kSampleMaskWriteFuncName variable references.
    TType *boolType = new TType(EbtBool);
    boolType->setQualifier(EvqConst);
    TVariable *coverageMaskEnabledVar =
        new TVariable(symbolTable, ImmutableString(mtl::kCoverageMaskEnabledConstName), boolType,
                      SymbolType::AngleInternal);

    TFunction *sampleMaskWriteFunc =
        new TFunction(symbolTable, kSampleMaskWriteFuncName, SymbolType::AngleInternal,
                      StaticType::GetBasic<EbtVoid>(), false);

    TType *uintType = new TType(EbtUInt);
    TVariable *maskArg =
        new TVariable(symbolTable, ImmutableString("mask"), uintType, SymbolType::AngleInternal);
    sampleMaskWriteFunc->addParameter(maskArg);

    // coverageMask
    TIntermBinary *coverageMask = driverUniforms->getCoverageMaskFieldRef();

    // Insert this code to the end of main()
    // if (ANGLECoverageMaskEnabled)
    // {
    //      ANGLEWriteSampleMask(ANGLEUniforms.coverageMask);
    // }
    TIntermSequence args;
    args.push_back(coverageMask);
    TIntermAggregate *callSampleMaskWriteFunc =
        TIntermAggregate::CreateFunctionCall(*sampleMaskWriteFunc, &args);
    TIntermBlock *callBlock = new TIntermBlock;
    callBlock->appendStatement(callSampleMaskWriteFunc);

    TIntermSymbol *coverageMaskEnabled = new TIntermSymbol(coverageMaskEnabledVar);
    TIntermIfElse *ifCall              = new TIntermIfElse(coverageMaskEnabled, callBlock, nullptr);

    return RunAtTheEndOfShader(this, root, ifCall, symbolTable);
}

ANGLE_NO_DISCARD bool TranslatorMetal::insertRasterizerDiscardLogic(TInfoSinkBase &sink,
                                                                    TIntermBlock *root)
{
    // This transformation leaves the tree in an inconsistent state by using a variable that's
    // defined in text, outside of the knowledge of the AST.
    mValidateASTOptions.validateVariableReferences = false;

    TSymbolTable *symbolTable = &getSymbolTable();

    // Insert rasterizationDisabled specialization constant.
    sink << "layout (constant_id=0) const bool " << mtl::kRasterizerDiscardEnabledConstName;
    sink << " = false;\n";

    // Create kRasterizerDiscardEnabledConstName variable reference.
    TType *boolType = new TType(EbtBool);
    boolType->setQualifier(EvqConst);
    TVariable *discardEnabledVar =
        new TVariable(symbolTable, ImmutableString(mtl::kRasterizerDiscardEnabledConstName),
                      boolType, SymbolType::AngleInternal);

    // Insert this code to the end of main()
    // if (ANGLERasterizerDisabled)
    // {
    //      gl_Position = vec4(-3.0, -3.0, -3.0, 1.0);
    // }
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create vec4(-3, -3, -3, 1):
    auto vec4Type = new TType(EbtFloat, 4);
    TIntermSequence vec4Args;
    vec4Args.push_back(CreateFloatNode(-3.0f));
    vec4Args.push_back(CreateFloatNode(-3.0f));
    vec4Args.push_back(CreateFloatNode(-3.0f));
    vec4Args.push_back(CreateFloatNode(1.0f));
    TIntermAggregate *constVarConstructor =
        TIntermAggregate::CreateConstructor(*vec4Type, &vec4Args);

    // Create the assignment "gl_Position = vec4(-3, -3, -3, 1)"
    TIntermBinary *assignment =
        new TIntermBinary(TOperator::EOpAssign, positionRef->deepCopy(), constVarConstructor);

    TIntermBlock *discardBlock = new TIntermBlock;
    discardBlock->appendStatement(assignment);

    TIntermSymbol *discardEnabled = new TIntermSymbol(discardEnabledVar);
    TIntermIfElse *ifCall         = new TIntermIfElse(discardEnabled, discardBlock, nullptr);

    return RunAtTheEndOfShader(this, root, ifCall, symbolTable);
}

// If the MSL fragment shader is non-void, we need to ensure
// that there is a return at the end. The MSL compiler
// will error out if there's no return at the end, even if all
// paths lead to discard_fragment(). So wrap discard in a wrapper function.
// Fixes dEQP-GLES3.functional.shaders.discard.basic_always
ANGLE_NO_DISCARD bool TranslatorMetal::replaceAllMainDiscardUses(TIntermBlock *root)
{
    // before
    // void main (void)
    // {
    //     o_color = v_color;
    //     discard;
    // }

    // after
    // void ANGLEDiscardWrapper()
    // {
    //    discard;
    // }
    // void main (void)
    // {
    //     o_color = v_color;
    //     ANGLEDiscardWrapper();
    // }

    TIntermFunctionDefinition *main   = FindMain(root);
    TIntermBlock *mainBody            = main->getBody();
    TIntermSequence *functionSequence = mainBody->getSequence();
    TIntermAggregate *discardFunc     = nullptr;
    // Iterate over all branches in main function, and replace all uses
    // of discard with ANGLEDiscardWrapper().
    for (size_t index = 0; index < functionSequence->size(); ++index)
    {
        TIntermNode *functionNode = (*functionSequence)[index];
        TIntermBranch *branchNode = functionNode->getAsBranchNode();
        if (branchNode != nullptr)
        {
            if (branchNode->getFlowOp() == EOpKill)
            {
                if (discardFunc == nullptr)
                {
                    discardFunc = createDiscardWrapperFunc(root);
                }
                bool replaced = mainBody->replaceChildNode(branchNode, discardFunc);
                if (!replaced)
                {
                    return false;
                }
            }
        }
    }
    return validateAST(root);
}

// Create discard_wrapper function to ensure that SPIRV-Cross will always have a return at the end
// of fragment shaders
ANGLE_NO_DISCARD TIntermAggregate *TranslatorMetal::createDiscardWrapperFunc(TIntermBlock *root)
{
    TInfoSinkBase &sink       = getInfoSink().obj;
    TSymbolTable *symbolTable = &getSymbolTable();

    sink << "void " << kDiscardWrapperFuncName << "()\n";
    sink << "{\n";
    sink << "   discard;\n";
    sink << "}\n";

    TFunction *discardWrapperFunc =
        new TFunction(symbolTable, ImmutableString(kDiscardWrapperFuncName),
                      SymbolType::AngleInternal, StaticType::GetBasic<EbtVoid>(), true);

    TIntermAggregate *callDiscardWrapperFunc =
        TIntermAggregate::CreateFunctionCall(*discardWrapperFunc, new TIntermSequence());
    return callDiscardWrapperFunc;
}

}  // namespace sh
