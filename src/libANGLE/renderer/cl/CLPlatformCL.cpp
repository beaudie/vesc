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

#include "anglebase/no_destructor.h"

#include "CL/cl_icd.h"

#include <cstdlib>
#include <vector>

#define GET_INFO(info, size, param, size_ret) \
    result = getPlatformInfo(platform, info, size, param, size_ret)

#define TRY_GET_INFO(info, size, param, size_ret)        \
    do                                                   \
    {                                                    \
        GET_INFO(info, size, param, size_ret);           \
        if (result != CL_SUCCESS)                        \
        {                                                \
            ERR() << "Failed to query CL platform info"; \
            continue;                                    \
        }                                                \
    } while (0)

namespace rx
{

CLPlatformCL::~CLPlatformCL() = default;

CLPlatformCL::ImplList CLPlatformCL::GetPlatforms()
{
    ImplList implList;

    // Load system OpenCL library and fetch entry points required to query platforms
    if (!Library())
    {
        Library().reset(angle::OpenSharedLibrary("OpenCL", angle::SearchType::SystemDir));
        if (!Library())
        {
            return implList;
        }
    }
    if (Library()->getSymbol("gANGLE") != nullptr)
    {
        ERR() << "Found ANGLE OpenCL library instead of system library";
        return implList;
    }
    auto getPlatformIDs =
        reinterpret_cast<cl_api_clGetPlatformIDs>(Library()->getSymbol("clGetPlatformIDs"));
    auto getPlatformInfo =
        reinterpret_cast<cl_api_clGetPlatformInfo>(Library()->getSymbol("clGetPlatformInfo"));
    if (getPlatformIDs == nullptr || getPlatformInfo == nullptr)
    {
        ERR() << "Failed to fetch entry points from OpenCL library";
        return implList;
    }

    // Fetch CL platforms
    cl_uint platformCount = 0u;
    cl_int result         = getPlatformIDs(0u, nullptr, &platformCount);
    if (result != CL_SUCCESS)
    {
        ERR() << "Failed to query platforms from OpenCL library";
        return implList;
    }
    std::vector<cl_platform_id> platforms(platformCount, nullptr);
    result = getPlatformIDs(platformCount, platforms.data(), nullptr);
    if (result != CL_SUCCESS)
    {
        ERR() << "Failed to query platforms from OpenCL library";
        return implList;
    }

    for (auto platform : platforms)
    {
        // Skip ANGLE CL implementation to prevent passthrough loop
        size_t paramSize = 0u;
        TRY_GET_INFO(CL_PLATFORM_VENDOR, 0u, nullptr, &paramSize);
        std::vector<std::string::value_type> param(paramSize, '\0');
        TRY_GET_INFO(CL_PLATFORM_VENDOR, paramSize, param.data(), nullptr);
        if (std::string(param.data()).compare(cl::Platform::GetVendor()) == 0)
        {
            continue;
        }

        // Skip platform if it is not ICD compatible
        TRY_GET_INFO(CL_PLATFORM_EXTENSIONS, 0u, nullptr, &paramSize);
        param.resize(paramSize, '\0');
        TRY_GET_INFO(CL_PLATFORM_EXTENSIONS, paramSize, param.data(), nullptr);
        std::string extensions(param.data());
        if (extensions.find("cl_khr_icd") == std::string::npos)
        {
            WARN() << "CL platform is not ICD compatible";
            continue;
        }

        // Fetch common platform info
        TRY_GET_INFO(CL_PLATFORM_PROFILE, 0u, nullptr, &paramSize);
        param.resize(paramSize, '\0');
        TRY_GET_INFO(CL_PLATFORM_PROFILE, paramSize, param.data(), nullptr);
        std::string profile(param.data());

        TRY_GET_INFO(CL_PLATFORM_VERSION, 0u, nullptr, &paramSize);
        param.resize(paramSize, '\0');
        TRY_GET_INFO(CL_PLATFORM_VERSION, paramSize, param.data(), nullptr);
        std::string versionString(param.data());

        const auto spacePos = versionString.find(' ');
        const auto dotPos   = versionString.find('.');
        if (spacePos == std::string::npos || dotPos == std::string::npos)
        {
            ERR() << "Failed to extract version from OpenCL version string: " << versionString;
            continue;
        }
        const auto major =
            static_cast<cl_uint>(std::strtol(&versionString[spacePos + 1u], nullptr, 10));
        const auto minor =
            static_cast<cl_uint>(std::strtol(&versionString[dotPos + 1u], nullptr, 10));
        if (major == 0)
        {
            ERR() << "Failed to extract version from OpenCL version string: " << versionString;
            continue;
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
                   << CL_VERSION_MINOR(version)
                   << " does not match version string: " << versionString;
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

        implList.emplace_back(new CLPlatformCL(
            platform, std::move(profile), std::move(versionString), version, std::move(name),
            std::move(extensions), std::move(extList), hostTimerRes));
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

std::unique_ptr<angle::Library> &CLPlatformCL::Library()
{
    static angle::base::NoDestructor<std::unique_ptr<angle::Library>> sLibrary;
    return *sLibrary;
}

}  // namespace rx
