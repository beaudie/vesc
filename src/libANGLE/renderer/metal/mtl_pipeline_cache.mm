//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_pipeline_cache.mm:
//    Defines classes for caching of mtl pipelines
//

#include "libANGLE/renderer/metal/mtl_pipeline_cache.h"

#include <tuple>

#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/mtl_msl_utils.h"
#include "libANGLE/renderer/metal/shaders/constants.h"

namespace rx
{
namespace mtl
{

namespace
{

constexpr unsigned int kMaxVertexPullingFunctions = 256;

// The cache tries to clean up this many states at once.
constexpr unsigned int kGCLimit = 32;

constexpr char kVertexPullingLibFuncName[] = "ANGLE_pullVertexAsUInt4";

NSString *GetVertexPullingSpecializedLibFunctionName(VertexPullingFunctionKey key)
{
    return [NSString stringWithFormat:@"%s%zu", kVertexPullingLibFuncName, key.hash()];
}

// Returns {isDefaultAttribute, divisor } pair.
std::tuple<BOOL, uint32_t> GetVertexPullingStepFunctionParams(VertexPullingFunctionKey key)
{
    if (key.attribute.bufferIndex == kDefaultAttribsBindingIndex)
    {
        return {YES, 0};
    }
    switch (key.layout.stepFunction)
    {
        case MTLVertexStepFunctionPerVertex:
            return {NO, 0};
        case MTLVertexStepFunctionPerInstance:
            return {NO, key.layout.stepRate};
    }
    UNREACHABLE();
    return {YES, 0};
}

bool HasDefaultAttribs(const RenderPipelineDesc &rpdesc)
{
    const VertexDesc &desc = rpdesc.vertexDescriptor;
    for (uint8_t i = 0; i < desc.numAttribs; ++i)
    {
        if (desc.attributes[i].bufferIndex == kDefaultAttribsBindingIndex)
        {
            return true;
        }
    }

    return false;
}

bool HasValidRenderTarget(const mtl::ContextDevice &device,
                          const MTLRenderPipelineDescriptor *descriptor)
{
    const NSUInteger maxColorRenderTargets = GetMaxNumberOfRenderTargetsForDevice(device);
    for (NSUInteger i = 0; i < maxColorRenderTargets; ++i)
    {
        auto colorAttachment = descriptor.colorAttachments[i];
        if (colorAttachment && colorAttachment.pixelFormat != MTLPixelFormatInvalid)
        {
            return true;
        }
    }

    if (descriptor.depthAttachmentPixelFormat != MTLPixelFormatInvalid)
    {
        return true;
    }

    if (descriptor.stencilAttachmentPixelFormat != MTLPixelFormatInvalid)
    {
        return true;
    }

    return false;
}

angle::Result ValidateRenderPipelineState(ContextMtl *context,
                                          const MTLRenderPipelineDescriptor *descriptor)
{
    const mtl::ContextDevice &device = context->getMetalDevice();

    if (!context->getDisplay()->getFeatures().allowRenderpassWithoutAttachment.enabled &&
        !HasValidRenderTarget(device, descriptor))
    {
        ANGLE_MTL_HANDLE_ERROR(
            context, "Render pipeline requires at least one render target for this device.",
            GL_INVALID_OPERATION);
        return angle::Result::Stop;
    }

    // Ensure the device can support the storage requirement for render targets.
    if (DeviceHasMaximumRenderTargetSize(device))
    {
        NSUInteger maxSize = GetMaxRenderTargetSizeForDeviceInBytes(device);
        NSUInteger renderTargetSize =
            ComputeTotalSizeUsedForMTLRenderPipelineDescriptor(descriptor, context, device);
        if (renderTargetSize > maxSize)
        {
            std::stringstream errorStream;
            errorStream << "This set of render targets requires " << renderTargetSize
                        << " bytes of pixel storage. This device supports " << maxSize << " bytes.";
            ANGLE_MTL_HANDLE_ERROR(context, errorStream.str().c_str(), GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }
    }

    return angle::Result::Continue;
}

// Compile and create "pullVertexAsUInt4" function specialed for the VertexPullingFunctionKey.
angle::Result CreateVertexPullingSpecializedLibFunction(ContextMtl *context,
                                                        const VertexPullingFunctionKey &key,
                                                        AutoObjCPtr<id<MTLFunction>> *outFunction)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        MTLFunctionConstantValues *funcConstants = [MTLFunctionConstantValues new];

        BOOL flag = key.attribute.vertexPullingOffsetIsAligned;
        [funcConstants setConstantValue:&flag
                                   type:MTLDataTypeBool
                               withName:@"kVertexPullingOffsetIsAligned"];

        uint value = key.attribute.vertexPullingType;
        [funcConstants setConstantValue:&value type:MTLDataTypeUInt withName:@"kVertexPullingType"];

        value = key.attribute.vertexPullingComponentCount;
        [funcConstants setConstantValue:&value
                                   type:MTLDataTypeUInt
                               withName:@"kVertexPullingComponentCount"];

        value = key.attribute.offset;
        [funcConstants setConstantValue:&value
                                   type:MTLDataTypeUInt
                               withName:@"kVertexPullingOffset"];

        value = key.layout.stride;
        [funcConstants setConstantValue:&value
                                   type:MTLDataTypeUInt
                               withName:@"kVertexPullingStride"];

        BOOL isDefaultAttrib;
        uint32_t divisor;
        std::tie(isDefaultAttrib, divisor) = GetVertexPullingStepFunctionParams(key);
        [funcConstants setConstantValue:&divisor
                                   type:MTLDataTypeUInt
                               withName:@"kVertexPullingDivisor"];

        [funcConstants setConstantValue:&isDefaultAttrib
                                   type:MTLDataTypeBool
                               withName:@"kVertexPullingUseDefaultAttribs"];

        value = key.attribute.vertexPullingConvertMode;
        [funcConstants setConstantValue:&value
                                   type:MTLDataTypeUInt
                               withName:@"kVertexPullingConvertMode"];

        MTLFunctionDescriptor *libFuncDesc = [MTLFunctionDescriptor new];
        libFuncDesc.name            = [NSString stringWithUTF8String:kVertexPullingLibFuncName];
        libFuncDesc.specializedName = GetVertexPullingSpecializedLibFunctionName(key);
        libFuncDesc.constantValues  = funcConstants;

        NSError *err = nil;
        auto libFunc = [context->getDisplay()->getExtendedDefaultShadersLib()
            newFunctionWithDescriptor:libFuncDesc
                                error:&err];
        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        *outFunction = adoptObjCObj(libFunc);
    }

