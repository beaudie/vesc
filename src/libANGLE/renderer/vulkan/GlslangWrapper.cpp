//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangWrapper: Wrapper for Vulkan's glslang compiler.
//

#include "libANGLE/renderer/vulkan/GlslangWrapper.h"

// glslang's version of ShaderLang.h, not to be confused with ANGLE's.
// Our function defs conflict with theirs, but we carefully manage our includes to prevent this.
#include <ShaderLang.h>

// Other glslang includes.
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>

#include <array>

#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libaNGLE/renderer/vulkan/FeaturesVk.h"

namespace rx
{
bool IsLineMode(gl::PrimitiveMode primitiveMode)
{
    switch (primitiveMode)
    {
        case gl::PrimitiveMode::LineLoop:
        case gl::PrimitiveMode::LineStrip:
        case gl::PrimitiveMode::LineStripAdjacency:
        case gl::PrimitiveMode::Lines:
            return true;

        default:
            return false;
    }
}

namespace
{
constexpr char kQualifierMarkerBegin[] = "@@ QUALIFIER-";
constexpr char kLayoutMarkerBegin[]    = "@@ LAYOUT-";
constexpr char kMarkerEnd[]            = " @@";
constexpr char kUniformQualifier[]     = "uniform";

void GetBuiltInResourcesFromCaps(const gl::Caps &caps, TBuiltInResource *outBuiltInResources)
{
    outBuiltInResources->maxDrawBuffers                   = caps.maxDrawBuffers;
    outBuiltInResources->maxAtomicCounterBindings         = caps.maxAtomicCounterBufferBindings;
    outBuiltInResources->maxAtomicCounterBufferSize       = caps.maxAtomicCounterBufferSize;
    outBuiltInResources->maxClipPlanes                    = caps.maxClipPlanes;
    outBuiltInResources->maxCombinedAtomicCounterBuffers  = caps.maxCombinedAtomicCounterBuffers;
    outBuiltInResources->maxCombinedAtomicCounters        = caps.maxCombinedAtomicCounters;
    outBuiltInResources->maxCombinedImageUniforms         = caps.maxCombinedImageUniforms;
    outBuiltInResources->maxCombinedTextureImageUnits     = caps.maxCombinedTextureImageUnits;
    outBuiltInResources->maxCombinedShaderOutputResources = caps.maxCombinedShaderOutputResources;
    outBuiltInResources->maxComputeWorkGroupCountX        = caps.maxComputeWorkGroupCount[0];
    outBuiltInResources->maxComputeWorkGroupCountY        = caps.maxComputeWorkGroupCount[1];
    outBuiltInResources->maxComputeWorkGroupCountZ        = caps.maxComputeWorkGroupCount[2];
    outBuiltInResources->maxComputeWorkGroupSizeX         = caps.maxComputeWorkGroupSize[0];
    outBuiltInResources->maxComputeWorkGroupSizeY         = caps.maxComputeWorkGroupSize[1];
    outBuiltInResources->maxComputeWorkGroupSizeZ         = caps.maxComputeWorkGroupSize[2];
    outBuiltInResources->minProgramTexelOffset            = caps.minProgramTexelOffset;
    outBuiltInResources->maxFragmentUniformVectors        = caps.maxFragmentUniformVectors;
    outBuiltInResources->maxFragmentInputComponents       = caps.maxFragmentInputComponents;
    outBuiltInResources->maxGeometryInputComponents       = caps.maxGeometryInputComponents;
    outBuiltInResources->maxGeometryOutputComponents      = caps.maxGeometryOutputComponents;
    outBuiltInResources->maxGeometryOutputVertices        = caps.maxGeometryOutputVertices;
    outBuiltInResources->maxGeometryTotalOutputComponents = caps.maxGeometryTotalOutputComponents;
    outBuiltInResources->maxLights                        = caps.maxLights;
    outBuiltInResources->maxProgramTexelOffset            = caps.maxProgramTexelOffset;
    outBuiltInResources->maxVaryingComponents             = caps.maxVaryingComponents;
    outBuiltInResources->maxVaryingVectors                = caps.maxVaryingVectors;
    outBuiltInResources->maxVertexAttribs                 = caps.maxVertexAttributes;
    outBuiltInResources->maxVertexOutputComponents        = caps.maxVertexOutputComponents;
    outBuiltInResources->maxVertexUniformVectors          = caps.maxVertexUniformVectors;
}

void InsertLayoutSpecifierString(std::string *shaderString,
                                 const std::string &variableName,
                                 const std::string &layoutString)
{
    std::stringstream searchStringBuilder;
    searchStringBuilder << kLayoutMarkerBegin << variableName << kMarkerEnd;
    std::string searchString = searchStringBuilder.str();

    if (!layoutString.empty())
    {
        angle::ReplaceSubstring(shaderString, searchString, "layout(" + layoutString + ")");
    }
    else
    {
        angle::ReplaceSubstring(shaderString, searchString, layoutString);
    }
}

void InsertQualifierSpecifierString(std::string *shaderString,
                                    const std::string &variableName,
                                    const std::string &replacementString)
{
    std::stringstream searchStringBuilder;
    searchStringBuilder << kQualifierMarkerBegin << variableName << kMarkerEnd;
    std::string searchString = searchStringBuilder.str();
    angle::ReplaceSubstring(shaderString, searchString, replacementString);
}

void EraseLayoutAndQualifierStrings(std::string *vertexSource,
                                    std::string *fragmentSource,
                                    const std::string &uniformName)
{
    InsertLayoutSpecifierString(vertexSource, uniformName, "");
    InsertLayoutSpecifierString(fragmentSource, uniformName, "");

    InsertQualifierSpecifierString(vertexSource, uniformName, "");
    InsertQualifierSpecifierString(fragmentSource, uniformName, "");
}

std::string GetMappedSamplerName(const std::string &originalName)
{
    std::string samplerName = gl::ParseResourceName(originalName, nullptr);

    // Samplers in structs are extracted.
    std::replace(samplerName.begin(), samplerName.end(), '.', '_');

    // Samplers in arrays of structs are also extracted.
    std::replace(samplerName.begin(), samplerName.end(), '[', '_');
    samplerName.erase(std::remove(samplerName.begin(), samplerName.end(), ']'), samplerName.end());
    return samplerName;
}
}  // anonymous namespace

// static
GlslangWrapper *GlslangWrapper::mInstance = nullptr;

// static
GlslangWrapper *GlslangWrapper::GetReference()
{
    if (!mInstance)
    {
        mInstance = new GlslangWrapper();
    }

    mInstance->addRef();

    return mInstance;
}

// static
void GlslangWrapper::ReleaseReference()
{
    if (mInstance->getRefCount() == 1)
    {
        mInstance->release();
        mInstance = nullptr;
    }
    else
    {
        mInstance->release();
    }
}

GlslangWrapper::GlslangWrapper()
{
    int result = ShInitialize();
    ASSERT(result != 0);
}

GlslangWrapper::~GlslangWrapper()
{
    int result = ShFinalize();
    ASSERT(result != 0);
}

gl::LinkResult GlslangWrapper::linkProgram(const gl::Context *glContext,
                                           const gl::ProgramState &programState,
                                           const gl::ProgramLinkedResources &resources,
                                           const gl::Caps &glCaps,
                                           const FeaturesVk &features,
                                           gl::PrimitiveMode drawMode,
                                           std::vector<uint32_t> *vertexCodeOut,
                                           std::vector<uint32_t> *fragmentCodeOut)
{
    gl::Shader *glVertexShader   = programState.getAttachedShader(gl::ShaderType::Vertex);
    gl::Shader *glFragmentShader = programState.getAttachedShader(gl::ShaderType::Fragment);

    std::string vertexSource   = glVertexShader->getTranslatedSource(glContext);
    std::string fragmentSource = glFragmentShader->getTranslatedSource(glContext);

    // Parse attribute locations and replace them in the vertex shader.
    // See corresponding code in OutputVulkanGLSL.cpp.
    // TODO(jmadill): Also do the same for ESSL 3 fragment outputs.
    for (const sh::Attribute &attribute : programState.getAttributes())
    {
        // Warning: If we endup supporting ES 3.0 shaders and up, Program::linkAttributes is going
        // to bring us all attributes in this list instead of only the active ones.
        ASSERT(attribute.active);

        std::string locationString = "location = " + Str(attribute.location);
        InsertLayoutSpecifierString(&vertexSource, attribute.name, locationString);
        InsertQualifierSpecifierString(&vertexSource, attribute.name, "in");
    }

    // The attributes in the programState could have been filled with active attributes only
    // depending on the shader version. If there is inactive attributes left, we have to remove
    // their @@ QUALIFIER and @@ LAYOUT markers.
    for (const sh::Attribute &attribute : glVertexShader->getAllAttributes(glContext))
    {
        if (attribute.active)
        {
            continue;
        }

        InsertLayoutSpecifierString(&vertexSource, attribute.name, "");
        InsertQualifierSpecifierString(&vertexSource, attribute.name, "");
    }

    // Assign varying locations.
    for (const gl::PackedVaryingRegister &varyingReg : resources.varyingPacking.getRegisterList())
    {
        const auto &varying = *varyingReg.packedVarying;

        std::string locationString = "location = " + Str(varyingReg.registerRow);
        if (varyingReg.registerColumn > 0)
        {
            ASSERT(!varying.varying->isStruct());
            ASSERT(!gl::IsMatrixType(varying.varying->type));
            locationString += ", component = " + Str(varyingReg.registerColumn);
        }

        InsertLayoutSpecifierString(&vertexSource, varying.varying->name, locationString);
        InsertLayoutSpecifierString(&fragmentSource, varying.varying->name, locationString);

        ASSERT(varying.interpolation == sh::INTERPOLATION_SMOOTH);
        InsertQualifierSpecifierString(&vertexSource, varying.varying->name, "out");
        InsertQualifierSpecifierString(&fragmentSource, varying.varying->name, "in");
    }

    // Remove all the markers for unused varyings.
    for (const std::string &varyingName : resources.varyingPacking.getInactiveVaryingNames())
    {
        EraseLayoutAndQualifierStrings(&vertexSource, &fragmentSource, varyingName);
    }

    // Bind the default uniforms for vertex and fragment shaders.
    // See corresponding code in OutputVulkanGLSL.cpp.
    std::string uniformsSearchString("@@ DEFAULT-UNIFORMS-SET-BINDING @@");

    std::string vertexDefaultUniformsBinding   = "set = 0, binding = 0";
    std::string fragmentDefaultUniformsBinding = "set = 0, binding = 1";

    angle::ReplaceSubstring(&vertexSource, uniformsSearchString, vertexDefaultUniformsBinding);
    angle::ReplaceSubstring(&fragmentSource, uniformsSearchString, fragmentDefaultUniformsBinding);

    // Assign textures to a descriptor set and binding.
    int textureCount     = 0;
    const auto &uniforms = programState.getUniforms();
    for (unsigned int uniformIndex : programState.getSamplerUniformRange())
    {
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];
        std::string setBindingString            = "set = 1, binding = " + Str(textureCount);

        // Samplers in structs are extracted and renamed.
        const std::string samplerName = GetMappedSamplerName(samplerUniform.name);

        ASSERT(samplerUniform.isActive(gl::ShaderType::Vertex) ||
               samplerUniform.isActive(gl::ShaderType::Fragment));
        if (samplerUniform.isActive(gl::ShaderType::Vertex))
        {
            InsertLayoutSpecifierString(&vertexSource, samplerName, setBindingString);
        }
        InsertQualifierSpecifierString(&vertexSource, samplerName, kUniformQualifier);

        if (samplerUniform.isActive(gl::ShaderType::Fragment))
        {
            InsertLayoutSpecifierString(&fragmentSource, samplerName, setBindingString);
        }
        InsertQualifierSpecifierString(&fragmentSource, samplerName, kUniformQualifier);

        textureCount++;
    }

