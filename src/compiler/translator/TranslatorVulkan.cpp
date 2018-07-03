//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorVulkan:
//   A GLSL-based translator that outputs shaders that fit GL_KHR_vulkan_glsl.
//   The shaders are then fed into glslang to spit out SPIR-V (libANGLE-side).
//   See: https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
//

#include "compiler/translator/TranslatorVulkan.h"

#include "angle_gl.h"
#include "common/utilities.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/OutputVulkanGLSL.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/tree_ops/NameEmbeddedUniformStructs.h"
#include "compiler/translator/tree_ops/RewriteStructSamplers.h"
#include "compiler/translator/tree_util/BuiltIn_autogen.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{
// This traverses nodes, find the struct ones and add their declarations to the sink. It also
// removes the nodes from the tree as it processes them.
class DeclareStructTypesTraverser : public TIntermTraverser
{
  public:
    explicit DeclareStructTypesTraverser(TOutputVulkanGLSL *outputVulkanGLSL)
        : TIntermTraverser(true, false, false), mOutputVulkanGLSL(outputVulkanGLSL)
    {
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        ASSERT(visit == PreVisit);

        if (!mInGlobalScope)
        {
            return false;
        }

        const TIntermSequence &sequence = *(node->getSequence());
        TIntermTyped *declarator        = sequence.front()->getAsTyped();
        const TType &type               = declarator->getType();

        if (type.isStructSpecifier())
        {
            const TStructure *structure = type.getStruct();

            // Embedded structs should be parsed away by now.
            ASSERT(structure->symbolType() != SymbolType::Empty);
            mOutputVulkanGLSL->writeStructType(structure);

            TIntermSymbol *symbolNode = declarator->getAsSymbolNode();
            if (symbolNode && symbolNode->variable().symbolType() == SymbolType::Empty)
            {
                // Remove the struct specifier declaration from the tree so it isn't parsed again.
                TIntermSequence emptyReplacement;
                mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                                emptyReplacement);
            }
        }

        return false;
    }

  private:
    TOutputVulkanGLSL *mOutputVulkanGLSL;
};

class DeclareDefaultUniformsTraverser : public TIntermTraverser
{
  public:
    DeclareDefaultUniformsTraverser(TInfoSinkBase *sink,
                                    ShHashFunction64 hashFunction,
                                    NameMap *nameMap)
        : TIntermTraverser(true, true, true),
          mSink(sink),
          mHashFunction(hashFunction),
          mNameMap(nameMap),
          mInDefaultUniform(false)
    {
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        // TODO(jmadill): Compound declarations.
        ASSERT(sequence.size() == 1);

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isUniform = (type.getQualifier() == EvqUniform) && !IsOpaqueType(type.getBasicType());

        if (visit == PreVisit)
        {
            if (isUniform)
            {
                (*mSink) << "    " << GetTypeName(type, mHashFunction, mNameMap) << " ";
                mInDefaultUniform = true;
            }
        }
        else if (visit == InVisit)
        {
            mInDefaultUniform = isUniform;
        }
        else if (visit == PostVisit)
        {
            if (isUniform)
            {
                (*mSink) << ";\n";

                // Remove the uniform declaration from the tree so it isn't parsed again.
                TIntermSequence emptyReplacement;
                mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                                emptyReplacement);
            }

            mInDefaultUniform = false;
        }
        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (mInDefaultUniform)
        {
            const ImmutableString &name = symbol->variable().name();
            ASSERT(!name.beginsWith("gl_"));
            (*mSink) << HashName(name, mHashFunction, mNameMap) << ArrayString(symbol->getType());
        }
    }

  private:
    TInfoSinkBase *mSink;
    ShHashFunction64 mHashFunction;
    NameMap *mNameMap;
    bool mInDefaultUniform;
};

constexpr ImmutableString kFlippedPointCoordName = ImmutableString("flippedPointCoord");

