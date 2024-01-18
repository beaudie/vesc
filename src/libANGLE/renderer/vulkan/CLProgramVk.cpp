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

#include "clspv/Compiler.h"

#include "spirv/unified1/spirv.hpp"

#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"
#include "spirv/unified1/NonSemanticClspvReflection.h"

namespace rx
{

namespace
{
#if defined(ANGLE_ENABLE_ASSERTS)
constexpr bool kAngleDebug = true;
#else
constexpr bool kAngleDebug = false;
#endif

// Used by SPIRV-Tools to parse reflection info
spv_result_t ParseReflection(CLProgramVk::SpvReflectionData &reflectionData,
                             const spv_parsed_instruction_t &spvInstr)
{
    // Parse spir-v opcodes
    switch (spvInstr.opcode)
    {
        // --- Clspv specific parsing for below cases ---
        case spv::OpExtInst:
        {
            switch (spvInstr.words[4])
            {
                case NonSemanticClspvReflectionKernel:
                {
                    // Extract kernel name and args - add to kernel args map
                    std::string functionName = reflectionData.spvStrLookup[spvInstr.words[6]];
                    uint32_t numArgs         = reflectionData.spvIntLookup[spvInstr.words[7]];
                    reflectionData.kernelArgsMap[functionName] = CLKernelArguments();
                    reflectionData.kernelArgsMap[functionName].resize(numArgs);

                    // Store kernel flags and attributes
                    reflectionData.kernelFlags[functionName] =
                        reflectionData.spvIntLookup[spvInstr.words[8]];
                    reflectionData.kernelAttributes[functionName] =
                        reflectionData.spvStrLookup[spvInstr.words[9]];

                    // Save kernel name to reflection table for later use/lookup in parser routine
                    reflectionData.spvStrLookup[spvInstr.words[2]] = std::string(functionName);
                    break;
                }
                case NonSemanticClspvReflectionArgumentInfo:
                {
                    CLKernelVk::ArgInfo kernelArgInfo;
                    kernelArgInfo.name = reflectionData.spvStrLookup[spvInstr.words[5]];
                    // If instruction has more than 5 instruction operands (minus instruction
                    // name/opcode), that means we have arg qualifiers. ArgumentInfo also counts as
                    // an operand for OpExtInst. In below example, [ %e %f %g %h ] are the arg
                    // qualifier operands.
                    //
                    // %a = OpExtInst %b %c ArgumentInfo %d [ %e %f %g %h ]
                    if (spvInstr.num_operands > 5)
                    {
                        kernelArgInfo.typeName = reflectionData.spvStrLookup[spvInstr.words[6]];
                        kernelArgInfo.addressQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[7]];
                        kernelArgInfo.accessQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[8]];
                        kernelArgInfo.typeQualifier =
                            reflectionData.spvIntLookup[spvInstr.words[9]];
                    }
                    // Store kern arg for later lookup
                    reflectionData.kernelArgInfos[spvInstr.words[2]] = std::move(kernelArgInfo);
                    break;
                }
                case NonSemanticClspvReflectionArgumentPodUniform:
                case NonSemanticClspvReflectionArgumentPointerUniform:
                case NonSemanticClspvReflectionArgumentPodStorageBuffer:
                {
                    CLKernelArgument kernelArg;
                    if (spvInstr.num_operands == 11)
                    {
                        const CLKernelVk::ArgInfo &kernelArgInfo =
                            reflectionData.kernelArgInfos[spvInstr.words[11]];
                        kernelArg.info.name             = kernelArgInfo.name;
                        kernelArg.info.typeName         = kernelArgInfo.typeName;
                        kernelArg.info.addressQualifier = kernelArgInfo.addressQualifier;
                        kernelArg.info.accessQualifier  = kernelArgInfo.accessQualifier;
                        kernelArg.info.typeQualifier    = kernelArgInfo.typeQualifier;
                    }
                    CLKernelArguments &kernelArgs =
                        reflectionData
                            .kernelArgsMap[reflectionData.spvStrLookup[spvInstr.words[5]]];
                    kernelArg.type    = spvInstr.words[4];
                    kernelArg.used    = true;
                    kernelArg.ordinal = reflectionData.spvIntLookup[spvInstr.words[6]];
                    kernelArg.op3     = reflectionData.spvIntLookup[spvInstr.words[7]];
                    kernelArg.op4     = reflectionData.spvIntLookup[spvInstr.words[8]];
                    kernelArg.op5     = reflectionData.spvIntLookup[spvInstr.words[9]];
                    kernelArg.op6     = reflectionData.spvIntLookup[spvInstr.words[10]];

                    if (!kernelArgs.empty())
                    {
                        kernelArgs.at(kernelArg.ordinal) = std::move(kernelArg);
                    }
                    break;
                }
                case NonSemanticClspvReflectionArgumentUniform:
                case NonSemanticClspvReflectionArgumentWorkgroup:
                case NonSemanticClspvReflectionArgumentStorageBuffer:
                case NonSemanticClspvReflectionArgumentPodPushConstant:
                case NonSemanticClspvReflectionArgumentPointerPushConstant:
                {
                    CLKernelArgument kernelArg;
                    if (spvInstr.num_operands == 9)
                    {
                        const CLKernelVk::ArgInfo &kernelArgInfo =
                            reflectionData.kernelArgInfos[spvInstr.words[9]];
                        kernelArg.info.name             = kernelArgInfo.name;
                        kernelArg.info.typeName         = kernelArgInfo.typeName;
                        kernelArg.info.addressQualifier = kernelArgInfo.addressQualifier;
                        kernelArg.info.accessQualifier  = kernelArgInfo.accessQualifier;
                        kernelArg.info.typeQualifier    = kernelArgInfo.typeQualifier;
                    }
                    CLKernelArguments &kernelArgs =
                        reflectionData
                            .kernelArgsMap[reflectionData.spvStrLookup[spvInstr.words[5]]];
                    kernelArg.type    = spvInstr.words[4];
                    kernelArg.used    = true;
                    kernelArg.ordinal = reflectionData.spvIntLookup[spvInstr.words[6]];
                    kernelArg.op3     = reflectionData.spvIntLookup[spvInstr.words[7]];
                    kernelArg.op4     = reflectionData.spvIntLookup[spvInstr.words[8]];
                    kernelArgs.at(kernelArg.ordinal) = std::move(kernelArg);
                    break;
                }
                case NonSemanticClspvReflectionPushConstantGlobalOffset:
                case NonSemanticClspvReflectionPushConstantRegionOffset:
                {
                    uint32_t offset = reflectionData.spvIntLookup[spvInstr.words[5]];
                    uint32_t size   = reflectionData.spvIntLookup[spvInstr.words[6]];
                    reflectionData.pushConstants[spvInstr.words[4]] = {
                        .stageFlags = 0, .offset = offset, .size = size};
                    break;
                }
                case NonSemanticClspvReflectionSpecConstantWorkgroupSize:
                {
                    reflectionData.specConstantWGS = {
                        reflectionData.spvIntLookup[spvInstr.words[5]],
                        reflectionData.spvIntLookup[spvInstr.words[6]],
                        reflectionData.spvIntLookup[spvInstr.words[7]]};
                    break;
                }
                case NonSemanticClspvReflectionPropertyRequiredWorkgroupSize:
                {
                    reflectionData
                        .kernelCompileWGS[reflectionData.spvStrLookup[spvInstr.words[5]]] = {
                        reflectionData.spvIntLookup[spvInstr.words[6]],
                        reflectionData.spvIntLookup[spvInstr.words[7]],
                        reflectionData.spvIntLookup[spvInstr.words[8]]};
                    break;
                }
                default:
                    break;
            }
            break;
        }
        // --- Regular SPIR-V opcode parsing for below cases ---
        case spv::OpString:
        {
            reflectionData.spvStrLookup[spvInstr.words[1]] =
                reinterpret_cast<const char *>(&spvInstr.words[2]);
            break;
        }
        case spv::OpConstant:
        {
            reflectionData.spvIntLookup[spvInstr.words[2]] = spvInstr.words[3];
            break;
        }
        default:
            break;
    }
    return SPV_SUCCESS;
}

