//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_context_device.mm:
//      Implementation of Metal framework's MTLDevice wrapper per context.
//

#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#if ANGLE_USE_METAL_OWNERSHIP_IDENTITY
#    include "libANGLE/renderer/metal/mtl_resource_spi.h"
#endif

namespace rx
{
namespace mtl
{

namespace
{

void fillPrivateTextureWithNonZeroValues(const ContextDevice *contextDevice,
                                         const id<MTLTexture> &tex,
                                         const MTLTextureDescriptor *desc)
{
    id<MTLCommandQueue> commandQueue             = [contextDevice->get() newCommandQueue];
    id<MTLCommandBuffer> commandBuffer           = [commandQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];

    // Do NOT replace with call to Texture::replaceRegion!
    // The point of this code is to fill the texture with non-zero values
    // to test that other parts of the code correctly clear the texture.
    // As such it needs to try to not affect other state of the system.
    const size_t numFaces =
        (desc.textureType == MTLTextureTypeCube || desc.textureType == MTLTextureTypeCubeArray) ? 6
                                                                                                : 1;

    // allocate space for the largest mip
    const angle::FormatID formatID   = Format::MetalToAngleFormatID(desc.pixelFormat);
    const angle::Format &angleFormat = angle::Format::Get(formatID);

    size_t bytesPerPixel     = angleFormat.pixelBytes;
    size_t bytesPerRowLevel0 = desc.width * bytesPerPixel;
    size_t size              = bytesPerRowLevel0 * desc.height * desc.depth;
    std::vector<uint8_t> data(size, 0xBD);
    id<MTLBuffer> buffer = [contextDevice->get() newBufferWithBytes:data.data()
                                                             length:size
                                                            options:MTLResourceStorageModeShared];

    MTLBlitOption blitOption = angleFormat.hasDepthOrStencilBits()
                                   ? MTLBlitOptionDepthFromDepthStencil
                                   : MTLBlitOptionNone;

    const size_t numSlices = desc.arrayLength * numFaces;
    for (size_t mipLevel = 0; mipLevel < desc.mipmapLevelCount; ++mipLevel)
    {
        MTLRegion region =
            MTLRegionMake3D(0, 0, 0, std::max(size_t(1), size_t(desc.width >> mipLevel)),
                            std::max(size_t(1), size_t(desc.height >> mipLevel)),
                            std::max(size_t(1), size_t(desc.depth >> mipLevel)));
        size_t bytesPerRow     = region.size.width * bytesPerPixel;
        size_t bytesPer2DImage = bytesPerRow * region.size.height;
        for (uint32_t slice = 0; slice < numSlices; ++slice)
        {
            [blitCommandEncoder copyFromBuffer:buffer
                                  sourceOffset:0
                             sourceBytesPerRow:bytesPerRow
                           sourceBytesPerImage:bytesPer2DImage
                                    sourceSize:region.size
                                     toTexture:tex
                              destinationSlice:slice
                              destinationLevel:mipLevel
                             destinationOrigin:region.origin
                                       options:blitOption];
        }
    }

    // Encode a blit pass to copy data from the source buffer to the private texture.
    [blitCommandEncoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

void fillTextureWithNonZeroValues(const ContextDevice *contextDevice,
                                  const id<MTLTexture> &tex,
                                  const MTLTextureDescriptor *desc)
{
    if (desc.storageMode == MTLStorageModePrivate)
    {
        fillPrivateTextureWithNonZeroValues(contextDevice, tex, desc);
        return;
    }

    // Do NOT replace with call to Texture::replaceRegion!
    // The point of this code is to fill the texture with non-zero values
    // to test that other parts of the code correctly clear the texture.
    // As such it needs to try to not affect other state of the system.
    const size_t numFaces =
        (desc.textureType == MTLTextureTypeCube || desc.textureType == MTLTextureTypeCubeArray) ? 6
                                                                                                : 1;

    // allocate space for the largest mip
    const angle::FormatID formatID   = Format::MetalToAngleFormatID(desc.pixelFormat);
    const angle::Format &angleFormat = angle::Format::Get(formatID);

    size_t bytesPerPixel     = angleFormat.pixelBytes;
    size_t bytesPerRowLevel0 = desc.width * bytesPerPixel;
    size_t size              = bytesPerRowLevel0 * desc.height * desc.depth;
    std::vector<uint8_t> data(size, 0xBD);

    const size_t numSlices = desc.arrayLength * numFaces;
    for (size_t mipLevel = 0; mipLevel < desc.mipmapLevelCount; ++mipLevel)
    {
        MTLRegion region =
            MTLRegionMake3D(0, 0, 0, std::max(size_t(1), size_t(desc.width >> mipLevel)),
                            std::max(size_t(1), size_t(desc.height >> mipLevel)),
                            std::max(size_t(1), size_t(desc.depth >> mipLevel)));
        size_t bytesPerRow     = region.size.width * bytesPerPixel;
        size_t bytesPer2DImage = bytesPerRow * region.size.height;
        for (uint32_t slice = 0; slice < numSlices; ++slice)
        {
            [tex replaceRegion:region
                   mipmapLevel:mipLevel
                         slice:slice
                     withBytes:data.data()
                   bytesPerRow:bytesPerRow
                 bytesPerImage:bytesPer2DImage];
        }
    }
}

}  // namespace

ContextDevice::ContextDevice(GLint ownershipIdentity)
{
#if ANGLE_USE_METAL_OWNERSHIP_IDENTITY
    mOwnershipIdentity = static_cast<task_id_token_t>(ownershipIdentity);
    if (mOwnershipIdentity != TASK_ID_TOKEN_NULL)
    {
        kern_return_t kr =
            mach_port_mod_refs(mach_task_self(), mOwnershipIdentity, MACH_PORT_RIGHT_SEND, 1);
        if (ANGLE_UNLIKELY(kr != KERN_SUCCESS))
        {
            ERR() << "mach_port_mod_refs failed with: %s (%x)" << mach_error_string(kr) << kr;
            ASSERT(false);
        }
    }
#endif
}

ContextDevice::~ContextDevice()
{
#if ANGLE_USE_METAL_OWNERSHIP_IDENTITY
    if (mOwnershipIdentity != TASK_ID_TOKEN_NULL)
    {
        kern_return_t kr =
            mach_port_mod_refs(mach_task_self(), mOwnershipIdentity, MACH_PORT_RIGHT_SEND, -1);
        if (ANGLE_UNLIKELY(kr != KERN_SUCCESS))
        {
            ERR() << "mach_port_mod_refs failed with: %s (%x)" << mach_error_string(kr) << kr;
            ASSERT(false);
        }
    }
#endif
}

AutoObjCPtr<id<MTLSamplerState>> ContextDevice::newSamplerStateWithDescriptor(
    MTLSamplerDescriptor *descriptor) const
{
    return adoptObjCObj([get() newSamplerStateWithDescriptor:descriptor]);
}

AutoObjCPtr<id<MTLTexture>> ContextDevice::newTextureWithDescriptor(
    MTLTextureDescriptor *descriptor) const
{

    auto resource = adoptObjCObj([get() newTextureWithDescriptor:descriptor]);
    setOwnerWithIdentity(resource);
    fillTextureWithNonZeroValues(this, resource, descriptor);
    return resource;
}

AutoObjCPtr<id<MTLTexture>> ContextDevice::newTextureWithDescriptor(
    MTLTextureDescriptor *descriptor,
    IOSurfaceRef iosurface,
    NSUInteger plane) const
{
    auto resource = adoptObjCObj([get() newTextureWithDescriptor:descriptor
                                                       iosurface:iosurface
                                                           plane:plane]);
    fillTextureWithNonZeroValues(this, resource, descriptor);
    return resource;
}

AutoObjCPtr<id<MTLBuffer>> ContextDevice::newBufferWithLength(NSUInteger length,
                                                              MTLResourceOptions options) const
{
    auto resource = adoptObjCObj([get() newBufferWithLength:length options:options]);
    setOwnerWithIdentity(resource);
    return resource;
}

AutoObjCPtr<id<MTLBuffer>> ContextDevice::newBufferWithBytes(const void *pointer,
                                                             NSUInteger length,
                                                             MTLResourceOptions options) const
{
    auto resource = adoptObjCObj([get() newBufferWithBytes:pointer length:length options:options]);
    setOwnerWithIdentity(resource);
    return resource;
}

AutoObjCPtr<id<MTLComputePipelineState>> ContextDevice::newComputePipelineStateWithFunction(
    id<MTLFunction> computeFunction,
    __autoreleasing NSError **error) const
{
    return adoptObjCObj([get() newComputePipelineStateWithFunction:computeFunction error:error]);
}

AutoObjCPtr<id<MTLRenderPipelineState>> ContextDevice::newRenderPipelineStateWithDescriptor(
    MTLRenderPipelineDescriptor *descriptor,
    __autoreleasing NSError **error) const
{
    return adoptObjCObj([get() newRenderPipelineStateWithDescriptor:descriptor error:error]);
}

AutoObjCPtr<id<MTLLibrary>> ContextDevice::newLibraryWithSource(
    NSString *source,
    MTLCompileOptions *options,
    __autoreleasing NSError **error) const
{
    return adoptObjCObj([get() newLibraryWithSource:source options:options error:error]);
}

AutoObjCPtr<id<MTLDepthStencilState>> ContextDevice::newDepthStencilStateWithDescriptor(
    MTLDepthStencilDescriptor *descriptor) const
{
    return adoptObjCObj([get() newDepthStencilStateWithDescriptor:descriptor]);
}

AutoObjCPtr<id<MTLSharedEvent>> ContextDevice::newSharedEvent() const
{
    return adoptObjCObj([get() newSharedEvent]);
}

void ContextDevice::setOwnerWithIdentity(id<MTLResource> resource) const
{
#if ANGLE_USE_METAL_OWNERSHIP_IDENTITY
    mtl::setOwnerWithIdentity(resource, mOwnershipIdentity);
#endif
}
}
}
