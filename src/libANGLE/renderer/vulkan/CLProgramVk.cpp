//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramVk.cpp: Implements the class methods for CLProgramVk.

#include "libANGLE/renderer/vulkan/CLProgramVk.h"

namespace rx
{

CLProgramVk::CLProgramVk(const cl::Program &program) : CLProgramImpl(program) {}

CLProgramVk::~CLProgramVk() = default;

cl_int CLProgramVk::build(const cl::DevicePtrs &devices, const char *options, cl::Program *notify)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLProgramVk::compile(const cl::DevicePtrs &devices,
                            const char *options,
                            const cl::ProgramPtrs &inputHeaders,
                            const char **headerIncludeNames,
                            cl::Program *notify)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLProgramVk::getInfo(cl::ProgramInfo name,
                            size_t valueSize,
                            void *value,
                            size_t *valueSizeRet) const
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

cl_int CLProgramVk::getBuildInfo(const cl::Device &device,
                                 cl::ProgramBuildInfo name,
                                 size_t valueSize,
                                 void *value,
                                 size_t *valueSizeRet) const
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

CLKernelImpl::Ptr CLProgramVk::createKernel(const cl::Kernel &kernel,
                                            const char *name,
                                            cl_int &errorCode)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    errorCode = CL_OUT_OF_RESOURCES;
    return CLKernelImpl::Ptr{};
}

cl_int CLProgramVk::createKernels(cl_uint numKernels,
                                  CLKernelImpl::CreateFuncs &createFuncs,
                                  cl_uint *numKernelsRet)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    return CL_OUT_OF_RESOURCES;
}

}  // namespace rx
