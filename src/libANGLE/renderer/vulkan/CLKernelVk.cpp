//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelVk.cpp: Implements the class methods for CLKernelVk.

#include "libANGLE/renderer/vulkan/CLKernelVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLDeviceVk.h"
#include "libANGLE/renderer/vulkan/CLProgramVk.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLKernelVk::CLKernelVk(const cl::Kernel &kernel,
                       std::string &name,
                       std::string &attributes,
                       CLKernelArguments &args)
    : CLKernelImpl(kernel),
      mProgram(&kernel.getProgram().getImpl<CLProgramVk>()),
      mContext(&kernel.getProgram().getContext().getImpl<CLContextVk>()),
      mName(name),
      mAttributes(attributes),
      mArgs(args)
{
    mShaderProgramHelper.setShader(gl::ShaderType::Compute,
                                   mKernel.getProgram().getImpl<CLProgramVk>().getShaderModule());
}

CLKernelVk::~CLKernelVk()
{
    for (vk::BindingPointer<vk::DescriptorSetLayout, vk::AtomicRefCounted<vk::DescriptorSetLayout>>
             &dsLayouts : mDescriptorSetLayouts)
    {
        dsLayouts.reset();
    }

    mPipelineLayout.reset();
    for (auto &pipelineHelper : mComputePipelineCache)
    {
        pipelineHelper.destroy(mContext->getDevice());
    }
    mShaderProgramHelper.destroy(mContext->getRenderer());
}

angle::Result CLKernelVk::setArg(cl_uint argIndex, size_t argSize, const void *argValue)
{
    auto &arg = mArgs.at(argIndex);
    if (arg.used)
    {
        arg.handle     = const_cast<void *>(argValue);
        arg.handleSize = argSize;
    }

    return angle::Result::Continue;
}

angle::Result CLKernelVk::createInfo(CLKernelImpl::Info *info) const
{
    info->functionName = mName;
    info->attributes   = mAttributes;
    info->numArgs      = static_cast<cl_uint>(mArgs.size());
    for (const auto &arg : mArgs)
    {
        ArgInfo argInfo;
        argInfo.name             = arg.info.name;
        argInfo.typeName         = arg.info.typeName;
        argInfo.accessQualifier  = arg.info.accessQualifier;
        argInfo.addressQualifier = arg.info.addressQualifier;
        argInfo.typeQualifier    = arg.info.typeQualifier;
        info->args.push_back(std::move(argInfo));
    }

    auto &ctx = mKernel.getProgram().getContext();
    info->workGroups.resize(ctx.getDevices().size());
    const CLProgramVk::DeviceProgramData *deviceProgramData = nullptr;
    for (auto i = 0u; i < ctx.getDevices().size(); ++i)
    {
        auto &workGroup     = info->workGroups[i];
        const auto deviceVk = &ctx.getDevices()[i]->getImpl<CLDeviceVk>();
        deviceProgramData   = mProgram->getDeviceProgramData(ctx.getDevices()[i]->getNative());
        if (deviceProgramData == nullptr)
        {
            continue;
        }

        // TODO: http://anglebug.com/8576
        ANGLE_TRY(
            deviceVk->getInfoSizeT(cl::DeviceInfo::MaxWorkGroupSize, &workGroup.workGroupSize));

        // TODO: http://anglebug.com/8575
        workGroup.privateMemSize = 0;
        workGroup.localMemSize   = 0;

        workGroup.prefWorkGroupSizeMultiple = 16u;
        workGroup.globalWorkSize            = {0, 0, 0};
        if (deviceProgramData->reflectionData.kernelCompileWGS.contains(mName))
        {
            workGroup.compileWorkGroupSize = {
                deviceProgramData->reflectionData.kernelCompileWGS.at(mName)[0],
                deviceProgramData->reflectionData.kernelCompileWGS.at(mName)[1],
                deviceProgramData->reflectionData.kernelCompileWGS.at(mName)[2]};
        }
        else
        {
            workGroup.compileWorkGroupSize = {0, 0, 0};
        }
    }

    return angle::Result::Continue;
}

angle::Result CLKernelVk::getOrCreateComputePipeline(vk::PipelineCacheAccess *pipelineCache,
                                                     cl::NDRange &ndrange,
                                                     const cl::Device &device,
                                                     vk::PipelineHelper **pipelineOut)
{

    uint32_t constantDataOffset = 0;
    std::vector<uint32_t> specConstantData;
    std::vector<VkSpecializationMapEntry> mapEntries;
    const CLProgramVk::DeviceProgramData *devProgramData =
        getProgram()->getDeviceProgramData(device.getNative());
    ASSERT(devProgramData != nullptr);

    cl::CompiledWorkgroupSize compiledWorkgroupSize =
        devProgramData->getCompiledWGS(getKernelName());

    // Configure the workgroup size (WGS) for this compute shader
    if (compiledWorkgroupSize != std::array<uint32_t, 3>{0, 0, 0})
    {
        // We use "reqd_work_group_size" kernel attribute for WGS here. Frontend validation should
        // have already ensured that this value matches user-passed LWS.
        ASSERT(ndrange.lws == compiledWorkgroupSize);
        ndrange.lws = compiledWorkgroupSize;
    }
    else
    {
        if (ndrange.nullLocalWorkSize)
        {
            // NULL value was passed, in which case the OpenCL implementation will determine
            // how to be break the global work-items into appropriate work-group instances.
            ndrange.lws = device.getImpl<CLDeviceVk>().selectWorkGroupSize(ndrange);
        }

        // If at least one of the kernels does not use the reqd_work_group_size attribute, the
        // Vulkan SPIR-V produced by the compiler will contain specialization constants
        std::array<uint32_t, 3> specConstantWorkgroupSizeIDs =
            devProgramData->reflectionData.specConstantWorkgroupSizeIDs;
        for (cl_uint i = 0; i < ndrange.workDimensions; ++i)
        {
            mapEntries.push_back(
                VkSpecializationMapEntry{.constantID = specConstantWorkgroupSizeIDs.at(i),
                                         .offset     = constantDataOffset,
                                         .size       = sizeof(uint32_t)});
            specConstantData.push_back(ndrange.lws[i]);
            constantDataOffset += sizeof(uint32_t);
        }
    }

    // Now get or create (on compute pipeline cache miss) compute pipeline and return it
    VkSpecializationInfo computeSpecializationInfo{
        .mapEntryCount = (uint32_t)mapEntries.size(),
        .pMapEntries   = mapEntries.data(),
        .dataSize      = specConstantData.size() * sizeof(uint32_t),
        .pData         = specConstantData.data(),
    };
    return mShaderProgramHelper.getOrCreateComputePipeline(
        mContext, &mComputePipelineCache, pipelineCache, getPipelineLayout().get(),
        vk::ComputePipelineFlags{}, PipelineSource::Draw, pipelineOut, mName.c_str(),
        &computeSpecializationInfo);
}

}  // namespace rx
