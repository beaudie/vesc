//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RendererMtl.mm:
//    Implements the class methods for RendererMtl.
//

#include "libANGLE/renderer/metal/RendererMtl.h"

#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/mtl_common.h"

namespace rx
{
RendererMtl::RendererMtl() : mUtils(this) {}

RendererMtl::~RendererMtl() {}

angle::Result RendererMtl::initialize(egl::Display *display)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        mMetalDevice = [MTLCreateSystemDefaultDevice() ANGLE_MTL_AUTORELEASE];
        if (!mMetalDevice)
        {
            return angle::Result::Stop;
        }

        mCmdQueue.set([[mMetalDevice.get() newCommandQueue] ANGLE_MTL_AUTORELEASE]);

        mCapsInitialized = false;

        GlslangInitialize();
        initializeLimitations();

        ANGLE_TRY(mFormatTable.initialize(this));

        return mUtils.initialize();
    }
}
void RendererMtl::onDestroy()
{
    for (auto &nullTex : mNullTextures)
    {
        nullTex.reset();
    }
    mUtils.onDestroy();
    mCmdQueue.reset();
    mMetalDevice     = nil;
    mCapsInitialized = false;

    GlslangRelease();
}

std::string RendererMtl::getVendorString() const
{
    std::string vendorString = "Google Inc.";
    if (mMetalDevice)
    {
        vendorString += " Metal Renderer: ";
        vendorString += mMetalDevice.get().name.UTF8String;
    }

    return vendorString;
}

std::string RendererMtl::getRendererDescription() const
{
    std::string desc = "Metal Renderer";

    if (mMetalDevice)
    {
        desc += ": ";
        desc += mMetalDevice.get().name.UTF8String;
    }

    return desc;
}

gl::Caps RendererMtl::getNativeCaps() const
{
    ensureCapsInitialized();
    return mNativeCaps;
}
const gl::TextureCapsMap &RendererMtl::getNativeTextureCaps() const
{
    ensureCapsInitialized();
    return mNativeTextureCaps;
}
const gl::Extensions &RendererMtl::getNativeExtensions() const
{
    ensureCapsInitialized();
    return mNativeExtensions;
}

const mtl::TextureRef &RendererMtl::getNullTexture(const gl::Context *context,
                                                   gl::TextureType typeEnum)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    int type               = static_cast<int>(typeEnum);
    if (!mNullTextures[type])
    {
        // initialize content with zeros
        MTLRegion region           = MTLRegionMake2D(0, 0, 1, 1);
        const uint8_t zeroPixel[4] = {0, 0, 0, 255};

        const auto &rgbaFormat = getPixelFormat(angle::FormatID::R8G8B8A8_UNORM);

        switch (typeEnum)
        {
            case gl::TextureType::_2D:
                (void)(mtl::Texture::Make2DTexture(contextMtl, rgbaFormat, 1, 1, 1, false,
                                                   &mNullTextures[type]));
                mNullTextures[type]->replaceRegion(contextMtl, region, 0, 0, zeroPixel,
                                                   sizeof(zeroPixel));
                break;
            case gl::TextureType::CubeMap:
                (void)(mtl::Texture::MakeCubeTexture(contextMtl, rgbaFormat, 1, 1, false,
                                                     &mNullTextures[type]));
                for (int f = 0; f < 6; ++f)
                {
                    mNullTextures[type]->replaceRegion(contextMtl, region, 0, f, zeroPixel,
                                                       sizeof(zeroPixel));
                }
                break;
            default:
                UNREACHABLE();
                // NOTE(hqle): Support more texture types.
        }
        ASSERT(mNullTextures[type]);
    }

    return mNullTextures[type];
}

