//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangUtils: Wrapper for Khronos's glslang compiler.
//

#include "libANGLE/renderer/metal/mtl_glslang_utils.h"

#include <regex>

#include <spirv_msl.hpp>

#include "common/apple_platform_utils.h"
#include "compiler/translator/TranslatorMetal.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

namespace rx
{
namespace mtl
{
namespace
{
constexpr uint32_t kGlslangTextureDescSet              = 0;
constexpr uint32_t kGlslangDefaultUniformAndXfbDescSet = 1;
constexpr uint32_t kGlslangDriverUniformsDescSet       = 2;
constexpr uint32_t kGlslangShaderResourceDescSet       = 3;

constexpr char kShadowSamplerCompareModesVarName[] = "ANGLEShadowCompareModes";

// Original mapping of front end from sampler name to multiple sampler slots (in form of
// slot:count pair)
using OriginalSamplerBindingMap =
    angle::HashMap<std::string, std::vector<std::pair<uint32_t, uint32_t>>>;

angle::Result HandleError(ErrorHandler *context, GlslangError)
{
    ANGLE_MTL_TRY(context, false);
    return angle::Result::Stop;
}

void ResetGlslangProgramInterfaceInfo(GlslangProgramInterfaceInfo *programInterfaceInfo)
{
    // These are binding options passed to glslang. The actual binding might be changed later
    // by spirv-cross.
    programInterfaceInfo->uniformsAndXfbDescriptorSetIndex  = kGlslangDefaultUniformAndXfbDescSet;
    programInterfaceInfo->currentUniformBindingIndex        = 0;
    programInterfaceInfo->textureDescriptorSetIndex         = kGlslangTextureDescSet;
    programInterfaceInfo->currentTextureBindingIndex        = 0;
    programInterfaceInfo->driverUniformsDescriptorSetIndex  = kGlslangDriverUniformsDescSet;
    programInterfaceInfo->shaderResourceDescriptorSetIndex  = kGlslangShaderResourceDescSet;
    programInterfaceInfo->currentShaderResourceBindingIndex = 0;
    programInterfaceInfo->locationsUsedForXfbExtension      = 0;

    static_assert(kDefaultUniformsBindingIndex != 0, "kDefaultUniformsBindingIndex must not be 0");
    static_assert(kDriverUniformsBindingIndex != 0, "kDriverUniformsBindingIndex must not be 0");
}


GlslangSourceOptions CreateSourceOptions()
{
    GlslangSourceOptions options;
    options.emulateTransformFeedback = true;
    // We can early out in the MSL once transform feedback is written
    options.transformFeedbackEarlyReturn = true;
    return options;
}

spv::ExecutionModel ShaderTypeToSpvExecutionModel(gl::ShaderType shaderType)
{
    switch (shaderType)
    {
        case gl::ShaderType::Vertex:
            return spv::ExecutionModelVertex;
        case gl::ShaderType::Fragment:
            return spv::ExecutionModelFragment;
        default:
            UNREACHABLE();
            return spv::ExecutionModelMax;
    }
}

void BindBuffers(spirv_cross::CompilerMSL *compiler,
                 const spirv_cross::SmallVector<spirv_cross::Resource> &resources,
                 gl::ShaderType shaderType,
                 const angle::HashMap<std::string, uint32_t> &uboOriginalBindings,
                 const angle::HashMap<uint32_t, uint32_t> &xfbOriginalBindings,
                 std::array<uint32_t, kMaxGLUBOBindings> *uboBindingsRemapOut,
                 std::array<uint32_t, kMaxShaderXFBs> *xfbBindingRemapOut,
                 bool *uboArgumentBufferUsed)
{
    auto &compilerMsl = *compiler;

    uint32_t totalUniformBufferSlots = 0;
    uint32_t totalXfbSlots           = 0;
    struct UniformBufferVar
    {
        const char *name = nullptr;
        spirv_cross::MSLResourceBinding resBinding;
        uint32_t arraySize;
    };
    std::vector<UniformBufferVar> uniformBufferBindings;

    for (const spirv_cross::Resource &resource : resources)
    {
        spirv_cross::MSLResourceBinding resBinding;
        resBinding.stage = ShaderTypeToSpvExecutionModel(shaderType);

        if (compilerMsl.has_decoration(resource.id, spv::DecorationDescriptorSet))
        {
            resBinding.desc_set =
                compilerMsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        }

        if (!compilerMsl.has_decoration(resource.id, spv::DecorationBinding))
        {
            continue;
        }

        resBinding.binding = compilerMsl.get_decoration(resource.id, spv::DecorationBinding);

        uint32_t bindingPoint = 0;
        // NOTE(hqle): We use separate discrete binding point for now, in future, we should use
        // one argument buffer for each descriptor set.
        switch (resBinding.desc_set)
        {
            case kGlslangTextureDescSet:
                // Texture binding point is ignored. We let spirv-cross automatically assign it and
                // retrieve it later
                continue;
            case kGlslangDriverUniformsDescSet:
                bindingPoint = mtl::kDriverUniformsBindingIndex;
                break;
            case kGlslangDefaultUniformAndXfbDescSet:
                if (shaderType != gl::ShaderType::Vertex ||
                    !xfbOriginalBindings.count(resBinding.binding))
                {
                    bindingPoint = mtl::kDefaultUniformsBindingIndex;
                }
                else
                {
                    // XFB buffer
                    uint32_t xfbSlot = xfbOriginalBindings.at(resBinding.binding);

                    totalXfbSlots++;
                    // XFB buffer is allocated slot starting from last discrete Metal buffer slot.
                    bindingPoint = kMaxShaderBuffers - 1 - xfbSlot;

                    xfbBindingRemapOut->at(xfbSlot) = bindingPoint;
                }
                break;
            case kGlslangShaderResourceDescSet:
            {
                UniformBufferVar uboVar;
                uboVar.name                       = resource.name.c_str();
                uboVar.resBinding                 = resBinding;
                const spirv_cross::SPIRType &type = compilerMsl.get_type_from_variable(resource.id);
                if (!type.array.empty())
                {
                    uboVar.arraySize = type.array[0];
                }
                else
                {
                    uboVar.arraySize = 1;
                }
                totalUniformBufferSlots += uboVar.arraySize;
                uniformBufferBindings.push_back(uboVar);
            }
                continue;
            default:
                // We don't support this descriptor set.
                continue;
        }

        resBinding.msl_buffer = bindingPoint;

        compilerMsl.add_msl_resource_binding(resBinding);
    }  // for (resources)

    if (totalUniformBufferSlots == 0)
    {
        return;
    }

    // Remap the uniform buffers bindings. glslang allows uniform buffers array to use exactly
    // one slot in the descriptor set. However, metal enforces that the uniform buffers array
    // use (n) slots where n=array size.
    uint32_t currentSlot = 0;
    uint32_t maxUBODiscreteSlots =
        kMaxShaderBuffers - totalXfbSlots - kUBOArgumentBufferBindingIndex;

    if (totalUniformBufferSlots > maxUBODiscreteSlots)
    {
        // If shader uses more than maxUBODiscreteSlots number of UBOs, encode them all into
        // an argument buffer. Each buffer will be assigned [[id(n)]] attribute.
        *uboArgumentBufferUsed = true;
    }
    else
    {
        // Use discrete buffer binding slot for UBOs which translates each slot to [[buffer(n)]]
        *uboArgumentBufferUsed = false;
        // Discrete buffer binding slot starts at kUBOArgumentBufferBindingIndex
        currentSlot += kUBOArgumentBufferBindingIndex;
    }

    for (UniformBufferVar &uboVar : uniformBufferBindings)
    {
        spirv_cross::MSLResourceBinding &resBinding = uboVar.resBinding;
        resBinding.msl_buffer                       = currentSlot;

        uint32_t originalBinding = uboOriginalBindings.at(uboVar.name);

        for (uint32_t i = 0; i < uboVar.arraySize; ++i, ++currentSlot)
        {
            // Use consecutive slot for member in array
            uboBindingsRemapOut->at(originalBinding + i) = currentSlot;
        }

        compilerMsl.add_msl_resource_binding(resBinding);
    }
}

void GetAssignedSamplerBindings(const spirv_cross::CompilerMSL &compilerMsl,
                                const OriginalSamplerBindingMap &originalBindings,
                                std::array<SamplerBinding, mtl::kMaxGLSamplerBindings> *bindings)
{
    for (const spirv_cross::Resource &resource : compilerMsl.get_shader_resources().sampled_images)
    {
        uint32_t descriptorSet = 0;
        if (compilerMsl.has_decoration(resource.id, spv::DecorationDescriptorSet))
        {
            descriptorSet = compilerMsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        }

        // We already assigned descriptor set 0 to textures. Just to double check.
        ASSERT(descriptorSet == kGlslangTextureDescSet);
        ASSERT(compilerMsl.has_decoration(resource.id, spv::DecorationBinding));

        uint32_t actualTextureSlot = compilerMsl.get_automatic_msl_resource_binding(resource.id);
        uint32_t actualSamplerSlot =
            compilerMsl.get_automatic_msl_resource_binding_secondary(resource.id);

        // Assign sequential index for subsequent array elements
        const std::vector<std::pair<uint32_t, uint32_t>> &resOrignalBindings =
            originalBindings.at(resource.name);
        uint32_t currentTextureSlot = actualTextureSlot;
        uint32_t currentSamplerSlot = actualSamplerSlot;
        for (const std::pair<uint32_t, uint32_t> &originalBindingRange : resOrignalBindings)
        {
            SamplerBinding &actualBinding = bindings->at(originalBindingRange.first);
            actualBinding.textureBinding  = currentTextureSlot;
            actualBinding.samplerBinding  = currentSamplerSlot;

            currentTextureSlot += originalBindingRange.second;
            currentSamplerSlot += originalBindingRange.second;
        }
    }
}

std::string PostProcessTranslatedMsl(bool hasDepthSampler, const std::string &translatedSource)
{
    std::string source;
    if (hasDepthSampler)
    {
        // Add ANGLEShadowCompareModes variable to main(), We need to add here because it is the
        // only way without modifying spirv-cross.
        std::regex mainDeclareRegex(
            R"(((vertex|fragment|kernel)\s+[_a-zA-Z0-9<>]+\s+main[^\(]*\())");
        std::string mainDeclareReplaceStr = std::string("$1constant uniform<uint> *") +
                                            kShadowSamplerCompareModesVarName + "[[buffer(" +
                                            Str(kShadowSamplerCompareModesBindingIndex) + ")]], ";
        source = std::regex_replace(translatedSource, mainDeclareRegex, mainDeclareReplaceStr);
    }
    else
    {
        source = translatedSource;
    }

    // Add function_constant attribute to gl_SampleMask.
    // Even though this varying is only used when ANGLECoverageMaskEnabled is true,
    // the spirv-cross doesn't assign function_constant attribute to it. Thus it won't be dead-code
    // removed when ANGLECoverageMaskEnabled=false.
    std::string sampleMaskReplaceStr = std::string("[[sample_mask, function_constant(") +
                                       sh::mtl::kCoverageMaskEnabledConstName +
                                       ")]]";

    // This replaces "gl_SampleMask [[sample_mask]]"
    //          with "gl_SampleMask [[sample_mask, function_constant(ANGLECoverageMaskEnabled)]]"
    std::regex sampleMaskDeclareRegex(R"(\[\s*\[\s*sample_mask\s*\]\s*\])");
    return std::regex_replace(source, sampleMaskDeclareRegex, sampleMaskReplaceStr);
}

// Customized spirv-cross compiler
class SpirvToMslCompiler : public spirv_cross::CompilerMSL
{
  public:
    SpirvToMslCompiler(Context *context, std::vector<uint32_t> &&spriv)
        : spirv_cross::CompilerMSL(spriv), mContext(context)
    {}

