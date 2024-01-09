//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceVk.cpp: Implements the class methods for CLDeviceVk.

#include "libANGLE/renderer/vulkan/CLDeviceVk.h"
#include "libANGLE/renderer/vulkan/CLPlatformVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

#include "libANGLE/Display.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLDeviceVk::CLDeviceVk(const cl::Device &device, const egl::Display *display) : CLDeviceImpl(device)
{
    ASSERT(display);
    ASSERT(display->isInitialized());

    using info                              = cl::DeviceInfo;
    auto displayVk                          = GetImplAs<DisplayVk>(display);
    mRenderer                               = displayVk->getRenderer();
    const VkPhysicalDeviceProperties &props = mRenderer->getPhysicalDeviceProperties();

    // Setup initial device mInfo fields
    mInfoString = {{info::Name, std::string(props.deviceName)},
                   {info::Vendor, mRenderer->getVendorString()},
                   {info::DriverVersion, mRenderer->getVersionString(true)},
                   {info::Version, std::string("OpenCL 1.2 " + mRenderer->getVersionString(true))},
                   {info::Profile, std::string("FULL_PROFILE")},
                   {info::OpenCL_C_Version, std::string("OpenCL C 1.2 ")},
                   {info::LatestConformanceVersionPassed, std::string("FIXME")}};

    mInfoSizeT = {
        {info::MaxWorkGroupSize, props.limits.maxComputeWorkGroupInvocations},

        {info::MaxGlobalVariableSize, 0},
        {info::GlobalVariablePreferredTotalSize, 0},

        // TODO: Evaluate/Replace these hardcoded values
        {info::MaxParameterSize, 1024},
        {info::ProfilingTimerResolution, 1},
        {info::PrintfBufferSize, 1024 * 1024},
        {info::PreferredWorkGroupSizeMultiple, 16},
    };

    mInfoULong = {
        {info::LocalMemSize, props.limits.maxComputeSharedMemorySize},

        {info::SVM_Capabilities, 0},
        {info::QueueOnDeviceProperties, 0},
        {info::PartitionAffinityDomain, 0},
        {info::DeviceEnqueueCapabilities, 0},
        {info::QueueOnHostProperties, CL_QUEUE_PROFILING_ENABLE},

        // TODO: Evaluate/Replace these hardcoded values
        {info::HalfFpConfig, 0},
        {info::DoubleFpConfig, 0},
        {info::GlobalMemCacheSize, 0},
        {info::GlobalMemSize, 1024 * 1024 * 1024},
        {info::MaxConstantBufferSize, 64 * 1024},
        {info::SingleFpConfig, CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN | CL_FP_FMA},
        {info::AtomicMemoryCapabilities,
         CL_DEVICE_ATOMIC_ORDER_RELAXED | CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP},
        {info::AtomicFenceCapabilities, CL_DEVICE_ATOMIC_ORDER_RELAXED |
                                            CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM |
                                            CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP},
    };

    mInfoUInt = {
        {info::VendorID, props.vendorID},
        {info::MaxReadImageArgs, props.limits.maxPerStageDescriptorSampledImages},
        {info::MaxWriteImageArgs, props.limits.maxPerStageDescriptorStorageImages},
        {info::MaxReadWriteImageArgs, props.limits.maxPerStageDescriptorStorageImages},
        {info::GlobalMemCachelineSize, static_cast<cl_uint>(props.limits.nonCoherentAtomSize)},

        {info::Available, CL_TRUE},
        {info::LinkerAvailable, CL_TRUE},
        {info::CompilerAvailable, CL_TRUE},
        {info::MaxOnDeviceQueues, 0},
        {info::MaxOnDeviceEvents, 0},
        {info::QueueOnDeviceMaxSize, 0},
        {info::QueueOnDevicePreferredSize, 0},
        {info::MaxPipeArgs, 0},
        {info::PipeMaxPacketSize, 0},
        {info::PipeSupport, CL_FALSE},
        {info::PipeMaxActiveReservations, 0},
        {info::ErrorCorrectionSupport, CL_FALSE},
        {info::PreferredInteropUserSync, CL_TRUE},
        {info::ExecutionCapabilities, CL_EXEC_KERNEL},

        // TODO: Evaluate/Replace these hardcoded values
        {info::AddressBits, 64},
        {info::EndianLittle, CL_TRUE},
        {info::LocalMemType, CL_LOCAL},
        {info::MaxSamplers, 0},
        {info::MaxConstantArgs, 8},
        {info::MaxNumSubGroups, 0},
        {info::MaxComputeUnits, 4},
        {info::MaxClockFrequency, 555},
        {info::MaxWorkItemDimensions, 3},
        {info::MinDataTypeAlignSize, 128},
        {info::GlobalMemCacheType, CL_NONE},
        {info::HostUnifiedMemory, CL_TRUE},
        {info::NativeVectorWidthChar, 4},
        {info::NativeVectorWidthShort, 2},
        {info::NativeVectorWidthInt, 1},
        {info::NativeVectorWidthLong, 1},
        {info::NativeVectorWidthFloat, 1},
        {info::NativeVectorWidthDouble, 1},
        {info::NativeVectorWidthHalf, 0},
        {info::PartitionMaxSubDevices, 0},
        {info::PreferredVectorWidthInt, 1},
        {info::PreferredVectorWidthLong, 1},
        {info::PreferredVectorWidthChar, 4},
        {info::PreferredVectorWidthHalf, 0},
        {info::PreferredVectorWidthShort, 2},
        {info::PreferredVectorWidthFloat, 1},
        {info::PreferredVectorWidthDouble, 0},
        {info::PreferredLocalAtomicAlignment, 0},
        {info::PreferredGlobalAtomicAlignment, 0},
        {info::PreferredPlatformAtomicAlignment, 0},
        {info::NonUniformWorkGroupSupport, CL_TRUE},
        {info::GenericAddressSpaceSupport, CL_FALSE},
        {info::SubGroupIndependentForwardProgress, CL_FALSE},
        {info::WorkGroupCollectiveFunctionsSupport, CL_FALSE},
    };
}

