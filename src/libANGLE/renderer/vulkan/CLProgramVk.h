//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramVk.h: Defines the class interface for CLProgramVk, implementing CLProgramImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_

#include "libANGLE/renderer/vulkan/cl_types.h"

#include "libANGLE/renderer/CLProgramImpl.h"

namespace rx
{

class CLProgramVk : public CLProgramImpl
{
  public:
    CLProgramVk(const cl::Program &program);
    ~CLProgramVk() override;

    // getSource override here is for returning CL source from: [ IL, Binary, BuiltinKernels ]
    std::string getSource(cl_int &errorCode) const override { return std::string{}; }

    cl_int build(const cl::DevicePtrs &devices, const char *options, cl::Program *notify) override;

    cl_int compile(const cl::DevicePtrs &devices,
                   const char *options,
                   const cl::ProgramPtrs &inputHeaders,
                   const char **headerIncludeNames,
                   cl::Program *notify) override;

    cl_int getInfo(cl::ProgramInfo name,
                   size_t valueSize,
                   void *value,
                   size_t *valueSizeRet) const override;

    cl_int getBuildInfo(const cl::Device &device,
                        cl::ProgramBuildInfo name,
                        size_t valueSize,
                        void *value,
                        size_t *valueSizeRet) const override;

    CLKernelImpl::Ptr createKernel(const cl::Kernel &kernel,
                                   const char *name,
                                   cl_int &errorCode) override;

    cl_int createKernels(cl_uint numKernels,
                         CLKernelImpl::CreateFuncs &createFuncs,
                         cl_uint *numKernelsRet) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLPROGRAMVK_H_