// Declares a new variable to replace gl_PointCoord with a version that is flipping the Y
// coordinate.
void FlipGLPointCoord(TIntermBlock *root, TSymbolTable *symbolTable)
{
    // Create a symbol reference to "gl_PointCoord"
    const TVariable *pointCoord  = BuiltInVariable::gl_PointCoord();
    TIntermSymbol *pointCoordRef = new TIntermSymbol(pointCoord);

    // Create a swizzle to "gl_PointCoord.x"
    TVector<int> swizzleOffsetX;
    swizzleOffsetX.push_back(0);
    TIntermSwizzle *pointCoordX = new TIntermSwizzle(pointCoordRef, swizzleOffsetX);

    // Create a swizzle to "gl_PointCoord.y"
    TVector<int> swizzleOffsetY;
    swizzleOffsetY.push_back(1);
    TIntermSwizzle *pointCoordY = new TIntermSwizzle(pointCoordRef, swizzleOffsetY);

    // Create a symbol reference to our new variable that will hold the modified gl_PointCoord.
    TVariable *replacementVar =
        new TVariable(symbolTable, kFlippedPointCoordName,
                      StaticType::Helpers::GetForVecMatHelper<EbtFloat, EbpMedium, EvqGlobal, 1>(2),
                      SymbolType::UserDefined);
    DeclareGlobalVariable(root, replacementVar);
    TIntermSymbol *flippedPointCoordsRef = new TIntermSymbol(replacementVar);

    // Create a constant "-1.0"
    const TType *constantType             = StaticType::GetBasic<EbtFloat>();
    TConstantUnion *constantValueMinusOne = new TConstantUnion();
    constantValueMinusOne->setFConst(-1.0f);
    TIntermConstantUnion *minusOne = new TIntermConstantUnion(constantValueMinusOne, *constantType);

    // Create a constant "1.0"
    TConstantUnion *constantValueOne = new TConstantUnion();
    constantValueOne->setFConst(1.0f);
    TIntermConstantUnion *one = new TIntermConstantUnion(constantValueOne, *constantType);

    // Create the expression "gl_PointCoord.y * -1.0 + 1.0"
    TIntermBinary *inverseY = new TIntermBinary(EOpMul, pointCoordY, minusOne);
    TIntermBinary *plusOne  = new TIntermBinary(EOpAdd, inverseY, one);

    // Create the new vec2 using the modified Y
    TIntermSequence *sequence = new TIntermSequence();
    sequence->push_back(pointCoordX);
    sequence->push_back(plusOne);
    TIntermAggregate *aggregate =
        TIntermAggregate::CreateConstructor(BuiltInVariable::gl_PointCoord()->getType(), sequence);

    // Use this new variable instead of gl_PointCoord everywhere.
    ReplaceVariable(root, pointCoord, replacementVar);

    // Assign this new value to flippedPointCoord
    TIntermBinary *assignment = new TIntermBinary(EOpInitialize, flippedPointCoordsRef, aggregate);

    // Add this assigment at the beginning of the main function
    TIntermFunctionDefinition *main = FindMain(root);
    TIntermSequence *mainSequence   = main->getBody()->getSequence();
    mainSequence->insert(mainSequence->begin(), assignment);
}

TIntermConstantUnion *CreateConstantOneHalf()
{
    const TType *constantType     = StaticType::GetBasic<TBasicType::EbtFloat>();
    TConstantUnion *constantValue = new TConstantUnion();
    constantValue->setFConst(0.5f);
    return new TIntermConstantUnion(constantValue, *constantType);
}

// This operation performs the viewport depth translation needed by Vulkan. In GL the viewport
// transformation is slightly different - see the GL 2.0 spec section "2.12.1 Controlling the
// Viewport". In Vulkan the corresponding spec section is currently "23.4. Coordinate
// Transformations".
// The equations reduce to an expression:
//
//     z_vk = w_gl * (0.5 * z_gl + 0.5)
//
// where z_vk is the depth output of a Vulkan vertex shader and z_gl is the same for GL.
void AppendVertexShaderDepthCorrectionToMain(TIntermBlock *root, TSymbolTable *symbolTable)
{
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.z"
    TVector<int> swizzleOffsetZ;
    swizzleOffsetZ.push_back(2);
    TIntermSwizzle *positionZ = new TIntermSwizzle(positionRef, swizzleOffsetZ);

    // Create a constant "0.5"
    TIntermConstantUnion *oneHalf = CreateConstantOneHalf();

    // Create the expression "gl_Position.z * 0.5 + 0.5"
    TIntermBinary *halfZ         = new TIntermBinary(TOperator::EOpMul, positionZ, oneHalf);
    TIntermBinary *halfZPlusHalf = new TIntermBinary(TOperator::EOpAdd, halfZ, oneHalf->deepCopy());

    // Create a swizzle to "gl_Position.w"
    TVector<int> swizzleOffsetW;
    swizzleOffsetW.push_back(3);
    TIntermSwizzle *positionW = new TIntermSwizzle(positionRef->deepCopy(), swizzleOffsetW);

    // Create the expression "gl_Position.w * (gl_Position.z * 0.5 + 0.5)"
    TIntermBinary *vulkanZ = new TIntermBinary(TOperator::EOpMul, positionW, halfZPlusHalf);

    // Create the assignment "gl_Position.z = gl_Position.w * (gl_Position.z * 0.5 + 0.5)"
    TIntermTyped *positionZLHS = positionZ->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionZLHS, vulkanZ);

    // Append the assignment as a statement at the end of the shader.
    RunAtTheEndOfShader(root, assignment, symbolTable);
}