class CLAsyncBuildTask : public angle::Closure
{
  public:
    CLAsyncBuildTask(CLProgramVk *programVk,
                     const cl::DevicePtrs &devices,
                     std::string options,
                     std::string internalOptions,
                     CLProgramVk::BuildType buildType,
                     const CLProgramVk::DeviceProgramDatas &inputProgramDatas,
                     cl::Program *notify)
        : mProgramVk(programVk),
          mDevices(devices),
          mOptions(options),
          mInternalOptions(internalOptions),
          mBuildType(buildType),
          mDeviceProgramDatas(inputProgramDatas),
          mNotify(notify)
    {}

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CLProgramVk::buildInternal (async)");
        CLProgramVk::ScopedProgramCallback spc(mNotify);
        if (!mProgramVk->buildInternal(mDevices, mOptions, mInternalOptions, mBuildType,
                                       mDeviceProgramDatas))
        {
            ERR() << "Async build failed for program (" << mProgramVk
                  << ")! Check the build status or build log for details.";
        }
    }

  private:
    CLProgramVk *mProgramVk;
    const cl::DevicePtrs mDevices;
    std::string mOptions;
    std::string mInternalOptions;
    CLProgramVk::BuildType mBuildType;
    const CLProgramVk::DeviceProgramDatas mDeviceProgramDatas;
    cl::Program *mNotify;
};

}  // namespace

