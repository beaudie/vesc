//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.cpp:
//   ANGLE frame capture util implementation.
//

#include "libANGLE/frame_capture_utils.h"

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/State.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace angle
{

bool IsValidColorAttachmentBinding(GLenum binding, size_t colorAttachmentsCount)
{
    return binding == GL_BACK || (binding >= GL_COLOR_ATTACHMENT0 &&
                                  (binding - GL_COLOR_ATTACHMENT0) < colorAttachmentsCount);
}

Result ReadPixelsFromAttachment(const gl::Context *context,
                                gl::Framebuffer *framebuffer,
                                const gl::FramebufferAttachment &framebufferAttachment,
                                ScratchBuffer *scratchBuffer,
                                MemoryBuffer **pixels)
{
    gl::Extents extents       = framebufferAttachment.getSize();
    GLenum binding            = framebufferAttachment.getBinding();
    gl::InternalFormat format = *framebufferAttachment.getFormat().info;
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        format = framebuffer->getImplementation()->getImplementationColorReadFormat(context);
    }
    ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                         scratchBuffer->getInitialized(
                             format.pixelBytes * extents.width * extents.height, pixels, 0));
    ANGLE_TRY(framebuffer->readPixels(context, gl::Rectangle{0, 0, extents.width, extents.height},
                                      format.format, format.type, gl::PixelPackState{}, nullptr,
                                      (*pixels)->data()));
    return Result::Continue;
}

Result SerializeContext(gl::BinaryOutputStream *bos, const gl::Context *context)
{
    ScratchBuffer scratchBuffer(1);
    const gl::FramebufferManager &framebufferManager =
        context->getState().getFramebufferManagerForCapture();
    for (const auto &framebuffer : framebufferManager)
    {
        gl::Framebuffer *framebufferPtr = framebuffer.second;
        ANGLE_TRY(SerializeFramebuffer(context, bos, &scratchBuffer, framebufferPtr));
    }
    const gl::BufferManager &bufferManager = context->getState().getBufferManagerForCapture();
    for (const auto &buffer : bufferManager)
    {
        gl::Buffer *bufferPtr = buffer.second;
        ANGLE_TRY(SerializeBuffer(context, bos, &scratchBuffer, bufferPtr));
    }
    scratchBuffer.clear();
    return Result::Continue;
}

Result SerializeFramebuffer(const gl::Context *context,
                            gl::BinaryOutputStream *bos,
                            ScratchBuffer *scratchBuffer,
                            gl::Framebuffer *framebuffer)
{
    return SerializeFramebufferState(context, bos, scratchBuffer, framebuffer,
                                     framebuffer->getState());
}

Result SerializeFramebufferState(const gl::Context *context,
                                 gl::BinaryOutputStream *bos,
                                 ScratchBuffer *scratchBuffer,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState)
{
    bos->writeInt(framebufferState.id().value);
    bos->writeString(framebufferState.getLabel());
    bos->writeIntVector(framebufferState.getDrawBufferStates());
    bos->writeInt(framebufferState.getReadBufferState());
    bos->writeInt(framebufferState.getDefaultWidth());
    bos->writeInt(framebufferState.getDefaultHeight());
    bos->writeInt(framebufferState.getDefaultSamples());
    bos->writeInt(framebufferState.getDefaultFixedSampleLocations());
    bos->writeInt(framebufferState.getDefaultLayers());

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState.getColorAttachments();
    for (const gl::FramebufferAttachment &colorAttachment : colorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     colorAttachment));
        }
    }
    if (framebuffer->getDepthStencilAttachment())
    {
        ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                 *framebuffer->getDepthStencilAttachment()));
    }
    else
    {
        if (framebuffer->getDepthAttachment())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     *framebuffer->getDepthAttachment()));
        }
        if (framebuffer->getStencilAttachment())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     *framebuffer->getStencilAttachment()));
        }
    }
    return Result::Continue;
}

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      gl::BinaryOutputStream *bos,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment)
{
    bos->writeInt(framebufferAttachment.type());
    // serialize target variable
    bos->writeInt(framebufferAttachment.getBinding());
    if (framebufferAttachment.type() == GL_TEXTURE)
    {
        SerializeImageIndex(bos, framebufferAttachment.getTextureImageIndex());
    }
    bos->writeInt(framebufferAttachment.getNumViews());
    bos->writeInt(framebufferAttachment.isMultiview());
    bos->writeInt(framebufferAttachment.getBaseViewIndex());
    bos->writeInt(framebufferAttachment.getRenderToTextureSamples());

    GLenum prevReadBufferState = framebuffer->getReadBufferState();
    GLenum binding             = framebufferAttachment.getBinding();
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        framebuffer->setReadBuffer(framebufferAttachment.getBinding());
        ANGLE_TRY(framebuffer->syncState(context, GL_FRAMEBUFFER));
    }
    MemoryBuffer *pixelsPtr = nullptr;
    ANGLE_TRY(ReadPixelsFromAttachment(context, framebuffer, framebufferAttachment, scratchBuffer,
                                       &pixelsPtr));
    bos->writeBytes(pixelsPtr->data(), pixelsPtr->size());
    // Reset framebuffer state
    framebuffer->setReadBuffer(prevReadBufferState);
    return Result::Continue;
}

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex)
{
    bos->writeEnum(imageIndex.getType());
    bos->writeInt(imageIndex.getLevelIndex());
    bos->writeInt(imageIndex.getLayerIndex());
    bos->writeInt(imageIndex.getLayerCount());
}