TIntermRaw *GenerateLineRasterIfDef()
{
    const TType *voidType = StaticType::GetBasic<EbtVoid>();
    return new TIntermRaw(*voidType,
                          ImmutableString("#ifdef ANGLE_ENABLE_LINE_SEGMENT_RASTERIZATION"));
}

TIntermRaw *GenerateEndIf()
{
    const TType *voidType = StaticType::GetBasic<EbtVoid>();
    return new TIntermRaw(*voidType, ImmutableString("#endif"));
}

TVariable *AddANGLEPositionVaryingDeclaration(TIntermBlock *root, TSymbolTable *symbolTable)
{
    TIntermSequence *insertSequence = new TIntermSequence;

    insertSequence->push_back(GenerateLineRasterIfDef());

    // Define a driver varying vec2 "ANGLEPosition".
    TType *varyingType               = new TType(EbtFloat, EbpMedium, EvqVaryingIn, 2);
    TVariable *varyingVar            = new TVariable(symbolTable, ImmutableString("ANGLEPosition"),
                                          varyingType, SymbolType::AngleInternal);
    TIntermSymbol *varyingDeclarator = new TIntermSymbol(varyingVar);
    TIntermDeclaration *varyingDecl  = new TIntermDeclaration;
    varyingDecl->appendDeclarator(varyingDeclarator);
    insertSequence->push_back(varyingDecl);

    insertSequence->push_back(GenerateEndIf());

    // Insert the declarations before Main.
    size_t mainIndex = FindMainIndex(root);
    root->insertChildNodes(mainIndex, *insertSequence);

    return varyingVar;
}

void AddANGLEPositionVarying(TIntermBlock *root, TSymbolTable *symbolTable)
{
    TVariable *anglePosition = AddANGLEPositionVaryingDeclaration(root, symbolTable);

    // Add a reference to "gl_Position".
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.xy"
    TVector<int> swizzleOffsetXY;
    swizzleOffsetXY.push_back(0);
    swizzleOffsetXY.push_back(1);
    TIntermSwizzle *positionXY = new TIntermSwizzle(positionRef, swizzleOffsetXY);

    // Add a reference to "ANGLEPosition".
    TIntermSymbol *varyingRef = new TIntermSymbol(anglePosition);

    // Create an assignment "ANGLEPosition = gl_Position.xy".
    TIntermBinary *assignment = new TIntermBinary(EOpAssign, varyingRef, positionXY);

    // Ensure the assignment runs at the end of the main() function.
    TIntermFunctionDefinition *main = FindMain(root);
    TIntermBlock *mainBody          = main->getBody();
    mainBody->appendStatement(GenerateLineRasterIfDef());
    mainBody->appendStatement(assignment);
    mainBody->appendStatement(GenerateEndIf());
}

