//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformCL.cpp:
//    Implements the class methods for CLPlatformCL.
//

#include "libANGLE/renderer/cl/CLPlatformCL.h"

#include "libANGLE/CLPlatform.h"
#include "libANGLE/Debug.h"

#include "common/angle_version.h"

extern "C" {
#include "icd.h"
}  // extern "C"

#include <cstdlib>
#include <vector>

namespace rx
{

CLPlatformCL::~CLPlatformCL() = default;

CLPlatformCL::ImplList CLPlatformCL::GetPlatforms(bool isIcd)
{
    // Initialize Khronos OpenCL ICD Loader
    if (khrIcdVendors == nullptr)
    {
        // Store ANGLE OpenCL ICD library to make sure the ICD Loader will skip it
        khrIcdVendors          = new KHRicdVendor{};
        khrIcdVendors->library = khrIcdOsLibraryLoad(ANGLE_OPENCL_ICD_LIB_NAME);
        if (khrIcdVendors->library != nullptr)
        {
            khrIcdInitialize();
            khrIcdOsLibraryUnload(khrIcdVendors->library);
            khrIcdVendors->library = nullptr;
        }
    }

    // Try to create pass-through platform for each found ICD platform
    ImplList implList;
    for (auto *vendorIt = khrIcdVendors; vendorIt != nullptr; vendorIt = vendorIt->next)
    {
        if (vendorIt->platform != nullptr)
        {
            auto impl = Create(vendorIt->platform);
            if (impl)
            {
                implList.emplace_back(std::move(impl));
            }
        }
    }
    return implList;
}

CLPlatformCL::CLPlatformCL(cl_platform_id platform,
                           std::string &&profile,
                           std::string &&versionString,
                           cl_version version,
                           std::string &&name,
                           std::string &&extensions,
                           ExtensionList &&extensionList,
                           cl_ulong hostTimerRes)
    : mPlatform(platform),
      mProfile(std::move(profile)),
      mVersionString(std::move(versionString)),
      mName(std::move(name)),
      mExtensions(std::move(extensions)),
      mExtensionList(std::move(extensionList)),
      mVersion(version),
      mHostTimerRes(hostTimerRes)
{}

#define GET_INFO(info, size, param, size_ret) \
    result = platform->getDispatch().clGetPlatformInfo(platform, info, size, param, size_ret)

#define TRY_GET_INFO(info, size, param, size_ret)        \
    do                                                   \
    {                                                    \
        GET_INFO(info, size, param, size_ret);           \
        if (result != CL_SUCCESS)                        \
        {                                                \
            ERR() << "Failed to query CL platform info"; \
            return std::unique_ptr<CLPlatformCL>();      \
        }                                                \
    } while (0)

std::unique_ptr<CLPlatformCL> CLPlatformCL::Create(cl_platform_id platform)
{
    cl_int result    = 0;
    size_t paramSize = 0u;
    std::vector<std::string::value_type> param;

    // Skip ANGLE CL implementation to prevent passthrough loop
    TRY_GET_INFO(CL_PLATFORM_VENDOR, 0u, nullptr, &paramSize);
    param.resize(paramSize, '\0');
    TRY_GET_INFO(CL_PLATFORM_VENDOR, paramSize, param.data(), nullptr);
    if (std::string(param.data()).compare(cl::Platform::GetVendor()) == 0)
    {
        ERR() << "Tried to create CL pass-through back end for ANGLE library";
        return std::unique_ptr<CLPlatformCL>();
    }

    // Skip platform if it is not ICD compatible
    TRY_GET_INFO(CL_PLATFORM_EXTENSIONS, 0u, nullptr, &paramSize);
    param.resize(paramSize, '\0');
    TRY_GET_INFO(CL_PLATFORM_EXTENSIONS, paramSize, param.data(), nullptr);
    std::string extensions(param.data());
    if (extensions.find("cl_khr_icd") == std::string::npos)
    {
        WARN() << "CL platform is not ICD compatible";
        return std::unique_ptr<CLPlatformCL>();
    }

    // Fetch common platform info
    TRY_GET_INFO(CL_PLATFORM_PROFILE, 0u, nullptr, &paramSize);
    param.resize(paramSize, '\0');
    TRY_GET_INFO(CL_PLATFORM_PROFILE, paramSize, param.data(), nullptr);
    std::string profile(param.data());

    TRY_GET_INFO(CL_PLATFORM_VERSION, 0u, nullptr, &paramSize);
    param.resize(paramSize, '\0');
    TRY_GET_INFO(CL_PLATFORM_VERSION, paramSize, param.data(), nullptr);
    std::string versionStr(param.data());
    versionStr += " (ANGLE " ANGLE_VERSION_STRING ")";

    const auto spacePos = versionStr.find(' ');
    const auto dotPos   = versionStr.find('.');
    if (spacePos == std::string::npos || dotPos == std::string::npos)
    {
        ERR() << "Failed to extract version from OpenCL version string: " << versionStr;
        return std::unique_ptr<CLPlatformCL>();
    }
    const auto major = static_cast<cl_uint>(std::strtol(&versionStr[spacePos + 1u], nullptr, 10));
    const auto minor = static_cast<cl_uint>(std::strtol(&versionStr[dotPos + 1u], nullptr, 10));
    if (major == 0)
    {
        ERR() << "Failed to extract version from OpenCL version string: " << versionStr;
        return std::unique_ptr<CLPlatformCL>();
    }

    cl_version version = 0u;
    GET_INFO(CL_PLATFORM_NUMERIC_VERSION, sizeof(version), &version, nullptr);
    if (result != CL_SUCCESS)
    {
        version = CL_MAKE_VERSION(major, minor, 0);
    }
    else if (CL_VERSION_MAJOR(version) != major || CL_VERSION_MINOR(version) != minor)
    {
        WARN() << "CL_PLATFORM_NUMERIC_VERSION = " << CL_VERSION_MAJOR(version) << '.'
               << CL_VERSION_MINOR(version) << " does not match version string: " << versionStr;
    }

    TRY_GET_INFO(CL_PLATFORM_NAME, 0u, nullptr, &paramSize);
    param.resize(paramSize, '\0');
    TRY_GET_INFO(CL_PLATFORM_NAME, paramSize, param.data(), nullptr);
    std::string name("ANGLE pass-through -> ");
    name += param.data();

    rx::CLPlatformImpl::ExtensionList extList;
    GET_INFO(CL_PLATFORM_EXTENSIONS_WITH_VERSION, 0u, nullptr, &paramSize);
    if (result == CL_SUCCESS)
    {
        extList.resize(paramSize);
        TRY_GET_INFO(CL_PLATFORM_EXTENSIONS_WITH_VERSION, paramSize, extList.data(), nullptr);
    }

    cl_ulong hostTimerRes = 0u;
    GET_INFO(CL_PLATFORM_HOST_TIMER_RESOLUTION, sizeof(hostTimerRes), &hostTimerRes, nullptr);

    return std::unique_ptr<CLPlatformCL>(
        new CLPlatformCL(platform, std::move(profile), std::move(versionStr), version,
                         std::move(name), std::move(extensions), std::move(extList), hostTimerRes));
}

}  // namespace rx
