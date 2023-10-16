//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextVk.cpp: Implements the class methods for CLContextVk.

#include "libANGLE/renderer/vulkan/CLContextVk.h"

namespace rx
{

CLContextVk::CLContextVk(const cl::Context &context) : CLContextImpl(context) {}

CLContextVk::~CLContextVk() = default;

cl::DevicePtrs CLContextVk::getDevices(cl_int &errorCode) const
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return cl::DevicePtrs{};
}

CLCommandQueueImpl::Ptr CLContextVk::createCommandQueue(const cl::CommandQueue &commandQueue,
                                                        cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLCommandQueueImpl::Ptr{};
}

CLMemoryImpl::Ptr CLContextVk::createBuffer(const cl::Buffer &buffer,
                                            size_t size,
                                            void *hostPtr,
                                            cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLMemoryImpl::Ptr{};
}

CLMemoryImpl::Ptr CLContextVk::createImage(const cl::Image &image,
                                           cl::MemFlags flags,
                                           const cl_image_format &format,
                                           const cl::ImageDescriptor &desc,
                                           void *hostPtr,
                                           cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLMemoryImpl::Ptr{};
}

cl_int CLContextVk::getSupportedImageFormats(cl::MemFlags flags,
                                             cl::MemObjectType imageType,
                                             cl_uint numEntries,
                                             cl_image_format *imageFormats,
                                             cl_uint *numImageFormats)
{
    UNIMPLEMENTED();
    return CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
}

CLSamplerImpl::Ptr CLContextVk::createSampler(const cl::Sampler &sampler, cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLSamplerImpl::Ptr{};
}

CLProgramImpl::Ptr CLContextVk::createProgramWithSource(const cl::Program &program,
                                                        const std::string &source,
                                                        cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLProgramImpl::Ptr{};
}

CLProgramImpl::Ptr CLContextVk::createProgramWithIL(const cl::Program &program,
                                                    const void *il,
                                                    size_t length,
                                                    cl_int &errorCode)
{
    // We currently do not support creating programs from IL
    errorCode = CL_INVALID_VALUE;
    return CLProgramImpl::Ptr{};
}

CLProgramImpl::Ptr CLContextVk::createProgramWithBinary(const cl::Program &program,
                                                        const size_t *lengths,
                                                        const unsigned char **binaries,
                                                        cl_int *binaryStatus,
                                                        cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLProgramImpl::Ptr{};
}

CLProgramImpl::Ptr CLContextVk::createProgramWithBuiltInKernels(const cl::Program &program,
                                                                const char *kernel_names,
                                                                cl_int &errorCode)
{
    // We currently do not support builtin kernels
    errorCode = CL_INVALID_VALUE;
    return CLProgramImpl::Ptr{};
}

CLProgramImpl::Ptr CLContextVk::linkProgram(const cl::Program &program,
                                            const cl::DevicePtrs &devices,
                                            const char *options,
                                            const cl::ProgramPtrs &inputPrograms,
                                            cl::Program *notify,
                                            cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLProgramImpl::Ptr{};
}

CLEventImpl::Ptr CLContextVk::createUserEvent(const cl::Event &event, cl_int &errorCode)
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLEventImpl::Ptr{};
}

cl_int CLContextVk::waitForEvents(const cl::EventPtrs &events)
{
    UNIMPLEMENTED();
    return CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
}

}  // namespace rx