CLDeviceVk::~CLDeviceVk() = default;

CLDeviceImpl::Info CLDeviceVk::createInfo(cl::DeviceType type) const
{
    Info info(type);

    auto properties = mRenderer->getPhysicalDeviceProperties();

    info.maxWorkItemSizes.push_back(properties.limits.maxComputeWorkGroupSize[0]);
    info.maxWorkItemSizes.push_back(properties.limits.maxComputeWorkGroupSize[1]);
    info.maxWorkItemSizes.push_back(properties.limits.maxComputeWorkGroupSize[2]);

    // TODO: Query from Vulkan
    info.maxMemAllocSize = 1 << 30;

    // TODO: We currently do not support images
    info.imageSupport = CL_FALSE;

    info.image2D_MaxWidth          = properties.limits.maxImageDimension2D;
    info.image2D_MaxHeight         = properties.limits.maxImageDimension2D;
    info.image3D_MaxWidth          = properties.limits.maxImageDimension3D;
    info.image3D_MaxHeight         = properties.limits.maxImageDimension3D;
    info.image3D_MaxDepth          = properties.limits.maxImageDimension3D;
    info.imageMaxBufferSize        = properties.limits.maxImageDimension1D;
    info.imageMaxArraySize         = properties.limits.maxImageArrayLayers;
    info.imagePitchAlignment       = 0u;
    info.imageBaseAddressAlignment = 0u;

    info.memBaseAddrAlign     = 1024;
    info.execCapabilities     = CL_EXEC_KERNEL;
    info.queueOnDeviceMaxSize = 0u;
    info.builtInKernels       = "";
    info.version              = CL_MAKE_VERSION(1, 2, 0);
    info.versionStr           = "OpenCL 1.2 " + mRenderer->getVersionString(true);
    info.OpenCL_C_AllVersions = {{CL_MAKE_VERSION(3, 0, 0), "OpenCL C"},
                                 {CL_MAKE_VERSION(2, 0, 0), "OpenCL C"},
                                 {CL_MAKE_VERSION(1, 2, 0), "OpenCL C"},
                                 {CL_MAKE_VERSION(1, 1, 0), "OpenCL C"},
                                 {CL_MAKE_VERSION(1, 0, 0), "OpenCL C"}};

    info.OpenCL_C_Features         = {};
    info.extensionsWithVersion     = {};
    info.ILsWithVersion            = {};
    info.builtInKernelsWithVersion = {};
    info.partitionProperties       = {};
    info.partitionType             = {};
    info.IL_Version                = "";

    // Below extensions are required as of OpenCL 1.1
    info.extensions =
        "cl_khr_byte_addressable_store "
        "cl_khr_global_int32_base_atomics "
        "cl_khr_global_int32_extended_atomics "
        "cl_khr_local_int32_base_atomics "
        "cl_khr_local_int32_extended_atomics ";

    return info;
}

angle::Result CLDeviceVk::getInfoUInt(cl::DeviceInfo name, cl_uint *value) const
{
    if (mInfoUInt.count(name))
    {
        *value = mInfoUInt.at(name);
        return angle::Result::Continue;
    }
    ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
}

angle::Result CLDeviceVk::getInfoULong(cl::DeviceInfo name, cl_ulong *value) const
{
    if (mInfoULong.count(name))
    {
        *value = mInfoULong.at(name);
        return angle::Result::Continue;
    }
    ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
}

angle::Result CLDeviceVk::getInfoSizeT(cl::DeviceInfo name, size_t *value) const
{
    if (mInfoSizeT.count(name))
    {
        *value = mInfoSizeT.at(name);
        return angle::Result::Continue;
    }
    ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
}

angle::Result CLDeviceVk::getInfoStringLength(cl::DeviceInfo name, size_t *value) const
{
    if (mInfoString.count(name))
    {
        *value = mInfoString.at(name).length() + 1;
        return angle::Result::Continue;
    }
    ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
}

angle::Result CLDeviceVk::getInfoString(cl::DeviceInfo name, size_t size, char *value) const
{
    if (mInfoString.count(name))
    {
        std::strcpy(value, mInfoString.at(name).c_str());
        return angle::Result::Continue;
    }
    ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
}

angle::Result CLDeviceVk::createSubDevices(const cl_device_partition_property *properties,
                                           cl_uint numDevices,
                                           CreateFuncs &subDevices,
                                           cl_uint *numDevicesRet)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

}  // namespace rx
