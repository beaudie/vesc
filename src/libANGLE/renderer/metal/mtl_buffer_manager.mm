//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_buffer_manager.mm:
//    Implements the class methods for BufferManager.
//

#include "libANGLE/renderer/metal/mtl_buffer_manager.h"

#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

namespace rx
{

namespace mtl
{

namespace
{

constexpr size_t Log2(size_t bytes)
{
    return bytes == 1 ? 0 : (1 + Log2(bytes / 2));
}

const char *memUnitSuffix(size_t powerOf2)
{
    if (powerOf2 < 10)
    {
        return "b";
    }
    if (powerOf2 < 20)
    {
        return "k";
    }
    if (powerOf2 < 30)
    {
        return "M";
    }
    return "G";
}

size_t memUnitValue(size_t powerOf2)
{
    if (powerOf2 < 10)
    {
        return 1u << powerOf2;
    }
    if (powerOf2 < 20)
    {
        return 1u << (powerOf2 - 10);
    }
    if (powerOf2 < 30)
    {
        return 1u << (powerOf2 - 20);
    }
    return 1u << (powerOf2 - 30);
}

}  // namespace

BufferManager::BufferManager()
#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
    : mAllocations(kMaxSizePowerOf2, 0)
#endif
{}

angle::Result BufferManager::getFreeSerialBufferRef(ContextMtl *contextMtl, BufferRef &bufferRef)
{
    return getBuffer(contextMtl, kBufferSize, bufferRef);
}

void BufferManager::freeUnusedBuffers(ContextMtl *contextMtl)
{
    // Scan for the first buffer still in use.
    BufferList::iterator firstInUseIter =
        std::find_if(mInUseBuffers.begin(), mInUseBuffers.end(),
                     [&contextMtl](auto ref) { return ref->isBeingUsedByGPU(contextMtl); });

    // Move unused buffers to the free lists
    for (BufferList::iterator it = mInUseBuffers.begin(); it != firstInUseIter; ++it)
    {
        addBufferRefToFreeLists(*it);
    }
    mInUseBuffers.erase(mInUseBuffers.begin(), firstInUseIter);
}

void BufferManager::addBufferRefToFreeLists(mtl::BufferRef &bufferRef)
{
    const size_t bucketNdx = Log2(bufferRef->size());
    ASSERT(bucketNdx < kMaxSizePowerOf2);
    mFreeBuffers[bucketNdx].push_back(bufferRef);
}

void BufferManager::returnBuffer(ContextMtl *contextMtl, BufferRef &bufferRef)
{
    if (bufferRef->isBeingUsedByGPU(contextMtl))
    {
        mInUseBuffers.push_back(bufferRef);
    }
    else
    {
        addBufferRefToFreeLists(bufferRef);
    }
}

angle::Result BufferManager::getBuffer(ContextMtl *contextMtl, size_t size, BufferRef &bufferRef)
{
    freeUnusedBuffers(contextMtl);

    const size_t bucketNdx  = Log2(size);
    BufferList &freeBuffers = mFreeBuffers[bucketNdx];

    // If there are free buffers grab one
    if (!freeBuffers.empty())
    {
        bufferRef = freeBuffers.back();
        freeBuffers.pop_back();
        return angle::Result::Continue;
    }

    // Create a new one
    mtl::BufferRef newBufferRef;

    size_t allocSize = size_t(1) << bucketNdx;
    ANGLE_TRY(mtl::Buffer::MakeBufferWithSharedMemOpt(contextMtl, true, allocSize, nullptr,
                                                      &newBufferRef));

#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
    {
        mTotalMem += allocSize;
        mAllocations[bucketNdx]++;
        fprintf(stderr, "totalMem: %zu, ", mTotalMem);
        size_t numBuffers = 0;
        for (size_t i = 0; i < kMaxSizePowerOf2; ++i)
        {
            if (mAllocations[i])
            {
                numBuffers += mAllocations[i];
                fprintf(stderr, "%zu%s: %zu, ", memUnitValue(i), memUnitSuffix(i), mAllocations[i]);
            }
        }
        fprintf(stderr, " total: %zu\n", numBuffers);
    }
#endif

    bufferRef = newBufferRef;

    return angle::Result::Continue;
}

angle::Result BufferManager::queueBlitCopyDataToBuffer(ContextMtl *contextMtl,
                                                       const void *srcPtr,
                                                       size_t sizeToCopy,
                                                       size_t offset,
                                                       mtl::BufferRef &dstMetalBuffer)
{
    const uint8 *src = reinterpret_cast<const uint8 *>(srcPtr);
    for (size_t srcOffset = 0; srcOffset < sizeToCopy; srcOffset += kBufferSize)
    {
        mtl::BufferRef bufferRef;
        ANGLE_TRY(getBuffer(contextMtl, kBufferSize, bufferRef));

        // copy data to buffer
        uint8_t *ptr         = bufferRef->mapWithOpt(contextMtl, false, true);
        size_t subSizeToCopy = std::min(kBufferSize, sizeToCopy - srcOffset);
        std::copy(src + srcOffset, src + srcOffset + subSizeToCopy, ptr);
        bufferRef->unmapAndFlushSubset(contextMtl, offset, sizeToCopy);

        // queue blit
        mtl::BlitCommandEncoder *blitEncoder = contextMtl->getBlitCommandEncoder();
        blitEncoder->copyBuffer(bufferRef, 0, dstMetalBuffer, offset + srcOffset, subSizeToCopy);

        returnBuffer(contextMtl, bufferRef);
    }
    return angle::Result::Continue;
}

}  // namespace mtl
}  // namespace rx
