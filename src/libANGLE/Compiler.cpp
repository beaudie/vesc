//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Compiler.cpp: implements the gl::Compiler class.

#include "libANGLE/Compiler.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/CompilerImpl.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace gl
{

namespace
{

// To know when to call sh::Initialize and sh::Finalize.
size_t gActiveCompilers = 0;

}  // anonymous namespace

Compiler::Compiler(rx::GLImplFactory *implFactory, const State &state, egl::Display *display)
    : mImplementation(implFactory->createCompiler()),
      mSpec(SelectShaderSpec(state)),
      mOutputType(mImplementation->getTranslatorOutputType()),
      mResources()
{
    // TODO(http://anglebug.com/3819): Update for GL version specific validation
    ASSERT(state.getClientMajorVersion() == 1 || state.getClientMajorVersion() == 2 ||
           state.getClientMajorVersion() == 3 || state.getClientMajorVersion() == 4);

    {
        std::lock_guard<std::mutex> lock(display->getDisplayGlobalMutex());
        if (gActiveCompilers == 0)
        {
            sh::Initialize();
        }
        ++gActiveCompilers;
    }

    mResources = CalculateBuiltInResources(state);
}

Compiler::~Compiler() = default;

void Compiler::onDestroy(const Context *context)
{
    std::lock_guard<std::mutex> lock(context->getDisplay()->getDisplayGlobalMutex());
    for (auto &pool : mPools)
    {
        for (ShCompilerInstance &instance : pool)
        {
            instance.destroy();
        }
    }
    --gActiveCompilers;
    if (gActiveCompilers == 0)
    {
        sh::Finalize();
    }
}

ShCompilerInstance Compiler::getInstance(ShaderType type)
{
    ASSERT(type != ShaderType::InvalidEnum);
    auto &pool = mPools[type];
    if (pool.empty())
    {
        ShHandle handle = sh::ConstructCompiler(ToGLenum(type), mSpec, mOutputType, &mResources);
        ASSERT(handle);
        return ShCompilerInstance(handle, mOutputType, type);
    }
    else
    {
        ShCompilerInstance instance = std::move(pool.back());
        pool.pop_back();
        return instance;
    }
}

void Compiler::putInstance(ShCompilerInstance &&instance)
{
    static constexpr size_t kMaxPoolSize = 32;
    auto &pool                           = mPools[instance.getShaderType()];
    if (pool.size() < kMaxPoolSize)
    {
        pool.push_back(std::move(instance));
    }
    else
    {
        instance.destroy();
    }
}

ShShaderSpec Compiler::SelectShaderSpec(const State &state)
{
    const EGLenum clientType = state.getClientType();
    const EGLint profileMask = state.getProfileMask();
    const GLint majorVersion = state.getClientMajorVersion();
    const GLint minorVersion = state.getClientMinorVersion();
    bool isWebGL             = state.isWebGL();

    // For Desktop GL
    if (clientType == EGL_OPENGL_API)
    {
        if ((profileMask & EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT) != 0)
        {
            return SH_GL_CORE_SPEC;
        }
        else
        {
            return SH_GL_COMPATIBILITY_SPEC;
        }
    }

    if (majorVersion >= 3)
    {
        switch (minorVersion)
        {
            case 2:
                ASSERT(!isWebGL);
                return SH_GLES3_2_SPEC;
            case 1:
                return isWebGL ? SH_WEBGL3_SPEC : SH_GLES3_1_SPEC;
            case 0:
                return isWebGL ? SH_WEBGL2_SPEC : SH_GLES3_SPEC;
            default:
                UNREACHABLE();
        }
    }

    // GLES1 emulation: Use GLES3 shader spec.
    if (!isWebGL && majorVersion == 1)
    {
        return SH_GLES3_SPEC;
    }

    return isWebGL ? SH_WEBGL_SPEC : SH_GLES2_SPEC;
}

ShBuiltInResources Compiler::CalculateBuiltInResources(const State &state)
{
    const Caps &caps             = state.getCaps();
    const Extensions &extensions = state.getExtensions();

    ShBuiltInResources resources{};
    sh::InitBuiltInResources(&resources);
    resources.MaxVertexAttribs             = caps.maxVertexAttributes;
    resources.MaxVertexUniformVectors      = caps.maxVertexUniformVectors;
    resources.MaxVaryingVectors            = caps.maxVaryingVectors;
    resources.MaxVertexTextureImageUnits   = caps.maxShaderTextureImageUnits[ShaderType::Vertex];
    resources.MaxCombinedTextureImageUnits = caps.maxCombinedTextureImageUnits;
    resources.MaxTextureImageUnits         = caps.maxShaderTextureImageUnits[ShaderType::Fragment];
    resources.MaxFragmentUniformVectors    = caps.maxFragmentUniformVectors;
    resources.MaxDrawBuffers               = caps.maxDrawBuffers;
    resources.OES_standard_derivatives     = extensions.standardDerivativesOES;
    resources.EXT_draw_buffers             = extensions.drawBuffersEXT;
    resources.EXT_shader_texture_lod       = extensions.shaderTextureLodEXT;
    resources.EXT_shader_non_constant_global_initializers =
        extensions.shaderNonConstantGlobalInitializersEXT;
    resources.OES_EGL_image_external                = extensions.EGLImageExternalOES;
    resources.OES_EGL_image_external_essl3          = extensions.EGLImageExternalEssl3OES;
    resources.NV_EGL_stream_consumer_external       = extensions.EGLStreamConsumerExternalNV;
    resources.NV_shader_noperspective_interpolation = extensions.shaderNoperspectiveInterpolationNV;
    resources.ARB_texture_rectangle                 = extensions.textureRectangleANGLE;
    resources.EXT_gpu_shader5                       = extensions.gpuShader5EXT;
    resources.OES_shader_io_blocks                  = extensions.shaderIoBlocksOES;
    resources.EXT_shader_io_blocks                  = extensions.shaderIoBlocksEXT;
    resources.OES_texture_storage_multisample_2d_array =
        extensions.textureStorageMultisample2dArrayOES;
    resources.OES_texture_3D = extensions.texture3DOES;
    resources.ANGLE_base_vertex_base_instance_shader_builtin =
        extensions.baseVertexBaseInstanceShaderBuiltinANGLE;
    resources.ANGLE_multi_draw                 = extensions.multiDrawANGLE;
    resources.ANGLE_shader_pixel_local_storage = extensions.shaderPixelLocalStorageANGLE;
    resources.ANGLE_texture_multisample        = extensions.textureMultisampleANGLE;
    resources.APPLE_clip_distance              = extensions.clipDistanceAPPLE;
    // OES_shader_multisample_interpolation
    resources.OES_shader_multisample_interpolation = extensions.shaderMultisampleInterpolationOES;
    resources.OES_shader_image_atomic              = extensions.shaderImageAtomicOES;
    // TODO: use shader precision caps to determine if high precision is supported?
    resources.FragmentPrecisionHigh = 1;
    resources.EXT_frag_depth        = extensions.fragDepthEXT;

    // OVR_multiview state
    resources.OVR_multiview = extensions.multiviewOVR;

    // OVR_multiview2 state
    resources.OVR_multiview2 = extensions.multiview2OVR;
    resources.MaxViewsOVR    = caps.maxViews;

    // EXT_multisampled_render_to_texture and EXT_multisampled_render_to_texture2
    resources.EXT_multisampled_render_to_texture  = extensions.multisampledRenderToTextureEXT;
    resources.EXT_multisampled_render_to_texture2 = extensions.multisampledRenderToTexture2EXT;

    // WEBGL_video_texture
    resources.WEBGL_video_texture = extensions.videoTextureWEBGL;

    // OES_texture_cube_map_array
    resources.OES_texture_cube_map_array = extensions.textureCubeMapArrayOES;
    resources.EXT_texture_cube_map_array = extensions.textureCubeMapArrayEXT;

    // EXT_shadow_samplers
    resources.EXT_shadow_samplers = extensions.shadowSamplersEXT;

    // OES_texture_buffer
    resources.OES_texture_buffer = extensions.textureBufferOES;
    resources.EXT_texture_buffer = extensions.textureBufferEXT;

    // GL_EXT_YUV_target
    resources.EXT_YUV_target = extensions.YUVTargetEXT;

    resources.EXT_shader_framebuffer_fetch_non_coherent =
        extensions.shaderFramebufferFetchNonCoherentEXT;

    resources.EXT_shader_framebuffer_fetch = extensions.shaderFramebufferFetchEXT;

    // GL_EXT_clip_cull_distance
    resources.EXT_clip_cull_distance = extensions.clipCullDistanceEXT;

    // GL_EXT_primitive_bounding_box
    resources.EXT_primitive_bounding_box = extensions.primitiveBoundingBoxEXT;

    // GL_OES_primitive_bounding_box
    resources.OES_primitive_bounding_box = extensions.primitiveBoundingBoxOES;

    // GLSL ES 3.0 constants
    resources.MaxVertexOutputVectors  = caps.maxVertexOutputComponents / 4;
    resources.MaxFragmentInputVectors = caps.maxFragmentInputComponents / 4;
    resources.MinProgramTexelOffset   = caps.minProgramTexelOffset;
    resources.MaxProgramTexelOffset   = caps.maxProgramTexelOffset;

    // EXT_blend_func_extended
    resources.EXT_blend_func_extended  = extensions.blendFuncExtendedEXT;
    resources.MaxDualSourceDrawBuffers = caps.maxDualSourceDrawBuffers;

    // APPLE_clip_distance/EXT_clip_cull_distance
    resources.MaxClipDistances                = caps.maxClipDistances;
    resources.MaxCullDistances                = caps.maxCullDistances;
    resources.MaxCombinedClipAndCullDistances = caps.maxCombinedClipAndCullDistances;

    // OES_sample_variables
    resources.OES_sample_variables = extensions.sampleVariablesOES;
    resources.MaxSamples           = caps.maxSamples;

    // ANDROID_extension_pack_es31a
    resources.ANDROID_extension_pack_es31a = extensions.extensionPackEs31aANDROID;

    // KHR_blend_equation_advanced
    resources.KHR_blend_equation_advanced = extensions.blendEquationAdvancedKHR;

    // GLSL ES 3.1 constants
    resources.MaxProgramTextureGatherOffset    = caps.maxProgramTextureGatherOffset;
    resources.MinProgramTextureGatherOffset    = caps.minProgramTextureGatherOffset;
    resources.MaxImageUnits                    = caps.maxImageUnits;
    resources.MaxVertexImageUniforms           = caps.maxShaderImageUniforms[ShaderType::Vertex];
    resources.MaxFragmentImageUniforms         = caps.maxShaderImageUniforms[ShaderType::Fragment];
    resources.MaxComputeImageUniforms          = caps.maxShaderImageUniforms[ShaderType::Compute];
    resources.MaxCombinedImageUniforms         = caps.maxCombinedImageUniforms;
    resources.MaxCombinedShaderOutputResources = caps.maxCombinedShaderOutputResources;
    resources.MaxUniformLocations              = caps.maxUniformLocations;

    for (size_t index = 0u; index < 3u; ++index)
    {
        resources.MaxComputeWorkGroupCount[index] = caps.maxComputeWorkGroupCount[index];
        resources.MaxComputeWorkGroupSize[index]  = caps.maxComputeWorkGroupSize[index];
    }

    resources.MaxComputeUniformComponents = caps.maxShaderUniformComponents[ShaderType::Compute];
    resources.MaxComputeTextureImageUnits = caps.maxShaderTextureImageUnits[ShaderType::Compute];

    resources.MaxComputeAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::Compute];
    resources.MaxComputeAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Compute];

    resources.MaxVertexAtomicCounters   = caps.maxShaderAtomicCounters[ShaderType::Vertex];
    resources.MaxFragmentAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::Fragment];
    resources.MaxCombinedAtomicCounters = caps.maxCombinedAtomicCounters;
    resources.MaxAtomicCounterBindings  = caps.maxAtomicCounterBufferBindings;
    resources.MaxVertexAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Vertex];
    resources.MaxFragmentAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Fragment];
    resources.MaxCombinedAtomicCounterBuffers = caps.maxCombinedAtomicCounterBuffers;
    resources.MaxAtomicCounterBufferSize      = caps.maxAtomicCounterBufferSize;

    resources.MaxUniformBufferBindings       = caps.maxUniformBufferBindings;
    resources.MaxShaderStorageBufferBindings = caps.maxShaderStorageBufferBindings;

    // Needed by point size clamping workaround
    resources.MaxPointSize = caps.maxAliasedPointSize;

    if (state.getClientMajorVersion() == 2 && !extensions.drawBuffersEXT)
    {
        resources.MaxDrawBuffers = 1;
    }

    // Geometry Shader constants
    resources.EXT_geometry_shader          = extensions.geometryShaderEXT;
    resources.OES_geometry_shader          = extensions.geometryShaderOES;
    resources.MaxGeometryUniformComponents = caps.maxShaderUniformComponents[ShaderType::Geometry];
    resources.MaxGeometryUniformBlocks     = caps.maxShaderUniformBlocks[ShaderType::Geometry];
    resources.MaxGeometryInputComponents   = caps.maxGeometryInputComponents;
    resources.MaxGeometryOutputComponents  = caps.maxGeometryOutputComponents;
    resources.MaxGeometryOutputVertices    = caps.maxGeometryOutputVertices;
    resources.MaxGeometryTotalOutputComponents = caps.maxGeometryTotalOutputComponents;
    resources.MaxGeometryTextureImageUnits = caps.maxShaderTextureImageUnits[ShaderType::Geometry];

    resources.MaxGeometryAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Geometry];
    resources.MaxGeometryAtomicCounters      = caps.maxShaderAtomicCounters[ShaderType::Geometry];
    resources.MaxGeometryShaderStorageBlocks = caps.maxShaderStorageBlocks[ShaderType::Geometry];
    resources.MaxGeometryShaderInvocations   = caps.maxGeometryShaderInvocations;
    resources.MaxGeometryImageUniforms       = caps.maxShaderImageUniforms[ShaderType::Geometry];

    // Tessellation Shader constants
    resources.EXT_tessellation_shader        = extensions.tessellationShaderEXT;
    resources.MaxTessControlInputComponents  = caps.maxTessControlInputComponents;
    resources.MaxTessControlOutputComponents = caps.maxTessControlOutputComponents;
    resources.MaxTessControlTextureImageUnits =
        caps.maxShaderTextureImageUnits[ShaderType::TessControl];
    resources.MaxTessControlUniformComponents =
        caps.maxShaderUniformComponents[ShaderType::TessControl];
    resources.MaxTessControlTotalOutputComponents = caps.maxTessControlTotalOutputComponents;
    resources.MaxTessControlImageUniforms  = caps.maxShaderImageUniforms[ShaderType::TessControl];
    resources.MaxTessControlAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::TessControl];
    resources.MaxTessControlAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessControl];

    resources.MaxTessPatchComponents = caps.maxTessPatchComponents;
    resources.MaxPatchVertices       = caps.maxPatchVertices;
    resources.MaxTessGenLevel        = caps.maxTessGenLevel;

    resources.MaxTessEvaluationInputComponents  = caps.maxTessEvaluationInputComponents;
    resources.MaxTessEvaluationOutputComponents = caps.maxTessEvaluationOutputComponents;
    resources.MaxTessEvaluationTextureImageUnits =
        caps.maxShaderTextureImageUnits[ShaderType::TessEvaluation];
    resources.MaxTessEvaluationUniformComponents =
        caps.maxShaderUniformComponents[ShaderType::TessEvaluation];
    resources.MaxTessEvaluationImageUniforms =
        caps.maxShaderImageUniforms[ShaderType::TessEvaluation];
    resources.MaxTessEvaluationAtomicCounters =
        caps.maxShaderAtomicCounters[ShaderType::TessEvaluation];
    resources.MaxTessEvaluationAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessEvaluation];

    // Subpixel bits.
    resources.SubPixelBits = static_cast<int>(caps.subPixelBits);
    return resources;
}