// This block adds line segment rasterization behind #ifdef guards.
// The emulatation code looks like the following:
//
// void main()
// {
//     vec2 b = ((gl_Position * 0.5) + 0.5) * gl_Viewport.zw + gl_Viewport.xy;
//     vec2 ba = b - gl_FragCoord.xy;
//     vec2 bp = ba + (ba.yx * ba.yx / ba);
//     vec2 absbp = abs(bp);
//     if (absbp.x > 0.5 && absbp.y > 0.5)
//         discard;
//     <otherwise run fragment shader main>
// }
void AddLineSegmentRasterizationEmulation(TInfoSinkBase &sink,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable)
{
    TVariable *anglePosition = AddANGLEPositionVaryingDeclaration(root, symbolTable);

    // Define a driver uniform block "ANGLEUniforms" with a vec4 member "viewport".
    TFieldList *driverFieldList = new TFieldList;
    TType *driverViewportType   = new TType(EbtFloat, 4);
    TField *driverViewportSize  = new TField(driverViewportType, ImmutableString("viewport"),
                                            TSourceLoc(), SymbolType::AngleInternal);
    driverFieldList->push_back(driverViewportSize);
    TLayoutQualifier driverLayoutQualifier = TLayoutQualifier::Create();
    TInterfaceBlock *interfaceBlock =
        new TInterfaceBlock(symbolTable, ImmutableString("ANGLEUniformBlock"), driverFieldList,
                            driverLayoutQualifier, SymbolType::AngleInternal);

    // Make the inteface block into a declaration.
    TType *interfaceBlockType = new TType(interfaceBlock, EvqUniform, driverLayoutQualifier);
    TIntermDeclaration *driverUniformsDecl = new TIntermDeclaration;
    TVariable *driverUniformsVar = new TVariable(symbolTable, ImmutableString("ANGLEUniforms"),
                                                 interfaceBlockType, SymbolType::AngleInternal);
    TIntermSymbol *driverUniformsDeclarator = new TIntermSymbol(driverUniformsVar);
    driverUniformsDecl->appendDeclarator(driverUniformsDeclarator);

    TIntermSequence *insertSequence = new TIntermSequence;
    insertSequence->push_back(GenerateLineRasterIfDef());
    insertSequence->push_back(driverUniformsDecl);
    insertSequence->push_back(GenerateEndIf());

    // Insert the declarations before Main.
    size_t mainIndex = FindMainIndex(root);
    root->insertChildNodes(mainIndex, *insertSequence);

    // ANGLEUniforms.viewport
    TIntermSymbol *angleUniformsRef  = new TIntermSymbol(driverUniformsVar);
    TConstantUnion *viewportConstant = new TConstantUnion;
    viewportConstant->setIConst(0);
    TIntermConstantUnion *viewportIndex =
        new TIntermConstantUnion(viewportConstant, *StaticType::GetBasic<EbtInt>());
    TIntermBinary *viewportRef =
        new TIntermBinary(EOpIndexDirectInterfaceBlock, angleUniformsRef, viewportIndex);

    // Create a swizzle to "ANGLEUniforms.viewport.xy".
    TVector<int> swizzleOffsetXY;
    swizzleOffsetXY.push_back(0);
    swizzleOffsetXY.push_back(1);
    TIntermSwizzle *viewportXY = new TIntermSwizzle(viewportRef->deepCopy(), swizzleOffsetXY);

    // Create a swizzle to "ANGLEUniforms.viewport.zw".
    TVector<int> swizzleOffsetZW;
    swizzleOffsetZW.push_back(2);
    swizzleOffsetZW.push_back(3);
    TIntermSwizzle *viewportZW = new TIntermSwizzle(viewportRef, swizzleOffsetZW);

    // ANGLEPosition * 0.5
    TIntermSymbol *positionRef    = new TIntermSymbol(anglePosition);
    TIntermConstantUnion *oneHalf = CreateConstantOneHalf();
    TIntermBinary *halfPosition   = new TIntermBinary(EOpVectorTimesScalar, positionRef, oneHalf);

    // (ANGLEPosition * 0.5) + 0.5
    TIntermBinary *offsetHalfPosition =
        new TIntermBinary(EOpAdd, halfPosition, oneHalf->deepCopy());

    // ((ANGLEPosition * 0.5) + 0.5) * ANGLEUniforms.viewport.zw
    TIntermBinary *scaledPosition = new TIntermBinary(EOpMul, offsetHalfPosition, viewportZW);

    // ((ANGLEPosition * 0.5) + 0.5) * ANGLEUniforms.viewport + ANGLEUniforms.viewport.xy
    TIntermBinary *windowPosition = new TIntermBinary(EOpAdd, scaledPosition, viewportXY);

    // Assign to a temporary "b".
    TVariable *bTemp = CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, 2>());
    TIntermDeclaration *bDecl = CreateTempInitDeclarationNode(bTemp, windowPosition);

    // gl_FragCoord.xy
    const TVariable *fragCoord  = BuiltInVariable::gl_FragCoord();
    TIntermSymbol *fragCoordRef = new TIntermSymbol(fragCoord);
    TIntermSwizzle *fragCoordXY = new TIntermSwizzle(fragCoordRef, swizzleOffsetXY);

    // b - gl_FragCoord.xy
    TIntermSymbol *bRef           = CreateTempSymbolNode(bTemp);
    TIntermBinary *differenceExpr = new TIntermBinary(EOpSub, bRef, fragCoordXY);

    // Assign to a temporary "ba".
    TVariable *baTemp = CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, 2>());
    TIntermDeclaration *baDecl = CreateTempInitDeclarationNode(baTemp, differenceExpr);

    // Create a swizzle to "ba.yx".
    TVector<int> swizzleOffsetYX;
    swizzleOffsetYX.push_back(1);
    swizzleOffsetYX.push_back(0);
    TIntermSymbol *baRef = CreateTempSymbolNode(baTemp);
    TIntermSwizzle *baYX = new TIntermSwizzle(baRef, swizzleOffsetYX);

    // ba.yx * ba.yx
    TIntermBinary *baSquared = new TIntermBinary(EOpMul, baYX, baYX->deepCopy());

    // ba.yx * ba.yx / ba
    TIntermBinary *baSqNorm = new TIntermBinary(EOpDiv, baSquared, baRef->deepCopy());

    // ba + (ba.yx * ba.yx / ba)
    TIntermBinary *bpInit = new TIntermBinary(EOpAdd, baRef->deepCopy(), baSqNorm);

    // Assign to a temporary "bp".
    TVariable *bpTemp = CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, 2>());
    TIntermDeclaration *bpDecl = CreateTempInitDeclarationNode(bpTemp, bpInit);

    // Assign abs(bp) to a temporary "absbp".
    TIntermSymbol *bpRef = CreateTempSymbolNode(bpTemp);
    TIntermUnary *bpAbs  = new TIntermUnary(EOpAbs, bpRef, nullptr);
    TVariable *bpAbsTemp = CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, 2>());
    TIntermDeclaration *bpAbsDecl = CreateTempInitDeclarationNode(bpAbsTemp, bpAbs);

    // Create a swizzle to "absbp.x".
    TVector<int> swizzleOffsetX;
    swizzleOffsetX.push_back(0);
    TIntermSymbol *bpAbsRef = CreateTempSymbolNode(bpAbsTemp);
    TIntermSwizzle *bpAbsX  = new TIntermSwizzle(bpAbsRef, swizzleOffsetX);

    // absbp.x > 0.5
    TIntermBinary *xGreaterHalf = new TIntermBinary(EOpGreaterThan, bpAbsX, oneHalf->deepCopy());

    // Create a swizzle to "absbp.y".
    TVector<int> swizzleOffsetY;
    swizzleOffsetY.push_back(1);
    TIntermSwizzle *bpAbsY = new TIntermSwizzle(bpAbsRef->deepCopy(), swizzleOffsetY);

    // absbp.y > 0.5
    TIntermBinary *yGreaterHalf = new TIntermBinary(EOpGreaterThan, bpAbsY, oneHalf->deepCopy());

    // (absbp.x > 0.5) && (absbp.y > 0.5)
    TIntermBinary *bothGreaterHalf = new TIntermBinary(EOpLogicalAnd, xGreaterHalf, yGreaterHalf);

    // discard
    TIntermBranch *discard     = new TIntermBranch(EOpKill, nullptr);
    TIntermBlock *discardBlock = new TIntermBlock;
    discardBlock->appendStatement(discard);

    // if ((absbp.x > 0.5) && (absbp.y > 0.5)) discard;
    TIntermIfElse *ifStatement = new TIntermIfElse(bothGreaterHalf, discardBlock, nullptr);

    // Ensure the line raster code runs at the beginning of main().
    TIntermFunctionDefinition *main = FindMain(root);
    TIntermSequence *mainSequence   = main->getBody()->getSequence();
    ASSERT(mainSequence);

    std::array<TIntermNode *, 7> statements = {{GenerateLineRasterIfDef(), bDecl, baDecl, bpDecl,
                                                bpAbsDecl, ifStatement, GenerateEndIf()}};

    mainSequence->insert(mainSequence->begin(), statements.begin(), statements.end());
}
}  // anonymous namespace