    return angle::Result::Continue;
}

// This will construct a shader code to use the specialized function from
// CreateSpecializedVertexPullingLibFunction() to fetch the vertex attribute at the specified index.
// This allows runtime to choose the corresponding precompiled pulling function based on the
// attribute's offset, stride, format, etc. without the need to re-compile the whole shader
// program.
angle::Result StitchVertexPullingCode(ContextMtl *context,
                                      uint32_t attributeIdx,
                                      const VertexPullingFunctionKey &key,
                                      const AutoObjCPtr<id<MTLFunction>> &pullingLibFunction,
                                      AutoObjCPtr<id<MTLFunction>> *outStichedFunction)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        MTLFunctionStitchingInputNode *bufferInput =
            [[MTLFunctionStitchingInputNode alloc] initWithArgumentIndex:0];
        MTLFunctionStitchingInputNode *defaultAttribsBufferInput =
            [[MTLFunctionStitchingInputNode alloc] initWithArgumentIndex:1];
        MTLFunctionStitchingInputNode *vertexIndexInput =
            [[MTLFunctionStitchingInputNode alloc] initWithArgumentIndex:2];
        MTLFunctionStitchingInputNode *instanceIndexInput =
            [[MTLFunctionStitchingInputNode alloc] initWithArgumentIndex:3];
        MTLFunctionStitchingInputNode *baseInstanceInput =
            [[MTLFunctionStitchingInputNode alloc] initWithArgumentIndex:4];

        MTLFunctionStitchingFunctionNode *funcCall = [[MTLFunctionStitchingFunctionNode alloc]
                   initWithName:GetVertexPullingSpecializedLibFunctionName(key)
                      arguments:@[
                          bufferInput, defaultAttribsBufferInput, vertexIndexInput,
                          instanceIndexInput, baseInstanceInput
                      ]
            controlDependencies:@[]];