    void compileEx(gl::ShaderType shaderType,
                   const angle::HashMap<std::string, uint32_t> &uboOriginalBindings,
                   const angle::HashMap<uint32_t, uint32_t> &xfbOriginalBindings,
                   const OriginalSamplerBindingMap &originalSamplerBindings,
                   TranslatedShaderInfo *mslShaderInfoOut)
    {
        spirv_cross::CompilerMSL::Options compOpt;

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        compOpt.platform = spirv_cross::CompilerMSL::Options::macOS;
#else
        compOpt.platform = spirv_cross::CompilerMSL::Options::iOS;
#endif

        if (ANGLE_APPLE_AVAILABLE_XCI(10.14, 13.0, 12))
        {
            // Use Metal 2.1
            compOpt.set_msl_version(2, 1);
        }
        else
        {
            // Always use at least Metal 2.0.
            compOpt.set_msl_version(2);
        }

        compOpt.pad_fragment_output_components = true;

        // Tell spirv-cross to map default & driver uniform & storage blocks as we want
        spirv_cross::ShaderResources mslRes = spirv_cross::CompilerMSL::get_shader_resources();

        spirv_cross::SmallVector<spirv_cross::Resource> buffers = std::move(mslRes.uniform_buffers);
        buffers.insert(buffers.end(), mslRes.storage_buffers.begin(), mslRes.storage_buffers.end());

        BindBuffers(this, buffers, shaderType, uboOriginalBindings, xfbOriginalBindings,
                    &mslShaderInfoOut->actualUBOBindings, &mslShaderInfoOut->actualXFBBindings,
                    &mslShaderInfoOut->hasUBOArgumentBuffer);

        if (mslShaderInfoOut->hasUBOArgumentBuffer)
        {
            // Enable argument buffer.
            compOpt.argument_buffers = true;

            // Force UBO argument buffer binding to start at kUBOArgumentBufferBindingIndex.
            spirv_cross::MSLResourceBinding argBufferBinding = {};
            argBufferBinding.stage    = ShaderTypeToSpvExecutionModel(shaderType);
            argBufferBinding.desc_set = kGlslangShaderResourceDescSet;
            argBufferBinding.binding =
                spirv_cross::kArgumentBufferBinding;  // spirv-cross built-in binding.
            argBufferBinding.msl_buffer = kUBOArgumentBufferBindingIndex;  // Actual binding.
            spirv_cross::CompilerMSL::add_msl_resource_binding(argBufferBinding);

            // Force discrete slot bindings for textures, default uniforms & driver uniforms
            // instead of using argument buffer.
            spirv_cross::CompilerMSL::add_discrete_descriptor_set(kGlslangTextureDescSet);
            spirv_cross::CompilerMSL::add_discrete_descriptor_set(kGlslangDefaultUniformAndXfbDescSet);
            spirv_cross::CompilerMSL::add_discrete_descriptor_set(kGlslangDriverUniformsDescSet);
        }
        else
        {
            // Disable argument buffer generation for uniform buffers
            compOpt.argument_buffers = false;
        }

        spirv_cross::CompilerMSL::set_msl_options(compOpt);

        // Actual compilation
        mslShaderInfoOut->metalShaderSource =
            PostProcessTranslatedMsl(mHasDepthSampler, spirv_cross::CompilerMSL::compile());

        // Retrieve automatic texture slot assignments
        GetAssignedSamplerBindings(*this, originalSamplerBindings,
                                   &mslShaderInfoOut->actualSamplerBindings);
    }