    // Start the unused sampler bindings at something ridiculously high.
    constexpr int kBaseUnusedSamplerBinding = 100;
    int unusedSamplerBinding                = kBaseUnusedSamplerBinding;

    for (const gl::UnusedUniform &unusedUniform : resources.unusedUniforms)
    {
        if (unusedUniform.isSampler)
        {
            // Samplers in structs are extracted and renamed.
            std::string uniformName = GetMappedSamplerName(unusedUniform.name);

            std::stringstream layoutStringStream;

            layoutStringStream << "set = 0, binding = " << unusedSamplerBinding++;

            std::string layoutString = layoutStringStream.str();

            InsertLayoutSpecifierString(&vertexSource, uniformName, layoutString);
            InsertLayoutSpecifierString(&fragmentSource, uniformName, layoutString);

            InsertQualifierSpecifierString(&vertexSource, uniformName, kUniformQualifier);
            InsertQualifierSpecifierString(&fragmentSource, uniformName, kUniformQualifier);
        }
        else
        {
            EraseLayoutAndQualifierStrings(&vertexSource, &fragmentSource, unusedUniform.name);
        }
    }

    const char *defines = "";
    if (features.basicGLLineRasterization && IsLineMode(drawMode))
    {
        defines = "#define ANGLE_ENABLE_LINE_SEGMENT_RASTERIZATION\n";
    }