void RendererMtl::ensureCapsInitialized() const
{
    if (mCapsInitialized)
    {
        return;
    }

    mCapsInitialized = true;

    // Reset
    mNativeCaps = gl::Caps();

    // Fill extension and texture caps
    initializeExtensions();
    initializeTextureCaps();

    // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
    mNativeCaps.maxElementIndex  = std::numeric_limits<GLuint>::max() - 1;
    mNativeCaps.max3DTextureSize = 2048;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    mNativeCaps.max2DTextureSize          = 16384;
    mNativeCaps.maxVaryingVectors         = 31;
    mNativeCaps.maxVertexOutputComponents = 124;
#else
    if ([getMetalDevice() supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1])
    {
        mNativeCaps.max2DTextureSize          = 16384;
        mNativeCaps.maxVertexOutputComponents = 124;
        mNativeCaps.maxVaryingVectors         = mNativeCaps.maxVertexOutputComponents / 4;
    }
    else
    {
        mNativeCaps.max2DTextureSize          = 8192;
        mNativeCaps.maxVertexOutputComponents = 60;
        mNativeCaps.maxVaryingVectors         = mNativeCaps.maxVertexOutputComponents / 4;
    }
#endif

    mNativeCaps.maxArrayTextureLayers = 2048;
    mNativeCaps.maxLODBias            = 0;
    mNativeCaps.maxCubeMapTextureSize = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxRenderbufferSize   = mNativeCaps.max2DTextureSize;
    mNativeCaps.minAliasedPointSize   = 1;
    mNativeCaps.maxAliasedPointSize   = 511;

    mNativeCaps.minAliasedLineWidth = 1.0f;
    mNativeCaps.maxAliasedLineWidth = 1.0f;

    mNativeCaps.maxDrawBuffers       = mtl::kMaxRenderTargets;
    mNativeCaps.maxFramebufferWidth  = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxFramebufferHeight = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxColorAttachments  = mtl::kMaxRenderTargets;
    mNativeCaps.maxViewportWidth     = mNativeCaps.max2DTextureSize;
    mNativeCaps.maxViewportHeight    = mNativeCaps.max2DTextureSize;

    // NOTE(hqle): MSAA
    mNativeCaps.maxSampleMaskWords     = 0;
    mNativeCaps.maxColorTextureSamples = 1;
    mNativeCaps.maxDepthTextureSamples = 1;
    mNativeCaps.maxIntegerSamples      = 1;

    mNativeCaps.maxVertexAttributes           = mtl::kMaxVertexAttribs;
    mNativeCaps.maxVertexAttribBindings       = mtl::kMaxVertexAttribs;
    mNativeCaps.maxVertexAttribRelativeOffset = std::numeric_limits<GLint>::max();
    mNativeCaps.maxVertexAttribStride         = std::numeric_limits<GLint>::max();

    mNativeCaps.maxElementsIndices  = std::numeric_limits<GLuint>::max();
    mNativeCaps.maxElementsVertices = std::numeric_limits<GLuint>::max();

    // Looks like all floats are IEEE according to the docs here:
    mNativeCaps.vertexHighpFloat.setIEEEFloat();
    mNativeCaps.vertexMediumpFloat.setIEEEFloat();
    mNativeCaps.vertexLowpFloat.setIEEEFloat();
    mNativeCaps.fragmentHighpFloat.setIEEEFloat();
    mNativeCaps.fragmentMediumpFloat.setIEEEFloat();
    mNativeCaps.fragmentLowpFloat.setIEEEFloat();

    mNativeCaps.vertexHighpInt.setTwosComplementInt(32);
    mNativeCaps.vertexMediumpInt.setTwosComplementInt(32);
    mNativeCaps.vertexLowpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentHighpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentMediumpInt.setTwosComplementInt(32);
    mNativeCaps.fragmentLowpInt.setTwosComplementInt(32);

    GLuint maxUniformVectors = mtl::kDefaultUniformsMaxSize / (sizeof(GLfloat) * 4);

    const GLuint maxUniformComponents = maxUniformVectors * 4;

    // Uniforms are implemented using a uniform buffer, so the max number of uniforms we can
    // support is the max buffer range divided by the size of a single uniform (4X float).
    mNativeCaps.maxVertexUniformVectors                              = maxUniformVectors;
    mNativeCaps.maxShaderUniformComponents[gl::ShaderType::Vertex]   = maxUniformComponents;
    mNativeCaps.maxFragmentUniformVectors                            = maxUniformVectors;
    mNativeCaps.maxShaderUniformComponents[gl::ShaderType::Fragment] = maxUniformComponents;

    // NOTE(hqle): support UBO (ES 3.0 feature)
    mNativeCaps.maxShaderUniformBlocks[gl::ShaderType::Vertex]   = 0;
    mNativeCaps.maxShaderUniformBlocks[gl::ShaderType::Fragment] = 0;
    mNativeCaps.maxCombinedUniformBlocks                         = 0;

    // Note that we currently implement textures as combined image+samplers, so the limit is
    // the minimum of supported samplers and sampled images.
    mNativeCaps.maxCombinedTextureImageUnits                         = mtl::kMaxShaderSamplers;
    mNativeCaps.maxShaderTextureImageUnits[gl::ShaderType::Fragment] = mtl::kMaxShaderSamplers;
    mNativeCaps.maxShaderTextureImageUnits[gl::ShaderType::Vertex]   = mtl::kMaxShaderSamplers;

    // NOTE(hqle): support storage buffer.
    const uint32_t maxPerStageStorageBuffers                     = 0;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Vertex]   = maxPerStageStorageBuffers;
    mNativeCaps.maxShaderStorageBlocks[gl::ShaderType::Fragment] = maxPerStageStorageBuffers;
    mNativeCaps.maxCombinedShaderStorageBlocks                   = maxPerStageStorageBuffers;

    // Fill in additional limits for UBOs and SSBOs.
    mNativeCaps.maxUniformBufferBindings     = 0;
    mNativeCaps.maxUniformBlockSize          = 0;
    mNativeCaps.uniformBufferOffsetAlignment = 0;

    mNativeCaps.maxShaderStorageBufferBindings     = 0;
    mNativeCaps.maxShaderStorageBlockSize          = 0;
    mNativeCaps.shaderStorageBufferOffsetAlignment = 0;

    // NOTE(hqle): support UBO
    for (gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        mNativeCaps.maxCombinedShaderUniformComponents[shaderType] = maxUniformComponents;
    }

    mNativeCaps.maxCombinedShaderOutputResources = 0;

    mNativeCaps.maxTransformFeedbackInterleavedComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
    mNativeCaps.maxTransformFeedbackSeparateAttributes =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS;
    mNativeCaps.maxTransformFeedbackSeparateComponents =
        gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS;

    // NOTE(hqle): support MSAA.
    mNativeCaps.maxSamples = 1;
}