  private:
    // Override CompilerMSL
    void emit_header() override
    {
        spirv_cross::CompilerMSL::emit_header();
        if (!mHasDepthSampler)
        {
            return;
        }
        // Work around code for these issues:
        // - spriv_cross always translates shadow texture's sampling to sample_compare() and doesn't
        // take into account GL_TEXTURE_COMPARE_MODE=GL_NONE.
        // - on macOS, explicit level of detail parameter is not supported in sample_compare().
        // - on devices prior to iOS GPU family 3, changing sampler's compare mode outside shader is
        // not supported.
        if (!mContext->getDisplay()->getFeatures().allowRuntimeSamplerCompareMode.enabled)
        {
            statement("#define ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE");
        }

        statement("enum class ANGLECompareMode : uint");
        statement("{");
        statement("    None = 0,");
        statement("    Less,");
        statement("    LessEqual,");
        statement("    Greater,");
        statement("    GreaterEqual,");
        statement("    Never,");
        statement("    Always,");
        statement("    Equal,");
        statement("    NotEqual,");
        statement("};");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEcompare(T depth, T dref, UniformOrUInt compareMode)");
        statement("{");
        statement("   ANGLECompareMode mode = static_cast<ANGLECompareMode>(compareMode);");
        statement("   switch (mode)");
        statement("   {");
        statement("        case ANGLECompareMode::Less:");
        statement("            return dref < depth;");
        statement("        case ANGLECompareMode::LessEqual:");
        statement("            return dref <= depth;");
        statement("        case ANGLECompareMode::Greater:");
        statement("            return dref > depth;");
        statement("        case ANGLECompareMode::GreaterEqual:");
        statement("            return dref >= depth;");
        statement("        case ANGLECompareMode::Never:");
        statement("            return 0;");
        statement("        case ANGLECompareMode::Always:");
        statement("            return 1;");
        statement("        case ANGLECompareMode::Equal:");
        statement("            return dref == depth;");
        statement("        case ANGLECompareMode::NotEqual:");
        statement("            return dref != depth;");
        statement("        default:");
        statement("            return 1;");
        statement("   }");
        statement("}");
        statement("");

        statement("// Wrapper functions for shadow texture functions");
        // 2D PCF sampling
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexturePCF(depth2d<T> texture, sampler s, float2 coord, float "
                  "compare_value, Opt options, int2 offset, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("#if defined(__METAL_MACOS__)");
        statement("    float2 dims = float2(texture.get_width(), texture.get_height());");
        statement("    float2 imgCoord = coord * dims;");
        statement("    float2 texelSize = 1.0 / dims;");
        statement("    float2 weight = fract(imgCoord);");
        statement("    float tl = ANGLEcompare(texture.sample(s, coord, options, offset), "
                  "compare_value, shadowCompareMode);");
        statement("    float tr = ANGLEcompare(texture.sample(s, coord + float2(texelSize.x, 0.0), "
                  "options, offset), compare_value, shadowCompareMode);");
        statement("    float bl = ANGLEcompare(texture.sample(s, coord + float2(0.0, texelSize.y), "
                  "options, offset), compare_value, shadowCompareMode);");
        statement("    float br = ANGLEcompare(texture.sample(s, coord + texelSize, options, "
                  "offset), compare_value, shadowCompareMode);");
        statement("    float top = mix(tl, tr, weight.x);");
        statement("    float bottom = mix(bl, br, weight.x);");
        statement("    return mix(top, bottom, weight.y);");
        statement("#else  // if defined(__METAL_MACOS__)");
        statement("    return ANGLEcompare(texture.sample(s, coord, options, offset), "
                  "compare_value, shadowCompareMode);");
        statement("#endif  // if defined(__METAL_MACOS__)");
        statement("}");
        statement("");

        // Cube PCF sampling
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexturePCF(depthcube<T> texture, sampler s, float3 coord, float "
                  "compare_value, Opt options, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    // NOTE(hqle): to implement");
        statement("    return ANGLEcompare(texture.sample(s, coord, options), compare_value, "
                  "shadowCompareMode);");
        statement("}");
        statement("");

        // 2D array PCF sampling
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement(
            "inline T ANGLEtexturePCF(depth2d_array<T> texture, sampler s, float2 coord, uint "
            "array, float compare_value, Opt options, int2 offset, UniformOrUInt "
            "shadowCompareMode)");
        statement("{");
        statement("#if defined(__METAL_MACOS__)");
        statement("    float2 dims = float2(texture.get_width(), texture.get_height());");
        statement("    float2 imgCoord = coord * dims;");
        statement("    float2 texelSize = 1.0 / dims;");
        statement("    float2 weight = fract(imgCoord);");
        statement("    float tl = ANGLEcompare(texture.sample(s, coord, array, options, offset), "
                  "compare_value, shadowCompareMode);");
        statement("    float tr = ANGLEcompare(texture.sample(s, coord + float2(texelSize.x, 0.0), "
                  "array, options, offset), compare_value, shadowCompareMode);");
        statement("    float bl = ANGLEcompare(texture.sample(s, coord + float2(0.0, texelSize.y), "
                  "array, options, offset), compare_value, shadowCompareMode);");
        statement("    float br = ANGLEcompare(texture.sample(s, coord + texelSize, array, "
                  "options, offset), compare_value, shadowCompareMode);");
        statement("    float top = mix(tl, tr, weight.x);");
        statement("    float bottom = mix(bl, br, weight.x);");
        statement("    return mix(top, bottom, weight.y);");
        statement("#else  // if defined(__METAL_MACOS__)");
        statement("    return ANGLEcompare(texture.sample(s, coord, array, options, offset), "
                  "compare_value, shadowCompareMode);");
        statement("#endif  // if defined(__METAL_MACOS__)");
        statement("}");
        statement("");

        // 2D texture's sample_compare() wrapper
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d<T> texture, sampler s, float2 coord, float "
                  "compare_value, Opt options, int2 offset, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement(
            "#if defined(__METAL_MACOS__) || defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, compare_value, options, offset, "
                  "shadowCompareMode);");
        statement("#else");
        statement("    return texture.sample_compare(s, coord, compare_value, options, offset);");
        statement("#endif");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d<T> texture, sampler s, float2 coord, float "
                  "compare_value, Opt options, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtextureCompare(texture, s, coord, compare_value, options, "
                  "int2(0), shadowCompareMode);");
        statement("}");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d<T> texture, sampler s, float2 coord, float "
                  "compare_value, int2 offset, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("#if defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, compare_value, level(0), offset, "
                  "shadowCompareMode);");
        statement("#else");
        statement("    return texture.sample_compare(s, coord, compare_value, offset);");
        statement("#endif");
        statement("}");
        statement("");