ShCompilerInstance::ShCompilerInstance() : mHandle(nullptr) {}

ShCompilerInstance::ShCompilerInstance(ShHandle handle,
                                       ShShaderOutput outputType,
                                       ShaderType shaderType)
    : mHandle(handle), mOutputType(outputType), mShaderType(shaderType)
{}

ShCompilerInstance::~ShCompilerInstance()
{
    ASSERT(mHandle == nullptr);
}

void ShCompilerInstance::destroy()
{
    if (mHandle != nullptr)
    {
        sh::Destruct(mHandle);
        mHandle = nullptr;
    }
}

ShCompilerInstance::ShCompilerInstance(ShCompilerInstance &&other)
    : mHandle(other.mHandle), mOutputType(other.mOutputType), mShaderType(other.mShaderType)
{
    other.mHandle = nullptr;
}

ShCompilerInstance &ShCompilerInstance::operator=(ShCompilerInstance &&other)
{
    mHandle       = other.mHandle;
    mOutputType   = other.mOutputType;
    mShaderType   = other.mShaderType;
    other.mHandle = nullptr;
    return *this;
}

ShHandle ShCompilerInstance::getHandle()
{
    return mHandle;
}

ShaderType ShCompilerInstance::getShaderType() const
{
    return mShaderType;
}

const std::string &ShCompilerInstance::getBuiltinResourcesString() const
{
    return sh::GetBuiltInResourcesString(mHandle);
}

ShShaderOutput ShCompilerInstance::getShaderOutputType() const
{
    return mOutputType;
}

}  // namespace gl
