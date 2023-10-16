//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelVk.cpp: Implements the class methods for CLKernelVk.

#include "libANGLE/renderer/vulkan/CLKernelVk.h"

namespace rx
{

CLKernelVk::CLKernelVk(const cl::Kernel &kernel) : CLKernelImpl(kernel) {}

CLKernelVk::~CLKernelVk() = default;

cl_int CLKernelVk::setArg(cl_uint argIndex, size_t argSize, const void *argValue)
{
    UNIMPLEMENTED();
    return CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
}

CLKernelImpl::Info CLKernelVk::createInfo(cl_int &errorCode) const
{
    UNIMPLEMENTED();
    errorCode = CL_OUT_OF_RESOURCES;  // TODO(annestrand) Placeholder error for now
    return CLKernelImpl::Info{};
}

}  // namespace rx