        // Cube texture's sample_compare() wrapper
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement(
            "inline T ANGLEtextureCompare(depthcube<T> texture, sampler s, float3 coord, float "
            "compare_value, Opt options, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement(
            "#if defined(__METAL_MACOS__) || defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, compare_value, options, "
                  "shadowCompareMode);");
        statement("#else");
        statement("    return texture.sample_compare(s, coord, compare_value, options);");
        statement("#endif");
        statement("}");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement(
            "inline T ANGLEtextureCompare(depthcube<T> texture, sampler s, float3 coord, float "
            "compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("#if defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, compare_value, level(0), "
                  "shadowCompareMode);");
        statement("#else");
        statement("    return texture.sample_compare(s, coord, compare_value);");
        statement("#endif");
        statement("}");
        statement("");

        // 2D array texture's sample_compare() wrapper
        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d_array<T> texture, sampler s, float2 coord, "
                  "uint array, float compare_value, Opt options, int2 offset, UniformOrUInt "
                  "shadowCompareMode)");
        statement("{");
        statement(
            "#if defined(__METAL_MACOS__) || defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, array, compare_value, options, "
                  "offset, shadowCompareMode);");
        statement("#else");
        statement(
            "    return texture.sample_compare(s, coord, array, compare_value, options, offset);");
        statement("#endif");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d_array<T> texture, sampler s, float2 coord, "
                  "uint array, float compare_value, Opt options, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtextureCompare(texture, s, coord, array, compare_value, "
                  "options, int2(0), shadowCompareMode);");
        statement("}");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtextureCompare(depth2d_array<T> texture, sampler s, float2 coord, "
                  "uint array, float compare_value, int2 offset, UniformOrUInt "
                  "shadowCompareMode)");
        statement("{");
        statement("#if defined(ANGLE_MTL_NO_SAMPLER_RUNTIME_COMPARE_MODE)");
        statement("    return ANGLEtexturePCF(texture, s, coord, array, compare_value, level(0), "
                  "offset, shadowCompareMode);");
        statement("#else");
        statement("    return texture.sample_compare(s, coord, array, compare_value, offset);");
        statement("#endif");
        statement("}");
        statement("");