        NSString *stitchedFunctionName = [NSString
            stringWithUTF8String:MslGetVertexPullingHelperFunctionName(attributeIdx).c_str()];
        MTLFunctionStitchingGraph *funcCallGraph =
            [[MTLFunctionStitchingGraph alloc] initWithFunctionName:stitchedFunctionName
                                                              nodes:@[]
                                                         outputNode:funcCall
                                                         attributes:@[]];

        MTLStitchedLibraryDescriptor *stitchedLibDesc = [MTLStitchedLibraryDescriptor new];
        stitchedLibDesc.functions                     = @[ pullingLibFunction.get() ];
        stitchedLibDesc.functionGraphs                = @[ funcCallGraph ];

        NSError *err = nil;
        auto stitchedLibrary =
            context->getMetalDevice().newLibraryWithStitchedDescriptor(stitchedLibDesc, &err);

        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        MTLFunctionDescriptor *stitchedFuncDesc = [MTLFunctionDescriptor new];
        stitchedFuncDesc.name                   = stitchedFunctionName;

        *outStichedFunction =
            adoptObjCObj([stitchedLibrary.get() newFunctionWithDescriptor:stitchedFuncDesc
                                                                    error:&err]);
        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }
    }
    return angle::Result::Continue;
}

// Create render pipeline state with manual vertex fetching in shader.
angle::Result CreateRenderPipelineStateWithVertexPulling(
    ContextMtl *context,
    const PipelineKey &key,
    PipelineCache::VertexPullFunctionMap *vertexPullingFuncCache,
    MTLRenderPipelineDescriptor *objCDesc,
    AutoObjCPtr<id<MTLRenderPipelineState>> *outRenderPipeline)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        NSMutableArray *stitchedFunctions = [NSMutableArray new];
        const VertexDesc &vertexDesc      = key.pipelineDesc.vertexDescriptor;
        for (uint8_t i = 0; i < vertexDesc.numAttribs; ++i)
        {
            VertexPullingFunctionKey attribKey;
            attribKey.attribute = vertexDesc.attributes[i];
            if (attribKey.attribute.vertexPullingComponentCount == 0)
            {
                // disabled attribute.
                continue;
            }
            if (vertexDesc.attributes[i].bufferIndex == kDefaultAttribsBindingIndex)
            {
                attribKey.layout = {};
            }
            else
            {
                const auto &layout = vertexDesc.layouts[vertexDesc.attributes[i].bufferIndex];
                if (layout.stepFunction == kVertexStepFunctionInvalid)
                {
                    // disabled attribute
                    continue;
                }
                attribKey.layout = layout;
            }

            // Find the corresponding vertex pulling function in the cache.
            auto libFuncIter = vertexPullingFuncCache->Get(attribKey);
            if (libFuncIter == vertexPullingFuncCache->end())
            {
                angle::TrimCache(kMaxVertexPullingFunctions, kGCLimit, "vertex pulling function",
                                 vertexPullingFuncCache);

                AutoObjCPtr<id<MTLFunction>> pullingFunction;
                ANGLE_TRY(CreateVertexPullingSpecializedLibFunction(context, attribKey,
                                                                    &pullingFunction));

                libFuncIter =
                    vertexPullingFuncCache->Put(std::move(attribKey), std::move(pullingFunction));
            }

            // Generate stitched function to load this vertex attribute.
            AutoObjCPtr<id<MTLFunction>> stitchedFunction;
            ANGLE_TRY(StitchVertexPullingCode(context, i, attribKey, libFuncIter->second,
                                              &stitchedFunction));
            [stitchedFunctions addObject:stitchedFunction];
        }  // for (attributes)

        // Link stitched functions.
        MTLLinkedFunctions *linkedFuncs = [MTLLinkedFunctions new];
        linkedFuncs.privateFunctions    = stitchedFunctions;

        objCDesc.vertexLinkedFunctions = linkedFuncs;

        // Create pipeline state
        NSError *err = nil;
        auto newState =
            context->getMetalDevice().newRenderPipelineStateWithDescriptor(objCDesc, &err);
        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        *outRenderPipeline = newState;
    }
    return angle::Result::Continue;
}

