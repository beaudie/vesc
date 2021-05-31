//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueCL.cpp: Implements the class methods for CLCommandQueueCL.

#include "libANGLE/renderer/cl/CLCommandQueueCL.h"

#include "libANGLE/renderer/cl/CLEventCL.h"
#include "libANGLE/renderer/cl/CLMemoryCL.h"

#include "libANGLE/CLBuffer.h"

namespace rx
{

CLCommandQueueCL::CLCommandQueueCL(const cl::CommandQueue &commandQueue, cl_command_queue native)
    : CLCommandQueueImpl(commandQueue), mNative(native)
{}

CLCommandQueueCL::~CLCommandQueueCL()
{
    if (mNative->getDispatch().clReleaseCommandQueue(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL command-queue";
    }
}

cl_int CLCommandQueueCL::setProperty(cl::CommandQueueProperties properties, cl_bool enable)
{
    return mNative->getDispatch().clSetCommandQueueProperty(mNative, properties.get(), enable,
                                                            nullptr);
}

cl_int CLCommandQueueCL::enqueueReadBuffer(const cl::Buffer &buffer,
                                           bool blocking,
                                           size_t offset,
                                           size_t size,
                                           void *ptr,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueReadBuffer(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), blocking ? CL_TRUE : CL_FALSE, offset,
        size, ptr, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueWriteBuffer(const cl::Buffer &buffer,
                                            bool blocking,
                                            size_t offset,
                                            size_t size,
                                            const void *ptr,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueWriteBuffer(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), blocking ? CL_TRUE : CL_FALSE, offset,
        size, ptr, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueReadBufferRect(const cl::Buffer &buffer,
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
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueReadBufferRect(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), blocking ? CL_TRUE : CL_FALSE,
        bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch,
        hostSlicePitch, ptr, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueWriteBufferRect(const cl::Buffer &buffer,
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
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueWriteBufferRect(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), blocking ? CL_TRUE : CL_FALSE,
        bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch,
        hostSlicePitch, ptr, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                           const cl::Buffer &dstBuffer,
                                           size_t srcOffset,
                                           size_t dstOffset,
                                           size_t size,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueCopyBuffer(
        mNative, srcBuffer.getImpl<CLMemoryCL>().getNative(),
        dstBuffer.getImpl<CLMemoryCL>().getNative(), srcOffset, dstOffset, size,
        static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
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
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueCopyBufferRect(
        mNative, srcBuffer.getImpl<CLMemoryCL>().getNative(),
        dstBuffer.getImpl<CLMemoryCL>().getNative(), srcOrigin, dstOrigin, region, srcRowPitch,
        srcSlicePitch, dstRowPitch, dstSlicePitch, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

cl_int CLCommandQueueCL::enqueueFillBuffer(const cl::Buffer &buffer,
                                           const void *pattern,
                                           size_t patternSize,
                                           size_t offset,
                                           size_t size,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    const cl_int errorCode = mNative->getDispatch().clEnqueueFillBuffer(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), pattern, patternSize, offset, size,
        static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return errorCode;
}

void *CLCommandQueueCL::enqueueMapBuffer(const cl::Buffer &buffer,
                                         bool blocking,
                                         cl::MapFlags mapFlags,
                                         size_t offset,
                                         size_t size,
                                         const cl::EventPtrs &waitEvents,
                                         CLEventImpl::CreateFunc *eventCreateFunc,
                                         cl_int &errorCode)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    cl_event nativeEvent                     = nullptr;

    void *const region = mNative->getDispatch().clEnqueueMapBuffer(
        mNative, buffer.getImpl<CLMemoryCL>().getNative(), blocking ? CL_TRUE : CL_FALSE,
        mapFlags.get(), offset, size, static_cast<cl_uint>(nativeEvents.size()),
        nativeEvents.empty() ? nullptr : nativeEvents.data(),
        eventCreateFunc != nullptr ? &nativeEvent : nullptr, &errorCode);

    if (errorCode == CL_SUCCESS && eventCreateFunc != nullptr)
    {
        *eventCreateFunc = [=](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
    return region;
}

}  // namespace rx