        // 2D texture's generic sampling function
        statement("// Wrapper functions for shadow texture functions");
        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d<T> texture, sampler s, float2 coord, int2 offset, "
                  "float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, compare_value, offset, "
                  "shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord, offset);");
        statement("    }");
        statement("}");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d<T> texture, sampler s, float2 coord, float "
                  "compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtexture(texture, s, coord, int2(0), compare_value, "
                  "shadowCompareMode);");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d<T> texture, sampler s, float2 coord, Opt options, "
                  "int2 offset, float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, compare_value, options, "
                  "offset, shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord, options, offset);");
        statement("    }");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d<T> texture, sampler s, float2 coord, Opt options, "
                  "float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtexture(texture, s, coord, options, int2(0), compare_value, "
                  "shadowCompareMode);");
        statement("}");
        statement("");

        // Cube texture's generic sampling function
        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depthcube<T> texture, sampler s, float3 coord, float "
                  "compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, compare_value, "
                  "shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord);");
        statement("    }");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depthcube<T> texture, sampler s, float2 coord, Opt "
                  "options, float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, compare_value, options, "
                  "shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord, options);");
        statement("    }");
        statement("}");
        statement("");

        // 2D array texture's generic sampling function
        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d_array<T> texture, sampler s, float2 coord, uint "
                  "array, int2 offset, "
                  "float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, array, compare_value, "
                  "offset, shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord, array, offset);");
        statement("    }");
        statement("}");
        statement("");

        statement("template <typename T, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d_array<T> texture, sampler s, float2 coord, uint "
                  "array, float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtexture(texture, s, coord, array, int2(0), compare_value, "
                  "shadowCompareMode);");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d_array<T> texture, sampler s, float2 coord, uint "
                  "array, Opt options, int2 offset, "
                  "float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    if (shadowCompareMode)");
        statement("    {");
        statement("        return ANGLEtextureCompare(texture, s, coord, array, compare_value, "
                  "options, offset, shadowCompareMode);");
        statement("    }");
        statement("    else");
        statement("    {");
        statement("        return texture.sample(s, coord, array, options, offset);");
        statement("    }");
        statement("}");
        statement("");

        statement("template <typename T, typename Opt, typename UniformOrUInt>");
        statement("inline T ANGLEtexture(depth2d_array<T> texture, sampler s, float2 coord, uint "
                  "array, Opt options, float compare_value, UniformOrUInt shadowCompareMode)");
        statement("{");
        statement("    return ANGLEtexture(texture, s, coord, array, options, int2(0), "
                  "compare_value, shadowCompareMode);");
        statement("}");
        statement("");
    }

    Context *mContext;
    bool mHasDepthSampler                  = false;
};