// Create render pipeline state with auto vertex fetching from Metal API.
angle::Result CreateRenderPipelineStateWithoutVertexPulling(
    ContextMtl *context,
    const PipelineKey &key,
    MTLRenderPipelineDescriptor *objCDesc,
    AutoObjCPtr<id<MTLRenderPipelineState>> *outRenderPipeline)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        // Special attribute slot for default attribute
        if (HasDefaultAttribs(key.pipelineDesc))
        {
            auto defaultAttribLayoutObjCDesc = adoptObjCObj<MTLVertexBufferLayoutDescriptor>(
                [[MTLVertexBufferLayoutDescriptor alloc] init]);
            defaultAttribLayoutObjCDesc.get().stepFunction = MTLVertexStepFunctionConstant;
            defaultAttribLayoutObjCDesc.get().stepRate     = 0;
            defaultAttribLayoutObjCDesc.get().stride = kDefaultAttributeSize * kMaxVertexAttribs;

            [objCDesc.vertexDescriptor.layouts setObject:defaultAttribLayoutObjCDesc
                                      atIndexedSubscript:kDefaultAttribsBindingIndex];
        }
        // Create pipeline state
        NSError *err = nil;
        auto newState =
            context->getMetalDevice().newRenderPipelineStateWithDescriptor(objCDesc, &err);
        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        *outRenderPipeline = newState;
    }
    return angle::Result::Continue;
}

angle::Result CreateRenderPipelineState(
    ContextMtl *context,
    const PipelineKey &key,
    PipelineCache::VertexPullFunctionMap *vertexPullingFuncCache,
    AutoObjCPtr<id<MTLRenderPipelineState>> *outRenderPipeline)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        ASSERT(key.isRenderPipeline());
        if (!key.vertexShader)
        {
            // Render pipeline without vertex shader is invalid.
            ANGLE_MTL_HANDLE_ERROR(context, "Render pipeline without vertex shader is invalid.",
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        const bool useVertexPulling = context->getDisplay()->getFeatures().useVertexPulling.enabled;
        auto objCDesc               = key.pipelineDesc.createMetalDesc(
            /*useMetalVertexDesc=*/!useVertexPulling, key.vertexShader, key.fragmentShader);

        ANGLE_TRY(ValidateRenderPipelineState(context, objCDesc));

        if (useVertexPulling)
        {
            ANGLE_TRY(CreateRenderPipelineStateWithVertexPulling(
                context, key, vertexPullingFuncCache, objCDesc.get(), outRenderPipeline));
        }
        else
        {
            ANGLE_TRY(CreateRenderPipelineStateWithoutVertexPulling(context, key, objCDesc.get(),
                                                                    outRenderPipeline));
        }

        return angle::Result::Continue;
    }
}

angle::Result CreateComputePipelineState(
    ContextMtl *context,
    const PipelineKey &key,
    AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        ASSERT(!key.isRenderPipeline());
        if (!key.computeShader)
        {
            ANGLE_MTL_HANDLE_ERROR(context, "Compute pipeline without a shader is invalid.",
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        const mtl::ContextDevice &metalDevice = context->getMetalDevice();

        // Create pipeline state
        NSError *err  = nil;
        auto newState = metalDevice.newComputePipelineStateWithFunction(key.computeShader, &err);
        if (err)
        {
            ANGLE_MTL_HANDLE_ERROR(context, mtl::FormatMetalErrorMessage(err).c_str(),
                                   GL_INVALID_OPERATION);
            return angle::Result::Stop;
        }

        *outComputePipeline = newState;
        return angle::Result::Continue;
    }
}

}  // namespace

bool PipelineKey::isRenderPipeline() const
{
    if (vertexShader)
    {
        ASSERT(!computeShader);
        return true;
    }
    else
    {
        ASSERT(computeShader);
        return false;
    }
}

