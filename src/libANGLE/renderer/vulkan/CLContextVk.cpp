//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextVk.cpp: Implements the class methods for CLContextVk.

#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"
#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLContextVk::CLContextVk(const cl::Context &context,
                         const egl::Display *display,
                         const cl::DevicePtrs devicePtrs)
    : CLContextImpl(context),
      vk::Context(GetImplAs<DisplayVk>(display)->getRenderer()),
      mAssociatedDevices(devicePtrs)
{}

CLContextVk::~CLContextVk() = default;

void CLContextVk::handleError(VkResult errorCode,
                              const char *file,
                              const char *function,
                              unsigned int line)
{
    ASSERT(errorCode != VK_SUCCESS);

    CLenum clErrorCode = CL_SUCCESS;
    switch (errorCode)
    {
        case VK_ERROR_TOO_MANY_OBJECTS:
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            clErrorCode = CL_OUT_OF_HOST_MEMORY;
            break;
        default:
            clErrorCode = CL_INVALID_OPERATION;
    }
    ERR() << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode)
          << ". " << "CL error (" << clErrorCode << ")";

    if (errorCode == VK_ERROR_DEVICE_LOST)
    {
        handleDeviceLost();
    }
    ANGLE_CL_SET_ERROR(clErrorCode);
}

void CLContextVk::handleDeviceLost() const
{
    // For now just notify the renderer
    getRenderer()->notifyDeviceLost();
}

angle::Result CLContextVk::getDevices(cl::DevicePtrs *devicePtrsOut) const
{
    ASSERT(!mAssociatedDevices.empty());
    *devicePtrsOut = mAssociatedDevices;
    return angle::Result::Continue;
}

angle::Result CLContextVk::createCommandQueue(const cl::CommandQueue &commandQueue,
                                              CLCommandQueueImpl::Ptr *commandQueueOut)
{
    *commandQueueOut = CLCommandQueueVk::Ptr(new (std::nothrow) CLCommandQueueVk(commandQueue));
    if (*commandQueueOut == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    return angle::Result::Continue;
}

angle::Result CLContextVk::createBuffer(const cl::Buffer &buffer,
                                        size_t size,
                                        void *hostPtr,
                                        CLMemoryImpl::Ptr *bufferOut)
{
    auto memory = new (std::nothrow) CLBufferVk(buffer);
    if (memory == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(memory->create(size));
    *bufferOut = CLMemoryImpl::Ptr(memory);
    mData->mMemories.emplace(buffer.getNative());
    return angle::Result::Continue;
}

angle::Result CLContextVk::createImage(const cl::Image &image,
                                       cl::MemFlags flags,
                                       const cl_image_format &format,
                                       const cl::ImageDescriptor &desc,
                                       void *hostPtr,
                                       CLMemoryImpl::Ptr *imageOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::getSupportedImageFormats(cl::MemFlags flags,
                                                    cl::MemObjectType imageType,
                                                    cl_uint numEntries,
                                                    cl_image_format *imageFormats,
                                                    cl_uint *numImageFormats)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::createSampler(const cl::Sampler &sampler, CLSamplerImpl::Ptr *samplerOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::createProgramWithSource(const cl::Program &program,
                                                   const std::string &source,
                                                   CLProgramImpl::Ptr *programOut)
{
    CLProgramVk *programVk = new (std::nothrow) CLProgramVk(program);
    if (programVk == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(programVk->init());
    *programOut = CLProgramImpl::Ptr(std::move(programVk));

    return angle::Result::Continue;
}

angle::Result CLContextVk::createProgramWithIL(const cl::Program &program,
                                               const void *il,
                                               size_t length,
                                               CLProgramImpl::Ptr *programOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::createProgramWithBinary(const cl::Program &program,
                                                   const size_t *lengths,
                                                   const unsigned char **binaries,
                                                   cl_int *binaryStatus,
                                                   CLProgramImpl::Ptr *programOut)
{
    CLProgramVk *programVk = new (std::nothrow) CLProgramVk(program);
    if (programVk == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    ANGLE_TRY(programVk->init(lengths, binaries, binaryStatus));
    *programOut = CLProgramImpl::Ptr(std::move(programVk));

    return angle::Result::Continue;
}

angle::Result CLContextVk::createProgramWithBuiltInKernels(const cl::Program &program,
                                                           const char *kernel_names,
                                                           CLProgramImpl::Ptr *programOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::linkProgram(const cl::Program &program,
                                       const cl::DevicePtrs &devices,
                                       const char *options,
                                       const cl::ProgramPtrs &inputPrograms,
                                       cl::Program *notify,
                                       CLProgramImpl::Ptr *programOut)
{
    const cl::DevicePtrs &devicePtrs = !devices.empty() ? devices : mContext.getDevices();

    CLProgramVk::Ptr programImpl = CLProgramVk::Ptr(new (std::nothrow) CLProgramVk(program));
    if (programImpl == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    // We need to check to make sure that all or none of the program objects for each device contain
    // a valid binary.
    cl::DevicePtrs devicesWithAllBins;
    CLProgramVk::DeviceProgramDatas linkProgramDatas;
    for (const auto &devicePtr : devicePtrs)
    {
        bool foundBinary = false;
        for (const auto &inputProgram : inputPrograms)
        {
            auto deviceProgramData =
                inputProgram->getImpl<CLProgramVk>().getDeviceProgramData(devicePtr->getNative());
            if (deviceProgramData && !deviceProgramData->IR.empty())
            {
                // We only need one set of program binaries to link for all associated devices
                if (devicesWithAllBins.empty())
                {
                    linkProgramDatas.push_back(deviceProgramData);
                }
                foundBinary = true;
            }
            else
            {
                if (foundBinary)
                {
                    ANGLE_CL_RETURN_ERROR(CL_INVALID_OPERATION);
                }
            }
        }
        if (foundBinary)
        {
            devicesWithAllBins.push_back(devicePtr);
        }
    }

    // Also check to see if we have at least one device with all program binaries.
    if (devicesWithAllBins.empty())
    {
        ANGLE_CL_RETURN_ERROR(CL_INVALID_OPERATION);
    }

    // Perform link
    if (notify)
    {
        std::shared_ptr<angle::WaitableEvent> asyncEvent =
            mContext.getPlatform().getMultiThreadPool()->postWorkerTask(
                std::make_shared<CLAsyncBuildTask>(
                    programImpl.get(), devicesWithAllBins, std::string(options ? options : ""), "",
                    CLProgramVk::BuildType::COMPILE, linkProgramDatas, notify));
        ASSERT(asyncEvent != nullptr);
    }
    else
    {
        if (!programImpl->buildInternal(devicesWithAllBins, std::string(options ? options : ""), "",
                                        CLProgramVk::BuildType::LINK, linkProgramDatas))
        {
            ANGLE_CL_RETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
        }
    }

    *programOut = std::move(programImpl);
    return angle::Result::Continue;
}

angle::Result CLContextVk::createUserEvent(const cl::Event &event, CLEventImpl::Ptr *eventOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLContextVk::waitForEvents(const cl::EventPtrs &events)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

}  // namespace rx