angle::Result ConvertSpirvToMsl(Context *context,
                                gl::ShaderType shaderType,
                                const angle::HashMap<std::string, uint32_t> &uboOriginalBindings,
                                const angle::HashMap<uint32_t, uint32_t> &xfbOriginalBindings,
                                const OriginalSamplerBindingMap &originalSamplerBindings,
                                std::vector<uint32_t> *sprivCode,
                                TranslatedShaderInfo *translatedShaderInfoOut)
{
    if (!sprivCode || sprivCode->empty())
    {
        return angle::Result::Continue;
    }

    SpirvToMslCompiler compilerMsl(context, std::move(*sprivCode));

    // NOTE(hqle): spirv-cross uses exceptions to report error, what should we do here
    // in case of error?
    compilerMsl.compileEx(shaderType, uboOriginalBindings, xfbOriginalBindings,
                          originalSamplerBindings, translatedShaderInfoOut);
    if (translatedShaderInfoOut->metalShaderSource.size() == 0)
    {
        ANGLE_MTL_CHECK(context, false, GL_INVALID_OPERATION);
    }

    return angle::Result::Continue;
}

}  // namespace

void GlslangGetShaderSource(const gl::ProgramState &programState,
                            const gl::ProgramLinkedResources &resources,
                            gl::ShaderMap<std::string> *shaderSourcesOut,
                            std::string *xfbOnlyShaderSourceOut,
                            ShaderMapInterfaceVariableInfoMap *variableInfoMapOut,
                            ShaderInterfaceVariableInfoMap *xfbOnlyVSVariableInfoMapOut)
{
    GlslangSourceOptions options = CreateSourceOptions();
    GlslangProgramInterfaceInfo programInterfaceInfo;
    ResetGlslangProgramInterfaceInfo(&programInterfaceInfo);

    rx::GlslangGetShaderSource(options, programState, resources, &programInterfaceInfo,
                               shaderSourcesOut, variableInfoMapOut);

    // Special version for XFB only
    if (xfbOnlyShaderSourceOut && !programState.getLinkedTransformFeedbackVaryings().empty())
    {
        gl::Shader *glShader    = programState.getAttachedShader(gl::ShaderType::Vertex);
        *xfbOnlyShaderSourceOut = glShader ? glShader->getTranslatedSource() : "";

        GlslangProgramInterfaceInfo xfbOnlyInterfaceInfo;
        ResetGlslangProgramInterfaceInfo(&xfbOnlyInterfaceInfo);
        ShaderMapInterfaceVariableInfoMap xfbOnlyVariableMaps;

        options.emulateTransformFeedback = true;

        rx::GlslangGenTransformFeedbackEmulationOutputs(
            options, programState, &xfbOnlyInterfaceInfo, xfbOnlyShaderSourceOut,
            &xfbOnlyVariableMaps[gl::ShaderType::Vertex]);

        GlslangAssignLocations(options, programState.getExecutable(), gl::ShaderType::Vertex,
                               gl::ShaderType::Vertex, &xfbOnlyInterfaceInfo, &xfbOnlyVariableMaps);
        *xfbOnlyVSVariableInfoMapOut = std::move(xfbOnlyVariableMaps[gl::ShaderType::Vertex]);
    }
}