bool PipelineKey::operator==(const PipelineKey &rhs) const
{
    if (isRenderPipeline() != rhs.isRenderPipeline())
    {
        return false;
    }

    if (isRenderPipeline())
    {
        return std::tie(vertexShader, fragmentShader, pipelineDesc) ==
               std::tie(rhs.vertexShader, rhs.fragmentShader, rhs.pipelineDesc);
    }
    else
    {
        return computeShader == rhs.computeShader;
    }
}

size_t PipelineKey::hash() const
{
    if (isRenderPipeline())
    {
        return angle::HashMultiple(vertexShader.get(), fragmentShader.get(), pipelineDesc);
    }
    else
    {
        return angle::HashMultiple(computeShader.get());
    }
}

bool VertexPullingFunctionKey::operator==(const VertexPullingFunctionKey &rhs) const
{
    // For caching, ee don't consider bufferIdx & metalFormat when comparing.
    VertexAttributeDesc lhsAttribVal = attribute;
    lhsAttribVal.bufferIndex         = 0;
    lhsAttribVal.metalFormat         = 0;

    VertexAttributeDesc rhsAttribVal = rhs.attribute;
    rhsAttribVal.bufferIndex         = 0;
    rhsAttribVal.metalFormat         = 0;

    return layout == rhs.layout && lhsAttribVal == rhsAttribVal;
}
size_t VertexPullingFunctionKey::hash() const
{
    uint64_t key[2];

    // For caching, ignore bufferIndex & metalFormat
    VertexAttributeDesc hashableAttribVal = attribute;
    hashableAttribVal.bufferIndex         = 0;
    hashableAttribVal.metalFormat         = 0;

    key[0] = (static_cast<uint64_t>(layout.stepRate) << 32) | layout.stride;
    key[1] = static_cast<uint64_t>(layout.stepFunction) << 32;
    key[1] = key[1] | hashableAttribVal.asInt;

    return angle::ComputeGenericHash(key, sizeof(key));
}

PipelineCache::PipelineCache()
    : mPipelineCache(kMaxPipelines), mVertexPullFunctionsCache(kMaxVertexPullingFunctions)
{}

angle::Result PipelineCache::getRenderPipeline(
    ContextMtl *context,
    id<MTLFunction> vertexShader,
    id<MTLFunction> fragmentShader,
    const RenderPipelineDesc &desc,
    AutoObjCPtr<id<MTLRenderPipelineState>> *outRenderPipeline)
{
    PipelineKey key;
    key.vertexShader.retainAssign(vertexShader);
    key.fragmentShader.retainAssign(fragmentShader);
    key.pipelineDesc = desc;

    auto iter = mPipelineCache.Get(key);
    if (iter != mPipelineCache.end())
    {
        // Should be no way that this key matched a compute pipeline entry
        ASSERT(iter->second.renderPipeline);
        *outRenderPipeline = iter->second.renderPipeline;
        return angle::Result::Continue;
    }

    angle::TrimCache(kMaxPipelines, kGCLimit, "render pipeline", &mPipelineCache);

    PipelineVariant newPipeline;
    ANGLE_TRY(CreateRenderPipelineState(context, key, &mVertexPullFunctionsCache,
                                        &newPipeline.renderPipeline));

    iter = mPipelineCache.Put(std::move(key), std::move(newPipeline));

    *outRenderPipeline = iter->second.renderPipeline;
    return angle::Result::Continue;
}

angle::Result PipelineCache::getComputePipeline(
    ContextMtl *context,
    id<MTLFunction> computeShader,
    AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline)
{
    PipelineKey key;
    key.computeShader.retainAssign(computeShader);

    auto iter = mPipelineCache.Get(key);
    if (iter != mPipelineCache.end())
    {
        // Should be no way that this key matched a render pipeline entry
        ASSERT(iter->second.computePipeline);
        *outComputePipeline = iter->second.computePipeline;
        return angle::Result::Continue;
    }

    angle::TrimCache(kMaxPipelines, kGCLimit, "render pipeline", &mPipelineCache);

    PipelineVariant newPipeline;
    ANGLE_TRY(CreateComputePipelineState(context, key, &newPipeline.computePipeline));

    iter = mPipelineCache.Put(std::move(key), std::move(newPipeline));

    *outComputePipeline = iter->second.computePipeline;
    return angle::Result::Continue;
}

}  // namespace mtl
}  // namespace rx