TranslatorVulkan::TranslatorVulkan(sh::GLenum type, ShShaderSpec spec)
    : TCompiler(type, spec, SH_GLSL_450_CORE_OUTPUT)
{
}

void TranslatorVulkan::translate(TIntermBlock *root,
                                 ShCompileOptions compileOptions,
                                 PerformanceDiagnostics * /*perfDiagnostics*/)
{
    TInfoSinkBase &sink = getInfoSink().obj;
    TOutputVulkanGLSL outputGLSL(sink, getArrayIndexClampingStrategy(), getHashFunction(),
                                 getNameMap(), &getSymbolTable(), getShaderType(),
                                 getShaderVersion(), getOutputType(), compileOptions);

    sink << "#version 450 core\n";

    // Write out default uniforms into a uniform block assigned to a specific set/binding.
    int defaultUniformCount        = 0;
    int structTypesUsedForUniforms = 0;
    for (const auto &uniform : getUniforms())
    {
        if (!uniform.isBuiltIn() && uniform.staticUse && !gl::IsOpaqueType(uniform.type))
        {
            ++defaultUniformCount;
        }

        if (uniform.isStruct())
        {
            ++structTypesUsedForUniforms;
        }
    }

    // TODO(lucferron): Refactor this function to do less tree traversals.
    // http://anglebug.com/2461
    if (structTypesUsedForUniforms > 0)
    {
        NameEmbeddedStructUniforms(root, &getSymbolTable());

        defaultUniformCount -= RewriteStructSamplers(root, &getSymbolTable());

        // We must declare the struct types before using them.
        DeclareStructTypesTraverser structTypesTraverser(&outputGLSL);
        root->traverse(&structTypesTraverser);
        structTypesTraverser.updateTree();
    }

    if (defaultUniformCount > 0)
    {
        sink << "\nlayout(@@ DEFAULT-UNIFORMS-SET-BINDING @@) uniform defaultUniforms\n{\n";

        DeclareDefaultUniformsTraverser defaultTraverser(&sink, getHashFunction(), &getNameMap());
        root->traverse(&defaultTraverser);
        defaultTraverser.updateTree();

        sink << "};\n";
    }

    // Declare gl_FragColor and glFragData as webgl_FragColor and webgl_FragData
    // if it's core profile shaders and they are used.
    if (getShaderType() == GL_FRAGMENT_SHADER)
    {
        AddLineSegmentRasterizationEmulation(sink, root, &getSymbolTable());

        bool hasGLFragColor = false;
        bool hasGLFragData  = false;

        for (const OutputVariable &outputVar : outputVariables)
        {
            if (outputVar.name == "gl_FragColor")
            {
                ASSERT(!hasGLFragColor);
                hasGLFragColor = true;
                continue;
            }
            else if (outputVar.name == "gl_FragData")
            {
                ASSERT(!hasGLFragData);
                hasGLFragData = true;
                continue;
            }
        }
        ASSERT(!(hasGLFragColor && hasGLFragData));
        if (hasGLFragColor)
        {
            sink << "layout(location = 0) out vec4 webgl_FragColor;\n";
        }
        if (hasGLFragData)
        {
            sink << "layout(location = 0) out vec4 webgl_FragData[gl_MaxDrawBuffers];\n";
        }

        // Search for the gl_PointCoord usage, if its used, we need to flip the y coordinate.
        for (const Varying &inputVarying : inputVaryings)
        {
            if (!inputVarying.isBuiltIn())
            {
                continue;
            }

            if (inputVarying.name == "gl_PointCoord")
            {
                FlipGLPointCoord(root, &getSymbolTable());
                break;
            }
        }
    }
    else
    {
        ASSERT(getShaderType() == GL_VERTEX_SHADER);

        AddANGLEPositionVarying(root, &getSymbolTable());

        // Append depth range translation to main.
        AppendVertexShaderDepthCorrectionToMain(root, &getSymbolTable());
    }

    // Write translated shader.
    root->traverse(&outputGLSL);
}

bool TranslatorVulkan::shouldFlattenPragmaStdglInvariantAll()
{
    // Not necessary.
    return false;
}

}  // namespace sh