Result SerializeBuffer(const gl::Context *context,
                       gl::BinaryOutputStream *bos,
                       ScratchBuffer *scratchBuffer,
                       gl::Buffer *buffer)
{
    SerializeBufferState(bos, buffer->getState());
    MemoryBuffer *dataPtr = nullptr;
    ANGLE_CHECK_GL_ALLOC(
        const_cast<gl::Context *>(context),
        scratchBuffer->getInitialized(static_cast<size_t>(buffer->getSize()), &dataPtr, 0));
    ANGLE_TRY(buffer->getSubData(context, 0, dataPtr->size(), dataPtr->data()));
    bos->writeBytes(dataPtr->data(), dataPtr->size());
    return Result::Continue;
}

void SerializeBufferState(gl::BinaryOutputStream *bos, const gl::BufferState &bufferState)
{
    bos->writeString(bufferState.getLabel());
    bos->writeEnum(bufferState.getUsage());
    bos->writeInt(bufferState.getSize());
    bos->writeInt(bufferState.getAccessFlags());
    bos->writeInt(bufferState.getAccess());
    bos->writeInt(bufferState.isMapped());
    bos->writeInt(bufferState.getMapOffset());
    bos->writeInt(bufferState.getMapLength());
}

void SerializeVersion(gl::BinaryOutputStream *bos, const gl::Version &version)
{
    bos->writeInt(version.major);
    bos->writeInt(version.minor);
}