angle::Result GlslangGetShaderSpirvCode(ErrorHandler *context,
                                        const gl::ShaderBitSet &linkedShaderStages,
                                        const gl::Caps &glCaps,
                                        const gl::ShaderMap<std::string> &shaderSources,
                                        const ShaderMapInterfaceVariableInfoMap &variableInfoMap,
                                        gl::ShaderMap<std::vector<uint32_t>> *shaderCodeOut)
{
    gl::ShaderMap<SpirvBlob> initialSpirvBlobs;

    ANGLE_TRY(rx::GlslangGetShaderSpirvCode(
        [context](GlslangError error) { return HandleError(context, error); }, linkedShaderStages,
        glCaps, shaderSources, &initialSpirvBlobs));

    for (const gl::ShaderType shaderType : linkedShaderStages)
    {
        // we pass in false here to skip modifications related to  early fragment tests
        // optimizations and line rasterization. These are done in the initProgram time since they
        // are related to context state. We must keep original untouched spriv blobs here because we
        // do not have ability to add back in at initProgram time.
        angle::Result status = GlslangTransformSpirvCode(
            [context](GlslangError error) { return HandleError(context, error); }, shaderType,
            false, false, variableInfoMap[shaderType], initialSpirvBlobs[shaderType],
            &(*shaderCodeOut)[shaderType]);
        if (status != angle::Result::Continue)
        {
            return status;
        }
    }

    return angle::Result::Continue;
}