void RendererMtl::initializeExtensions() const
{
    // Reset
    mNativeExtensions = gl::Extensions();

    // Enable this for simple buffer readback testing, but some functionality is missing.
    // NOTE(hqle): Support full mapBufferRange extension.
    mNativeExtensions.mapBuffer              = true;
    mNativeExtensions.mapBufferRange         = false;
    mNativeExtensions.textureStorage         = true;
    mNativeExtensions.drawBuffers            = false;
    mNativeExtensions.fragDepth              = true;
    mNativeExtensions.framebufferBlit        = false;
    mNativeExtensions.framebufferMultisample = false;
    mNativeExtensions.copyTexture            = false;
    mNativeExtensions.copyCompressedTexture  = false;
    mNativeExtensions.debugMarker            = false;
    mNativeExtensions.robustness             = true;
    mNativeExtensions.textureBorderClamp     = false;  // not implemented yet
    mNativeExtensions.translatedShaderSource = true;
    mNativeExtensions.discardFramebuffer     = true;

    // Enable EXT_blend_minmax
    mNativeExtensions.blendMinMax = true;

    mNativeExtensions.eglImage         = false;
    mNativeExtensions.eglImageExternal = false;
    // NOTE(hqle): Support GL_OES_EGL_image_external_essl3.
    mNativeExtensions.eglImageExternalEssl3 = false;

    mNativeExtensions.memoryObject   = false;
    mNativeExtensions.memoryObjectFd = false;

    mNativeExtensions.semaphore   = false;
    mNativeExtensions.semaphoreFd = false;

    // TODO: Enable this always and emulate instanced draws if any divisor exceeds the maximum
    // supported.  http://anglebug.com/2672
    mNativeExtensions.instancedArraysANGLE = false;

    mNativeExtensions.robustBufferAccessBehavior = false;

    mNativeExtensions.eglSync = false;

    // NOTE(hqle): support occlusion query
    mNativeExtensions.occlusionQueryBoolean = false;

    mNativeExtensions.disjointTimerQuery          = false;
    mNativeExtensions.queryCounterBitsTimeElapsed = false;
    mNativeExtensions.queryCounterBitsTimestamp   = false;

    mNativeExtensions.textureFilterAnisotropic = true;
    mNativeExtensions.maxTextureAnisotropy     = 16;

    // NOTE(hqle): Support true NPOT textures.
    mNativeExtensions.textureNPOT = false;

    mNativeExtensions.texture3DOES = false;

    mNativeExtensions.standardDerivatives = true;

    mNativeExtensions.elementIndexUint = true;
}

void RendererMtl::initializeTextureCaps() const
{
    mNativeTextureCaps.clear();

    mFormatTable.generateTextureCaps(this, &mNativeTextureCaps,
                                     &mNativeCaps.compressedTextureFormats);

    // Re-verify texture extensions.
    mNativeExtensions.setTextureExtensionSupport(mNativeTextureCaps);
}

void RendererMtl::initializeLimitations()
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
//   Texture swizzle is only supported if macos sdk 10.15 is present
#    if defined(__MAC_10_15)
    if (ANGLE_APPLE_AVAILABLE_XC(10.15, 13.0))
    {
        // The runtime OS must be MacOS 10.15+ or Mac Catalyst for this to be supported:
        mNativeLimitations.hasTextureSwizzle = [getMetalDevice() supportsFamily:MTLGPUFamilyMac2];
    }
#    endif
#elif TARGET_OS_IOS
    // Base Vertex drawing is only supported since GPU family 3.
    if (![getMetalDevice() supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1])
    {
        mNativeLimitations.hasBaseVertexInstancedDraw = false;
    }
    if (![getMetalDevice() supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1])
    {
        mNativeLimitations.hasNonUniformDispatch = false;
    }

#    if !TARGET_OS_SIMULATOR
    mNativeLimitations.allowSeparatedDepthStencilBuffers = true;
#    endif
#endif
}
}