void SerializeGLGlobalStates(gl::BinaryOutputStream *bos, const gl::State &state)
{
    bos->writeInt(state.getClientType());
    bos->writeInt(state.getContextPriority());
    bos->writeInt(state.getClientMajorVersion());
    bos->writeInt(state.getClientMinorVersion());
    // ContextID mID;

    //// Caps to use for validation
    // Caps mCaps;
    // TextureCapsMap mTextureCaps;
    SerializeExtensions(bos, state.getExtensions());
    SerializeLimitations(bos, state.getLimitations());

    // egl::ShareGroup *mShareGroup;

    //// Cached values from Context's caps
    // GLuint mMaxDrawBuffers;
    // GLuint mMaxCombinedTextureImageUnits;

    SerializeColor(bos, state.getColorClearValue());
    bos->writeInt(state.getDepthClearValue());
    bos->writeInt(state.getStencilClearValue());
    SerializeRasterizerState(bos, state.getRasterizerState());
    bos->writeInt(state.isScissorTestEnabled());
    SerializeRectangle(bos, state.getScissor());
    const gl::BlendStateArray &blendStateArray = state.getBlendStateArray();
    for (size_t i = 0; i < blendStateArray.size(); i++)
    {
        SerializeBlendState(bos, blendStateArray[i]);
    }
    // BlendStateExt mBlendStateExt;
    // ColorF mBlendColor;
    // bool mSampleAlphaToCoverage;
    // bool mSampleCoverage;
    // GLfloat mSampleCoverageValue;
    // bool mSampleCoverageInvert;
    // bool mSampleMask;
    // GLuint mMaxSampleMaskWords;
    // std::array<GLbitfield, MAX_SAMPLE_MASK_WORDS> mSampleMaskValues;

    // DepthStencilState mDepthStencil;
    // GLint mStencilRef;
    // GLint mStencilBackRef;

    // GLfloat mLineWidth;

    // GLenum mGenerateMipmapHint;
    // GLenum mTextureFilteringHint;
    // GLenum mFragmentShaderDerivativeHint;

    // const bool mBindGeneratesResource;
    // const bool mClientArraysEnabled;

    // Rectangle mViewport;
    // float mNearZ;
    // float mFarZ;

    // Framebuffer *mReadFramebuffer;
    // Framebuffer *mDrawFramebuffer;
    // BindingPointer<Renderbuffer> mRenderbuffer;
    // Program *mProgram;
    // BindingPointer<ProgramPipeline> mProgramPipeline;
    // ProgramExecutable *mExecutable;

    //// GL_ANGLE_provoking_vertex
    // ProvokingVertexConvention mProvokingVertex;

    // using VertexAttribVector = std::vector<VertexAttribCurrentValueData>;
    // VertexAttribVector mVertexAttribCurrentValues;  // From glVertexAttrib
    // VertexArray *mVertexArray;
    // ComponentTypeMask mCurrentValuesTypeMask;

    //// Texture and sampler bindings
    // size_t mActiveSampler;  // Active texture unit selector - GL_TEXTURE0

    // TextureBindingMap mSamplerTextures;

    //// Texture Completeness Caching
    //// ----------------------------
    //// The texture completeness cache uses dirty bits to avoid having to scan the list of textures
    //// each draw call. This gl::State class implements angle::Observer interface. When subject
    //// Textures have state changes, messages reach 'State' (also any observing Framebuffers) via
    /// the / onSubjectStateChange method (above). This then invalidates the completeness cache.
    ////
    //// Note this requires that we also invalidate the completeness cache manually on events like
    //// re-binding textures/samplers or a change in the program. For more information see the
    //// Observer.h header and the design doc linked there.

    //// A cache of complete textures. nullptr indicates unbound or incomplete.
    //// Don't use BindingPointer because this cache is only valid within a draw call.
    //// Also stores a notification channel to the texture itself to handle texture change events.
    // ActiveTexturesCache mActiveTexturesCache;
    // std::vector<angle::ObserverBinding> mCompleteTextureBindings;

    // ActiveTextureMask mTexturesIncompatibleWithSamplers;

    // SamplerBindingVector mSamplers;

    //// It would be nice to merge the image and observer binding. Same for textures.
    // std::vector<ImageUnit> mImageUnits;

    // ActiveQueryMap mActiveQueries;

    //// Stores the currently bound buffer for each binding point. It has an entry for the element
    //// array buffer but it should not be used. Instead this bind point is owned by the current
    //// vertex array object.
    // BoundBufferMap mBoundBuffers;

    // using BufferVector = std::vector<OffsetBindingPointer<Buffer>>;
    // BufferVector mUniformBuffers;
    // BufferVector mAtomicCounterBuffers;
    // BufferVector mShaderStorageBuffers;

    // BindingPointer<TransformFeedback> mTransformFeedback;

    // PixelUnpackState mUnpack;
    // PixelPackState mPack;

    // bool mPrimitiveRestart;

    // Debug mDebug;

    // bool mMultiSampling;
    // bool mSampleAlphaToOne;

    // GLenum mCoverageModulation;

    //// GL_EXT_sRGB_write_control
    // bool mFramebufferSRGB;

    //// GL_ANGLE_robust_resource_initialization
    // const bool mRobustResourceInit;

    //// GL_ANGLE_program_cache_control
    // const bool mProgramBinaryCacheEnabled;

    //// GL_ANGLE_webgl_compatibility
    // bool mTextureRectangleEnabled;

    //// GL_KHR_parallel_shader_compile
    // GLuint mMaxShaderCompilerThreads;

    //// GL_APPLE_clip_distance/GL_EXT_clip_cull_distance
    // ClipDistanceEnableBits mClipDistancesEnabled;

    //// GLES1 emulation: state specific to GLES1
    // GLES1State mGLES1State;

    // DirtyBits mDirtyBits;
    // DirtyObjects mDirtyObjects;
    // mutable AttributesMask mDirtyCurrentValues;
    // ActiveTextureMask mDirtyTextures;
    // ActiveTextureMask mDirtySamplers;
    // ImageUnitMask mDirtyImages;

    //// The Overlay object, used by the backend to render the overlay.
    // const OverlayType *mOverlay;

    //// OES_draw_buffers_indexed
    // DrawBufferMask mBlendFuncConstantAlphaDrawBuffers;
    // DrawBufferMask mBlendFuncConstantColorDrawBuffers;
    // bool mNoSimultaneousConstantColorAndAlphaBlendFunc;
}