angle::Result SpirvCodeToMsl(Context *context,
                             const gl::ProgramState &programState,
                             const ShaderInterfaceVariableInfoMap &xfbVSVariableInfoMap,
                             gl::ShaderMap<std::vector<uint32_t>> *spirvShaderCode,
                             std::vector<uint32_t> *xfbOnlySpirvCode /** nullable */,
                             gl::ShaderMap<TranslatedShaderInfo> *mslShaderInfoOut,
                             TranslatedShaderInfo *mslXfbOnlyShaderInfoOut /** nullable */)
{
    // Retrieve original uniform buffer bindings generated by front end. We will need to do a remap.
    angle::HashMap<std::string, uint32_t> uboOriginalBindings;
    const std::vector<gl::InterfaceBlock> &blocks = programState.getUniformBlocks();
    for (uint32_t bufferIdx = 0; bufferIdx < blocks.size(); ++bufferIdx)
    {
        const gl::InterfaceBlock &block = blocks[bufferIdx];
        if (!uboOriginalBindings.count(block.mappedName))
        {
            uboOriginalBindings[block.mappedName] = bufferIdx;
        }
    }
    // Retrieve original XFB buffers bindings produced by front end.
    angle::HashMap<uint32_t, uint32_t> xfbOriginalBindings;
    for (uint32_t bufferIdx = 0; bufferIdx < kMaxShaderXFBs; ++bufferIdx)
    {
        std::string bufferName = rx::GetXfbBufferName(bufferIdx);
        if (xfbVSVariableInfoMap.count(bufferName))
        {
            xfbOriginalBindings[xfbVSVariableInfoMap.at(bufferName).binding] = bufferIdx;
        }
    }

    // Retrieve original sampler bindings produced by front end.
    OriginalSamplerBindingMap originalSamplerBindings;
    const std::vector<gl::SamplerBinding> &samplerBindings = programState.getSamplerBindings();
    const std::vector<gl::LinkedUniform> &uniforms         = programState.getUniforms();

    for (uint32_t textureIndex = 0; textureIndex < samplerBindings.size(); ++textureIndex)
    {
        const gl::SamplerBinding &samplerBinding = samplerBindings[textureIndex];
        uint32_t uniformIndex = programState.getUniformIndexFromSamplerIndex(textureIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];
        std::string mappedSamplerName           = GlslangGetMappedSamplerName(samplerUniform.name);
        originalSamplerBindings[mappedSamplerName].push_back(
            {textureIndex, static_cast<uint32_t>(samplerBinding.boundTextureUnits.size())});
    }

    // Do the actual translation
    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        std::vector<uint32_t> &sprivCode = spirvShaderCode->at(shaderType);
        ANGLE_TRY(ConvertSpirvToMsl(context, shaderType, uboOriginalBindings, xfbOriginalBindings,
                                    originalSamplerBindings, &sprivCode,
                                    &mslShaderInfoOut->at(shaderType)));
    }  // for (gl::ShaderType shaderType

    // Special version of XFB only
    if (xfbOnlySpirvCode && !programState.getLinkedTransformFeedbackVaryings().empty())
    {
        ANGLE_TRY(ConvertSpirvToMsl(context, gl::ShaderType::Vertex, uboOriginalBindings,
                                    xfbOriginalBindings, originalSamplerBindings, xfbOnlySpirvCode,
                                    mslXfbOnlyShaderInfoOut));
    }

    return angle::Result::Continue;
}
}  // namespace mtl
}  // namespace rx
