//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramVk.cpp: Implements the class methods for CLProgramVk.

#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLProgramVk::CLProgramVk(const cl::Program &program)
    : CLProgramImpl(program), mContext(&program.getContext().getImpl<CLContextVk>())
{
    cl::DevicePtrs devices;
    ANGLE_CL_IMPL_TRY(mContext->getDevices(&devices));

    // The devices associated with the program object are the devices associated with context
    for (const auto &dev : devices)
    {
        mAssociatedDevicePrograms[dev->getNative()] = DeviceProgramData{};
    }
}

CLProgramVk::CLProgramVk(const cl::Program &program,
                         const size_t *lengths,
                         const unsigned char **binaries,
                         cl_int *binaryStatus)
    : CLProgramImpl(program), mContext(&program.getContext().getImpl<CLContextVk>())
{
    // The devices associated with program come from device_list param from
    // clCreateProgramWithBinary
    for (const auto &dev : program.getDevices())
    {
        const unsigned char *binaryHandle = *binaries++;
        size_t binarySize                 = *lengths++;

        // Check for header
        if (binarySize < sizeof(ProgramBinaryOutputHeader))
        {
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_SET_ERROR(CL_INVALID_BINARY);
            return;
        }
        binarySize -= sizeof(ProgramBinaryOutputHeader);

        // Check for valid binary version from header
        const auto &binaryHeader =
            reinterpret_cast<const ProgramBinaryOutputHeader *>(binaryHandle);
        if (binaryHeader == nullptr)
        {
            ERR() << "NULL binary header!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_SET_ERROR(CL_INVALID_BINARY);
            return;
        }
        else if (binaryHeader->headerVersion < LatestSupportedBinaryVersion)
        {
            ERR() << "Binary version not compatible with runtime!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_SET_ERROR(CL_INVALID_BINARY);
            return;
        }
        binaryHandle += sizeof(ProgramBinaryOutputHeader);

        // See what kind of binary we have (i.e. SPIR-V or LLVM Bitcode)
        // https://llvm.org/docs/BitCodeFormat.html#llvm-ir-magic-number
        // https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_magic_number
        constexpr uint32_t LLVM_BC_MAGIC = 0xDEC04342;
        constexpr uint32_t SPIRV_MAGIC   = 0x07230203;
        const uint32_t &firstWord        = reinterpret_cast<const uint32_t *>(binaryHandle)[0];
        bool isBC                        = firstWord == LLVM_BC_MAGIC;
        bool isSPV                       = firstWord == SPIRV_MAGIC;
        if (!isBC && !isSPV)
        {
            ERR() << "Binary is neither SPIR-V nor LLVM Bitcode!";
            if (binaryStatus)
            {
                *binaryStatus++ = CL_INVALID_BINARY;
            }
            ANGLE_CL_SET_ERROR(CL_INVALID_BINARY);
            return;
        }

        // Add device binary to program
        DeviceProgramData devBin;
        devBin.binaryType = binaryHeader->binaryType;
        switch (devBin.binaryType)
        {
            case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
                devBin.binary.assign(binarySize / sizeof(uint32_t), 0);
                std::memcpy(devBin.binary.data(), binaryHandle, binarySize);
                break;
            case CL_PROGRAM_BINARY_TYPE_LIBRARY:
            case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
                devBin.IR.assign(binarySize, 0);
                std::memcpy(devBin.IR.data(), binaryHandle, binarySize);
                break;
            default:
                UNREACHABLE();
                ERR() << "Invalid binary type!";
                if (binaryStatus)
                {
                    *binaryStatus++ = CL_INVALID_BINARY;
                }
                ANGLE_CL_SET_ERROR(CL_INVALID_BINARY);
                return;
        }
        mAssociatedDevicePrograms[dev->getNative()] = std::move(devBin);
        if (binaryStatus)
        {
            *binaryStatus++ = CL_SUCCESS;
        }
    }
}

CLProgramVk::~CLProgramVk()
{
    for (auto &dsLayouts : mDescriptorSetLayouts)
    {
        dsLayouts.reset();
    }
    for (auto &pool : mDescriptorPools)
    {
        pool.reset();
    }
    mMetaDescriptorPool.destroy(mContext->getRenderer());
    mDescSetLayoutCache.destroy(mContext->getRenderer());
    mPipelineLayoutCache.destroy(mContext->getRenderer());
}

angle::Result CLProgramVk::build(const cl::DevicePtrs &devices,
                                 const char *options,
                                 cl::Program *notify)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLProgramVk::compile(const cl::DevicePtrs &devices,
                                   const char *options,
                                   const cl::ProgramPtrs &inputHeaders,
                                   const char **headerIncludeNames,
                                   cl::Program *notify)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLProgramVk::getInfo(cl::ProgramInfo name,
                                   size_t valueSize,
                                   void *value,
                                   size_t *valueSizeRet) const
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLProgramVk::getBuildInfo(const cl::Device &device,
                                        cl::ProgramBuildInfo name,
                                        size_t valueSize,
                                        void *value,
                                        size_t *valueSizeRet) const
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLProgramVk::createKernel(const cl::Kernel &kernel,
                                        const char *name,
                                        CLKernelImpl::Ptr *kernelOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLProgramVk::createKernels(cl_uint numKernels,
                                         CLKernelImpl::CreateFuncs &createFuncs,
                                         cl_uint *numKernelsRet)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

const CLProgramVk::DeviceProgramData *CLProgramVk::getDeviceProgramData(
    const _cl_device_id *device) const
{
    if (!mAssociatedDevicePrograms.contains(device))
    {
        WARN() << "Device (" << device << ") is not associated with program (" << this << ") !";
        return nullptr;
    }
    return &mAssociatedDevicePrograms.at(device);
}

const CLProgramVk::DeviceProgramData *CLProgramVk::getDeviceProgramData(
    const char *kernelName) const
{
    for (const auto &devProgram : mAssociatedDevicePrograms)
    {
        if (devProgram.second.containsKernel(kernelName))
        {
            return &devProgram.second;
        }
    }
    WARN() << "Kernel name (" << kernelName << ") is not associated with program (" << this
           << ") !";
    return nullptr;
}

}  // namespace rx