void SerializeExtensions(gl::BinaryOutputStream *bos, const gl::Extensions &extensions)
{
    bos->writeInt(extensions.elementIndexUintOES);
    bos->writeInt(extensions.packedDepthStencilOES);
    bos->writeInt(extensions.readDepthNV);
    bos->writeInt(extensions.readStencilNV);
    bos->writeInt(extensions.depthBufferFloat2NV);
    bos->writeInt(extensions.getProgramBinaryOES);
    bos->writeInt(extensions.rgb8rgba8OES);
    bos->writeInt(extensions.textureFormatBGRA8888);
    bos->writeInt(extensions.readFormatBGRA);
    bos->writeInt(extensions.pixelBufferObjectNV);
    bos->writeInt(extensions.glSyncARB);
    bos->writeInt(extensions.mapBufferOES);
    bos->writeInt(extensions.mapBufferRange);
    bos->writeInt(extensions.colorBufferHalfFloat);
    bos->writeInt(extensions.textureHalfFloat);
    bos->writeInt(extensions.textureHalfFloatLinear);
    bos->writeInt(extensions.textureFormat2101010REV);
    bos->writeInt(extensions.textureFloatOES);
    bos->writeInt(extensions.textureFloatLinearOES);
    bos->writeInt(extensions.textureRG);
    bos->writeInt(extensions.textureCompressionDXT1);
    bos->writeInt(extensions.textureCompressionDXT3);
    bos->writeInt(extensions.textureCompressionDXT5);
    bos->writeInt(extensions.textureCompressionS3TCsRGB);
    bos->writeInt(extensions.textureCompressionASTCLDRKHR);
    bos->writeInt(extensions.textureCompressionASTCHDRKHR);
    bos->writeInt(extensions.textureCompressionASTCOES);
    bos->writeInt(extensions.textureCompressionSliced3dASTCKHR);
    bos->writeInt(extensions.textureCompressionBPTC);
    bos->writeInt(extensions.textureCompressionRGTC);
    bos->writeInt(extensions.compressedETC1RGB8TextureOES);
    bos->writeInt(extensions.compressedETC1RGB8SubTexture);
    bos->writeInt(extensions.compressedETC2RGB8TextureOES);
    bos->writeInt(extensions.compressedETC2sRGB8TextureOES);
    bos->writeInt(extensions.compressedETC2PunchthroughARGB8TextureOES);
    bos->writeInt(extensions.compressedETC2PunchthroughAsRGB8AlphaTextureOES);
    bos->writeInt(extensions.compressedETC2RGBA8TextureOES);
    bos->writeInt(extensions.compressedETC2sRGB8Alpha8TextureOES);
    bos->writeInt(extensions.compressedEACR11UnsignedTextureOES);
    bos->writeInt(extensions.compressedEACR11SignedTextureOES);
    bos->writeInt(extensions.compressedEACRG11UnsignedTextureOES);
    bos->writeInt(extensions.compressedEACRG11SignedTextureOES);
    bos->writeInt(extensions.compressedTextureETC);
    bos->writeInt(extensions.compressedTexturePVRTC);
    bos->writeInt(extensions.compressedTexturePVRTCsRGB);
    bos->writeInt(extensions.sRGB);
    bos->writeInt(extensions.sRGBR8EXT);
    bos->writeInt(extensions.depthTextureANGLE);
    bos->writeInt(extensions.depthTextureOES);
    bos->writeInt(extensions.depthTextureCubeMapOES);
    bos->writeInt(extensions.depth24OES);
    bos->writeInt(extensions.depth32OES);
    bos->writeInt(extensions.texture3DOES);
    bos->writeInt(extensions.textureStorage);
    bos->writeInt(extensions.textureNPOTOES);
    bos->writeInt(extensions.drawBuffers);
    bos->writeInt(extensions.drawBuffersIndexedEXT);
    bos->writeInt(extensions.drawBuffersIndexedOES);
    bos->writeInt(extensions.textureFilterAnisotropic);
    bos->writeInt(extensions.maxTextureAnisotropy);
    bos->writeInt(extensions.occlusionQueryBoolean);
    bos->writeInt(extensions.fenceNV);
    bos->writeInt(extensions.disjointTimerQuery);
    bos->writeInt(extensions.queryCounterBitsTimeElapsed);
    bos->writeInt(extensions.queryCounterBitsTimestamp);
    bos->writeInt(extensions.robustness);
    bos->writeInt(extensions.robustBufferAccessBehavior);
    bos->writeInt(extensions.blendMinMax);
    bos->writeInt(extensions.framebufferBlit);
    bos->writeInt(extensions.framebufferMultisample);
    bos->writeInt(extensions.multisampledRenderToTexture);
    bos->writeInt(extensions.instancedArraysANGLE);
    bos->writeInt(extensions.instancedArraysEXT);
    bos->writeInt(extensions.packReverseRowOrder);
    bos->writeInt(extensions.standardDerivativesOES);
    bos->writeInt(extensions.shaderTextureLOD);
    bos->writeInt(extensions.fragDepth);
    bos->writeInt(extensions.multiview);
    bos->writeInt(extensions.maxViews);
    bos->writeInt(extensions.multiview2);
    bos->writeInt(extensions.textureUsage);
    bos->writeInt(extensions.translatedShaderSource);
    bos->writeInt(extensions.fboRenderMipmapOES);
    bos->writeInt(extensions.discardFramebuffer);
    bos->writeInt(extensions.debugMarker);
    bos->writeInt(extensions.eglImageOES);
    bos->writeInt(extensions.eglImageExternalOES);
    bos->writeInt(extensions.eglImageExternalEssl3OES);
    bos->writeInt(extensions.eglImageExternalWrapModesEXT);
    bos->writeInt(extensions.eglSyncOES);
    bos->writeInt(extensions.memoryObject);
    bos->writeInt(extensions.memoryObjectFd);
    bos->writeInt(extensions.memoryObjectFuchsiaANGLE);
    bos->writeInt(extensions.semaphore);
    bos->writeInt(extensions.semaphoreFd);
    bos->writeInt(extensions.semaphoreFuchsiaANGLE);
    bos->writeInt(extensions.eglStreamConsumerExternalNV);
    bos->writeInt(extensions.unpackSubimage);
    bos->writeInt(extensions.packSubimage);
    bos->writeInt(extensions.noperspectiveInterpolationNV);
    bos->writeInt(extensions.vertexHalfFloatOES);
    bos->writeInt(extensions.vertexArrayObjectOES);
    bos->writeInt(extensions.vertexAttribType1010102OES);
    bos->writeInt(extensions.debug);
    bos->writeInt(extensions.maxDebugMessageLength);
    bos->writeInt(extensions.maxDebugLoggedMessages);
    bos->writeInt(extensions.maxDebugGroupStackDepth);
    bos->writeInt(extensions.maxLabelLength);
    bos->writeInt(extensions.noError);
    bos->writeInt(extensions.lossyETCDecode);
    bos->writeInt(extensions.bindUniformLocation);
    bos->writeInt(extensions.syncQuery);
    bos->writeInt(extensions.copyTexture);
    bos->writeInt(extensions.copyCompressedTexture);
    bos->writeInt(extensions.copyTexture3d);
    bos->writeInt(extensions.webglCompatibility);
    bos->writeInt(extensions.requestExtension);
    bos->writeInt(extensions.bindGeneratesResource);
    bos->writeInt(extensions.robustClientMemory);
    bos->writeInt(extensions.textureBorderClampOES);
    bos->writeInt(extensions.textureSRGBDecode);
    bos->writeInt(extensions.textureSRGBOverride);
    bos->writeInt(extensions.sRGBWriteControl);
    bos->writeInt(extensions.colorBufferFloatRGB);
    bos->writeInt(extensions.colorBufferFloatRGBA);
    bos->writeInt(extensions.eglImageArray);
    bos->writeInt(extensions.colorBufferFloat);
    bos->writeInt(extensions.multisampleCompatibility);
    bos->writeInt(extensions.framebufferMixedSamples);
    bos->writeInt(extensions.textureNorm16);
    bos->writeInt(extensions.surfacelessContextOES);
    bos->writeInt(extensions.clientArrays);
    bos->writeInt(extensions.robustResourceInitialization);
    bos->writeInt(extensions.programCacheControl);
    bos->writeInt(extensions.textureRectangle);
    bos->writeInt(extensions.geometryShader);
    bos->writeInt(extensions.pointSizeArrayOES);
    bos->writeInt(extensions.textureCubeMapOES);
    bos->writeInt(extensions.pointSpriteOES);
    bos->writeInt(extensions.drawTextureOES);
    bos->writeInt(extensions.explicitContextGles1);
    bos->writeInt(extensions.explicitContext);
    bos->writeInt(extensions.parallelShaderCompile);
    bos->writeInt(extensions.textureStorageMultisample2DArrayOES);
    bos->writeInt(extensions.multiviewMultisample);
    bos->writeInt(extensions.blendFuncExtended);
    bos->writeInt(extensions.maxDualSourceDrawBuffers);
    bos->writeInt(extensions.floatBlend);
    bos->writeInt(extensions.memorySize);
    bos->writeInt(extensions.textureMultisample);
    bos->writeInt(extensions.multiDraw);
    bos->writeInt(extensions.provokingVertex);
    bos->writeInt(extensions.textureFilteringCHROMIUM);
    bos->writeInt(extensions.loseContextCHROMIUM);
    bos->writeInt(extensions.textureExternalUpdateANGLE);
    bos->writeInt(extensions.baseVertexBaseInstance);
    bos->writeInt(extensions.getImageANGLE);
    bos->writeInt(extensions.drawElementsBaseVertexOES);
    bos->writeInt(extensions.drawElementsBaseVertexEXT);
    bos->writeInt(extensions.shaderNonConstGlobalInitializersEXT);
    bos->writeInt(extensions.gpuShader5EXT);
    bos->writeInt(extensions.webglVideoTexture);
    bos->writeInt(extensions.clipDistanceAPPLE);
    bos->writeInt(extensions.textureCubeMapArrayOES);
    bos->writeInt(extensions.textureCubeMapArrayEXT);
}