CLProgramVk::CLProgramVk(const cl::Program &program)
    : CLProgramImpl(program), mContext(&program.getContext().getImpl<CLContextVk>())
{
    cl::DevicePtrs devices;
    ANGLE_CL_IMPL_TRY(mContext->getDevices(&devices));

    // The devices associated with the program object are the devices associated with context
    for (const auto &device : devices)
    {
        mAssociatedDevicePrograms[device->getNative()] = DeviceProgramData{};
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
    for (const cl::DevicePtr &device : program.getDevices())
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
        const ProgramBinaryOutputHeader *binaryHeader =
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
        DeviceProgramData deviceBin;
        deviceBin.binaryType = binaryHeader->binaryType;
        switch (deviceBin.binaryType)
        {
            case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
                deviceBin.binary.assign(binarySize / sizeof(uint32_t), 0);
                std::memcpy(deviceBin.binary.data(), binaryHandle, binarySize);
                break;
            case CL_PROGRAM_BINARY_TYPE_LIBRARY:
            case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
                deviceBin.IR.assign(binarySize, 0);
                std::memcpy(deviceBin.IR.data(), binaryHandle, binarySize);
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
        mAssociatedDevicePrograms[device->getNative()] = std::move(deviceBin);
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
    BuildType buildType = !mProgram.getSource().empty() ? BuildType::BUILD : BuildType::BINARY;
    const cl::DevicePtrs &devicePtrs = !devices.empty() ? devices : mProgram.getDevices();

    if (notify)
    {
        std::shared_ptr<angle::WaitableEvent> asyncEvent =
            mProgram.getContext().getPlatform().getMultiThreadPool()->postWorkerTask(
                std::make_shared<CLAsyncBuildTask>(this, devicePtrs,
                                                   std::string(options ? options : ""), "",
                                                   buildType, DeviceProgramDatas{}, notify));
        if (asyncEvent == nullptr)
        {
            ERR() << "MultiThreadPool failed to create async build task!";
            notify->callback();
            ANGLE_CL_RETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
        }
    }
    else
    {
        if (!buildInternal(devicePtrs, std::string(options ? options : ""), "", buildType,
                           DeviceProgramDatas{}))
        {
            ANGLE_CL_RETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
        }
    }
    return angle::Result::Continue;
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
    cl_uint valUInt            = 0u;
    void *valPointer           = nullptr;
    const void *copyValue      = nullptr;
    size_t copySize            = 0u;
    unsigned char **outputBins = reinterpret_cast<unsigned char **>(value);
    std::string kernelNamesList;
    std::vector<size_t> vBinarySizes;

    switch (name)
    {
        case cl::ProgramInfo::NumKernels:
            for (const auto &dev : mAssociatedDevicePrograms)
            {
                valUInt += static_cast<decltype(valUInt)>(dev.second.numKernels());
            }
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case cl::ProgramInfo::BinarySizes:
        {
            for (const auto &dev : mAssociatedDevicePrograms)
            {
                vBinarySizes.push_back(sizeof(ProgramBinaryOutputHeader) +
                                       (dev.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                                            ? dev.second.binary.size() * sizeof(uint32_t)
                                            : dev.second.IR.size()));
            }
            valPointer = vBinarySizes.data();
            copyValue  = valPointer;
            copySize   = vBinarySizes.size() * sizeof(size_t);
            break;
        }
        case cl::ProgramInfo::Binaries:
            for (const auto &dev : mAssociatedDevicePrograms)
            {
                const void *bin = dev.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                                      ? reinterpret_cast<const void *>(dev.second.binary.data())
                                      : reinterpret_cast<const void *>(dev.second.IR.data());
                size_t binSize  = dev.second.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE
                                      ? dev.second.binary.size() * sizeof(uint32_t)
                                      : dev.second.IR.size();
                ProgramBinaryOutputHeader header{.headerVersion = LatestSupportedBinaryVersion,
                                                 .binaryType    = dev.second.binaryType};

                if (outputBins != nullptr)
                {
                    if (*outputBins != nullptr)
                    {
                        std::memcpy(*outputBins, &header, sizeof(ProgramBinaryOutputHeader));
                        std::memcpy((*outputBins) + sizeof(ProgramBinaryOutputHeader), bin,
                                    binSize);
                    }
                    outputBins++;
                }

                // Spec just wants pointer size here
                copySize += sizeof(unsigned char *);
            }
            // We already copied the (headers + binaries) over - nothing else left to copy
            copyValue = nullptr;
            break;
        case cl::ProgramInfo::KernelNames:
            for (const auto &dev : mAssociatedDevicePrograms)
            {
                kernelNamesList = dev.second.getKernelNames();
            }
            valPointer = kernelNamesList.data();
            copyValue  = valPointer;
            copySize   = kernelNamesList.size() + 1;
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Program Object Queries table and param_value is not NULL.
        if (valueSize < copySize)
        {
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
        }
        if (copyValue != nullptr)
        {
            std::memcpy(value, copyValue, copySize);
        }
    }
    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }

    return angle::Result::Continue;
}

angle::Result CLProgramVk::getBuildInfo(const cl::Device &device,
                                        cl::ProgramBuildInfo name,
                                        size_t valueSize,
                                        void *value,
                                        size_t *valueSizeRet) const
{
    cl_uint valUInt           = 0;
    cl_build_status valStatus = 0;
    const void *copyValue     = nullptr;
    size_t copySize           = 0;
    auto &deviceProgramData   = mAssociatedDevicePrograms.at(device.getNative());

    switch (name)
    {
        case cl::ProgramBuildInfo::Status:
            valStatus = deviceProgramData.buildStatus;
            copyValue = &valStatus;
            copySize  = sizeof(valStatus);
            break;
        case cl::ProgramBuildInfo::Log:
            copyValue = deviceProgramData.buildLog.c_str();
            copySize  = deviceProgramData.buildLog.size() + 1;
            break;
        case cl::ProgramBuildInfo::Options:
            copyValue = mProgramOpts.c_str();
            copySize  = mProgramOpts.size() + 1;
            break;
        case cl::ProgramBuildInfo::BinaryType:
            valUInt   = deviceProgramData.binaryType;
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case cl::ProgramBuildInfo::GlobalVariableTotalSize:
            // Returns 0 if device does not support program scope global variables.
            valUInt   = 0;
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((value != nullptr) && (copySize > valueSize))
    {
        ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }
    else if ((value != nullptr) && (copyValue != nullptr))
    {
        memcpy(value, copyValue, std::min(valueSize, copySize));
    }

    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }

    return angle::Result::Continue;
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

bool CLProgramVk::buildInternal(const cl::DevicePtrs &devices,
                                std::string options,
                                std::string internalOptions,
                                BuildType buildType,
                                const DeviceProgramDatas &inputProgramDatas)
{
    std::scoped_lock<std::mutex> sl(mProgramMutex);
    std::string buildOptions(options + " " + internalOptions);

    // Cache original options string
    mProgramOpts = options;

    // Build for each associated device
    for (const auto &device : devices)
    {
        DeviceProgramData &deviceProgramData = mAssociatedDevicePrograms[device->getNative()];
        deviceProgramData.buildStatus        = CL_BUILD_IN_PROGRESS;

        if (buildType != BuildType::BINARY)
        {
            // Internal Clspv compiler flags
            buildOptions += " --long-vector";

            const bool compiledObj   = buildType == BuildType::COMPILE;
            const bool createLibrary = buildOptions.find("-create-library") != std::string::npos;
            if (createLibrary)
            {
                buildOptions = buildOptions.erase(buildOptions.find("-create-library"),
                                                  strlen("-create-library"));
            }

            // Invoke clspv
            switch (buildType)
            {
                case BuildType::COMPILE:
                {
                    buildOptions += " --output-format=bc";
                    [[fallthrough]];
                }
                case BuildType::BUILD:
                {
                    ScopedClspvContext clspvCtx;
                    const char *clSrc   = mProgram.getSource().c_str();
                    ClspvError clspvRet = clspvCompileFromSourcesString(
                        1, NULL, static_cast<const char **>(&clSrc), buildOptions.c_str(),
                        &clspvCtx.mOutputBin, &clspvCtx.mOutputBinSize, &clspvCtx.mOutputBuildLog);
                    deviceProgramData.buildLog =
                        clspvCtx.mOutputBuildLog != nullptr ? clspvCtx.mOutputBuildLog : "";
                    if (clspvRet != CLSPV_SUCCESS)
                    {
                        ERR() << "OpenCL build failed with: ClspvError(" << clspvRet << ")!";
                        deviceProgramData.buildStatus = CL_BUILD_ERROR;
                        return false;
                    }

                    if (compiledObj)
                    {
                        deviceProgramData.IR.assign(clspvCtx.mOutputBinSize, 0);
                        std::memcpy(deviceProgramData.IR.data(), clspvCtx.mOutputBin,
                                    clspvCtx.mOutputBinSize);
                    }
                    else
                    {
                        deviceProgramData.binary.assign(clspvCtx.mOutputBinSize / sizeof(uint32_t),
                                                        0);
                        std::memcpy(deviceProgramData.binary.data(), clspvCtx.mOutputBin,
                                    clspvCtx.mOutputBinSize);
                    }
                    break;
                }
                case BuildType::LINK:
                {
                    buildOptions += createLibrary ? " --output-format=bc" : "";
                    buildOptions += " -x ir";

                    ScopedClspvContext clspvCtx;
                    std::vector<size_t> vSizes;
                    std::vector<const char *> vBins;
                    for (const auto &inputProgramData : inputProgramDatas)
                    {
                        vSizes.push_back(inputProgramData->IR.size());
                        vBins.push_back(inputProgramData->IR.data());
                    }
                    ClspvError clspvRet = clspvCompileFromSourcesString(
                        inputProgramDatas.size(), vSizes.data(), vBins.data(), buildOptions.c_str(),
                        &clspvCtx.mOutputBin, &clspvCtx.mOutputBinSize, &clspvCtx.mOutputBuildLog);
                    deviceProgramData.buildLog =
                        clspvCtx.mOutputBuildLog != nullptr ? clspvCtx.mOutputBuildLog : "";
                    if (clspvRet != CLSPV_SUCCESS)
                    {
                        ERR() << "OpenCL build failed with: ClspvError(" << clspvRet << ")!";
                        deviceProgramData.buildStatus = CL_BUILD_ERROR;
                        return false;
                    }

                    deviceProgramData.IR.assign(clspvCtx.mOutputBinSize, 0);
                    std::memcpy(deviceProgramData.IR.data(), clspvCtx.mOutputBin,
                                clspvCtx.mOutputBinSize);
                    break;
                }
                default:
                {
                    UNREACHABLE();
                    return false;
                }
            }
            deviceProgramData.binaryType = createLibrary ? CL_PROGRAM_BINARY_TYPE_LIBRARY
                                           : compiledObj ? CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT
                                                         : CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
        }

        // Extract reflection info from spv binary and populate reflection data
        if (deviceProgramData.binaryType == CL_PROGRAM_BINARY_TYPE_EXECUTABLE)
        {
            spvtools::SpirvTools spvTool(SPV_ENV_UNIVERSAL_1_5);
            bool parseRet = spvTool.Parse(
                deviceProgramData.binary,
                [](const spv_endianness_t endianess, const spv_parsed_header_t &instruction) {
                    return SPV_SUCCESS;
                },
                [&deviceProgramData](const spv_parsed_instruction_t &instruction) {
                    return ParseReflection(deviceProgramData.reflectionData, instruction);
                });
            if (!parseRet)
            {
                ERR() << "Failed to parse reflection info from SPIR-V!";
                return false;
            }

            // Setup inital push constant range
            uint32_t pc_min_offset = UINT32_MAX, pc_max_offset = 0, pc_max_size = 0;
            for (const auto &pc : deviceProgramData.reflectionData.pushConstants)
            {
                pc_min_offset = pc.second.offset < pc_min_offset ? pc.second.offset : pc_min_offset;
                if (pc.second.offset >= pc_max_offset)
                {
                    pc_max_offset = pc.second.offset;
                    pc_max_size   = pc.second.size;
                }
            }
            deviceProgramData.pushConstRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            deviceProgramData.pushConstRange.offset =
                pc_min_offset == UINT32_MAX ? 0 : pc_min_offset;
            deviceProgramData.pushConstRange.size = pc_max_offset + pc_max_size;

            if (kAngleDebug)
            {
                if (mContext->getFeatures().clDumpVkSpirv.enabled)
                {
                    angle::spirv::Print(deviceProgramData.binary);
                }
            }
        }
        deviceProgramData.buildStatus = CL_BUILD_SUCCESS;
    }
    return true;
}

angle::spirv::Blob CLProgramVk::stripReflection(const DeviceProgramData *deviceProgramData)
{
    angle::spirv::Blob binaryStripped;
    spvtools::Optimizer opt(SPV_ENV_UNIVERSAL_1_5);
    opt.RegisterPass(spvtools::CreateStripReflectInfoPass());
    spvtools::OptimizerOptions optOptions;
    optOptions.set_run_validator(false);
    if (!opt.Run(deviceProgramData->binary.data(), deviceProgramData->binary.size(),
                 &binaryStripped, optOptions))
    {
        ERR() << "Could not strip reflection data from binary!";
    }
    return binaryStripped;
}

}  // namespace rx
