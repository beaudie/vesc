//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueVk.cpp: Implements the class methods for CLCommandQueueVk.

#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"

namespace rx
{

CLCommandQueueVk::CLCommandQueueVk(const cl::CommandQueue &commandQueue)
    : CLCommandQueueImpl(commandQueue)
{}

CLCommandQueueVk::~CLCommandQueueVk() = default;

cl_int CLCommandQueueVk::setProperty(cl::CommandQueueProperties properties, cl_bool enable)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueReadBuffer(const cl::Buffer &buffer,
                                           bool blocking,
                                           size_t offset,
                                           size_t size,
                                           void *ptr,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueWriteBuffer(const cl::Buffer &buffer,
                                            bool blocking,
                                            size_t offset,
                                            size_t size,
                                            const void *ptr,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueReadBufferRect(const cl::Buffer &buffer,
                                               bool blocking,
                                               const size_t bufferOrigin[3],
                                               const size_t hostOrigin[3],
                                               const size_t region[3],
                                               size_t bufferRowPitch,
                                               size_t bufferSlicePitch,
                                               size_t hostRowPitch,
                                               size_t hostSlicePitch,
                                               void *ptr,
                                               const cl::EventPtrs &waitEvents,
                                               CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueWriteBufferRect(const cl::Buffer &buffer,
                                                bool blocking,
                                                const size_t bufferOrigin[3],
                                                const size_t hostOrigin[3],
                                                const size_t region[3],
                                                size_t bufferRowPitch,
                                                size_t bufferSlicePitch,
                                                size_t hostRowPitch,
                                                size_t hostSlicePitch,
                                                const void *ptr,
                                                const cl::EventPtrs &waitEvents,
                                                CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                           const cl::Buffer &dstBuffer,
                                           size_t srcOffset,
                                           size_t dstOffset,
                                           size_t size,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
                                               const cl::Buffer &dstBuffer,
                                               const size_t srcOrigin[3],
                                               const size_t dstOrigin[3],
                                               const size_t region[3],
                                               size_t srcRowPitch,
                                               size_t srcSlicePitch,
                                               size_t dstRowPitch,
                                               size_t dstSlicePitch,
                                               const cl::EventPtrs &waitEvents,
                                               CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueFillBuffer(const cl::Buffer &buffer,
                                           const void *pattern,
                                           size_t patternSize,
                                           size_t offset,
                                           size_t size,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

void *CLCommandQueueVk::enqueueMapBuffer(const cl::Buffer &buffer,
                                         bool blocking,
                                         cl::MapFlags mapFlags,
                                         size_t offset,
                                         size_t size,
                                         const cl::EventPtrs &waitEvents,
                                         CLEventImpl::CreateFunc *eventCreateFunc,
                                         cl_int &errorCode)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    errorCode = CL_OUT_OF_RESOURCES;
    return nullptr;
}

cl_int CLCommandQueueVk::enqueueReadImage(const cl::Image &image,
                                          bool blocking,
                                          const size_t origin[3],
                                          const size_t region[3],
                                          size_t rowPitch,
                                          size_t slicePitch,
                                          void *ptr,
                                          const cl::EventPtrs &waitEvents,
                                          CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueWriteImage(const cl::Image &image,
                                           bool blocking,
                                           const size_t origin[3],
                                           const size_t region[3],
                                           size_t inputRowPitch,
                                           size_t inputSlicePitch,
                                           const void *ptr,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueCopyImage(const cl::Image &srcImage,
                                          const cl::Image &dstImage,
                                          const size_t srcOrigin[3],
                                          const size_t dstOrigin[3],
                                          const size_t region[3],
                                          const cl::EventPtrs &waitEvents,
                                          CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueFillImage(const cl::Image &image,
                                          const void *fillColor,
                                          const size_t origin[3],
                                          const size_t region[3],
                                          const cl::EventPtrs &waitEvents,
                                          CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueCopyImageToBuffer(const cl::Image &srcImage,
                                                  const cl::Buffer &dstBuffer,
                                                  const size_t srcOrigin[3],
                                                  const size_t region[3],
                                                  size_t dstOffset,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueCopyBufferToImage(const cl::Buffer &srcBuffer,
                                                  const cl::Image &dstImage,
                                                  size_t srcOffset,
                                                  const size_t dstOrigin[3],
                                                  const size_t region[3],
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

void *CLCommandQueueVk::enqueueMapImage(const cl::Image &image,
                                        bool blocking,
                                        cl::MapFlags mapFlags,
                                        const size_t origin[3],
                                        const size_t region[3],
                                        size_t *imageRowPitch,
                                        size_t *imageSlicePitch,
                                        const cl::EventPtrs &waitEvents,
                                        CLEventImpl::CreateFunc *eventCreateFunc,
                                        cl_int &errorCode)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    errorCode = CL_OUT_OF_RESOURCES;
    return nullptr;
}

cl_int CLCommandQueueVk::enqueueUnmapMemObject(const cl::Memory &memory,
                                               void *mappedPtr,
                                               const cl::EventPtrs &waitEvents,
                                               CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueMigrateMemObjects(const cl::MemoryPtrs &memObjects,
                                                  cl::MemMigrationFlags flags,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueNDRangeKernel(const cl::Kernel &kernel,
                                              cl_uint workDim,
                                              const size_t *globalWorkOffset,
                                              const size_t *globalWorkSize,
                                              const size_t *localWorkSize,
                                              const cl::EventPtrs &waitEvents,
                                              CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueTask(const cl::Kernel &kernel,
                                     const cl::EventPtrs &waitEvents,
                                     CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueNativeKernel(cl::UserFunc userFunc,
                                             void *args,
                                             size_t cbArgs,
                                             const cl::BufferPtrs &buffers,
                                             const std::vector<size_t> bufferPtrOffsets,
                                             const cl::EventPtrs &waitEvents,
                                             CLEventImpl::CreateFunc *eventCreateFunc)
{
    WARN() << "clEnqueueNativeKernel is not supported";
    return CL_INVALID_OPERATION;
}

cl_int CLCommandQueueVk::enqueueMarkerWithWaitList(const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueMarker(CLEventImpl::CreateFunc &eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueWaitForEvents(const cl::EventPtrs &events)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueBarrierWithWaitList(const cl::EventPtrs &waitEvents,
                                                    CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::enqueueBarrier()
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::flush()
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLCommandQueueVk::finish()
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

}  // namespace rx