void SerializeLimitations(gl::BinaryOutputStream *bos, const gl::Limitations &limitations)
{
    bos->writeInt(limitations.noFrontFacingSupport);
    bos->writeInt(limitations.noSampleAlphaToCoverageSupport);
    bos->writeInt(limitations.attributeZeroRequiresZeroDivisorInEXT);
    bos->writeInt(limitations.noSeparateStencilRefsAndMasks);
    bos->writeInt(limitations.shadersRequireIndexedLoopValidation);
    bos->writeInt(limitations.noSimultaneousConstantColorAndAlphaBlendFunc);
    bos->writeInt(limitations.noFlexibleVaryingPacking);
    bos->writeInt(limitations.noDoubleBoundTransformFeedbackBuffers);
    bos->writeInt(limitations.noVertexAttributeAliasing);
}

void SerializeRasterizerState(gl::BinaryOutputStream *bos,
                              const gl::RasterizerState &rasterizerState)
{
    bos->writeInt(rasterizerState.cullFace);
    bos->writeEnum(rasterizerState.cullMode);
    bos->writeInt(rasterizerState.frontFace);
    bos->writeInt(rasterizerState.polygonOffsetFill);
    bos->writeInt(rasterizerState.polygonOffsetFactor);
    bos->writeInt(rasterizerState.polygonOffsetUnits);
    bos->writeInt(rasterizerState.pointDrawMode);
    bos->writeInt(rasterizerState.multiSample);
    bos->writeInt(rasterizerState.rasterizerDiscard);
    bos->writeInt(rasterizerState.dither);
}

void SerializeRectangle(gl::BinaryOutputStream *bos, const gl::Rectangle &rectangle)
{
    bos->writeInt(rectangle.x);
    bos->writeInt(rectangle.y);
    bos->writeInt(rectangle.width);
    bos->writeInt(rectangle.height);
}

void SerializeBlendState(gl::BinaryOutputStream *bos, const gl::BlendState &blendState)
{
    bos->writeInt(blendState.blend);
    bos->writeInt(blendState.sourceBlendRGB);
    bos->writeInt(blendState.destBlendRGB);
    bos->writeInt(blendState.sourceBlendAlpha);
    bos->writeInt(blendState.destBlendAlpha);
    bos->writeInt(blendState.blendEquationRGB);
    bos->writeInt(blendState.blendEquationAlpha);
    bos->writeInt(blendState.colorMaskRed);
    bos->writeInt(blendState.colorMaskGreen);
    bos->writeInt(blendState.colorMaskBlue);
    bos->writeInt(blendState.colorMaskAlpha);
}

}  // namespace angle