    std::array<const char *, 4> strings = {
        {defines, vertexSource.c_str(), defines, fragmentSource.c_str()}};
    std::array<int, 4> lengths = {{}};
    for (size_t stringIndex = 0; stringIndex < strings.size(); ++stringIndex)
    {
        lengths[stringIndex] = static_cast<int>(strlen(strings[stringIndex]));
    }

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

    glslang::TShader vertexShader(EShLangVertex);
    vertexShader.setStringsWithLengths(&strings[0], &lengths[0], 2);
    vertexShader.setEntryPoint("main");

    TBuiltInResource builtInResources(glslang::DefaultTBuiltInResource);
    GetBuiltInResourcesFromCaps(glCaps, &builtInResources);

    bool vertexResult =
        vertexShader.parse(&builtInResources, 450, ECoreProfile, false, false, messages);
    if (!vertexResult)
    {
        return gl::InternalError() << "Internal error parsing Vulkan vertex shader:\n"
                                   << vertexShader.getInfoLog() << "\n"
                                   << vertexShader.getInfoDebugLog() << "\n";
    }

    glslang::TShader fragmentShader(EShLangFragment);
    fragmentShader.setStringsWithLengths(&strings[2], &lengths[2], 2);
    fragmentShader.setEntryPoint("main");
    bool fragmentResult =
        fragmentShader.parse(&builtInResources, 450, ECoreProfile, false, false, messages);
    if (!fragmentResult)
    {
        return gl::InternalError() << "Internal error parsing Vulkan fragment shader:\n"
                                   << fragmentShader.getInfoLog() << "\n"
                                   << fragmentShader.getInfoDebugLog() << "\n";
    }

    glslang::TProgram program;
    program.addShader(&vertexShader);
    program.addShader(&fragmentShader);
    bool linkResult = program.link(messages);
    if (!linkResult)
    {
        return gl::InternalError() << "Internal error linking Vulkan shaders:\n"
                                   << program.getInfoLog() << "\n";
    }

    glslang::TIntermediate *vertexStage   = program.getIntermediate(EShLangVertex);
    glslang::TIntermediate *fragmentStage = program.getIntermediate(EShLangFragment);
    glslang::GlslangToSpv(*vertexStage, *vertexCodeOut);
    glslang::GlslangToSpv(*fragmentStage, *fragmentCodeOut);

    return true;
}

}  // namespace rx
