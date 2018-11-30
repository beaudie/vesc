//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util.cpp: Helps client APIs make decisions based on rules
// data files.  For example, the Android EGL loader uses this library to
// determine whether to use ANGLE or a native GLES driver.

#include "feature_support_util.h"
#include "common/platform.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#include <android/log.h>
#include <unistd.h>
#endif
#include <fstream>
#include <list>

#include "../gpu_info_util/SystemInfo.h"

namespace angle
{

}  // namespace angle

extern "C" {

using namespace angle;

#if defined(ANGLE_PLATFORM_ANDROID)
// This function is part of the NOW-DEPRECATED version-1 API:
ANGLE_EXPORT bool ANGLEGetUtilityAPI(unsigned int *versionToUse)
{
    if (*versionToUse >= kFeatureVersion_LowestSupported)
    {
        if (*versionToUse <= kFeatureVersion_HighestSupported)
        {
            // This versionToUse is valid, and doesn't need to be changed.
            return true;
        }
        else
        {
            // The versionToUse is greater than the highest version supported; change it to the
            // highest version supported (caller will decide if it can use that version).
            *versionToUse = kFeatureVersion_HighestSupported;
            return true;
        }
    }
    else
    {
        // The versionToUse is less than the lowest version supported, which is an error.
        return false;
    }
}

// This function is part of the NOW-DEPRECATED version-1 API:
ANGLE_EXPORT bool AndroidUseANGLEForApplication(int rules_fd,
                                                long rules_offset,
                                                long rules_length,
                                                const char *appName,
                                                const char *deviceMfr,
                                                const char *deviceModel)
{
    Scenario scenario(appName, deviceMfr, deviceModel);
    bool rtn = false;
    scenario.logScenario();

    // Read the contents of the file into a string and then parse it:
    if (rules_fd < 0)
    {
        WARN("Asked to read a non-open JSON file");
        return rtn;
    }
    off_t fileSize       = rules_length;
    off_t startOfContent = rules_offset;
    lseek(rules_fd, startOfContent, SEEK_SET);
    char *buffer                 = new char[fileSize + 1];
    ssize_t numBytesRead         = read(rules_fd, buffer, fileSize);
    buffer[numBytesRead]         = '\0';
    std::string jsonFileContents = std::string(buffer);
    delete[] buffer;
    RuleList *rules = RuleList::ReadRulesFromJsonString(jsonFileContents);
    rules->logRules();

    rtn = rules->getUseANGLE(scenario);
    VERBOSE("Application \"%s\" should %s ANGLE", appName, rtn ? "use" : "NOT use");

    delete rules;
    return rtn;
}
#endif  // if defined(ANGLE_PLATFORM_ANDROID)

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEGetFeatureSupportUtilAPIVersion(unsigned int *versionToUse)
{
    if (!versionToUse || (*versionToUse < kFeatureVersion_LowestSupported))
    {
        // The versionToUse is either nullptr or is less than the lowest version supported, which
        // is an error.
        return false;
    }
    if (*versionToUse > kFeatureVersion_HighestSupported)
    {
        // The versionToUse is greater than the highest version supported; change it to the
        // highest version supported (caller will decide if it can use that version).
        *versionToUse = kFeatureVersion_HighestSupported;
    }
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEAndroidParseRulesString(const char *rulesString,
                                               RulesHandle *rulesHandle,
                                               int *rulesVersion)
{
    if (!rulesString || !rulesHandle || !rulesVersion)
    {
        return false;
    }

    std::string rulesFileContents = rulesString;
    RuleList *rules               = RuleList::ReadRulesFromJsonString(rulesFileContents);
    rules->logRules();

    *rulesHandle  = rules;
    *rulesVersion = 0;
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEGetSystemInfo(SystemInfoHandle *systemInfoHandle)
{
    if (!systemInfoHandle)
    {
        return false;
    }

    angle::SystemInfo *systemInfo = new angle::SystemInfo;
    if (GetSystemInfo(systemInfo))
    {
        *systemInfoHandle = systemInfo;
        return true;
    }
    return false;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEAddDeviceInfoToSystemInfo(const char *deviceMfr,
                                                 const char *deviceModel,
                                                 SystemInfoHandle systemInfoHandle)
{
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (!deviceMfr || !deviceModel || !systemInfo)
    {
        return false;
    }

    systemInfo->machineManufacturer = deviceMfr;
    systemInfo->machineModelName    = deviceModel;
    return true;
}

// This function is part of the version-2 API:
ANGLE_EXPORT bool ANGLEShouldBeUsedForApplication(const RulesHandle rulesHandle,
                                                  int rulesVersion,
                                                  const SystemInfoHandle systemInfoHandle,
                                                  const char *appName)
{
    RuleList *rules               = static_cast<RuleList *>(rulesHandle);
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (!rules || !systemInfo || !appName || (systemInfo->gpus.size() != 1))
    {
        return false;
    }

    Scenario scenario(appName, systemInfo->machineManufacturer.c_str(),
                      systemInfo->machineModelName.c_str());
    Version gpuDriverVersion(systemInfo->gpus[0].detailedDriverVersion.major,
                             systemInfo->gpus[0].detailedDriverVersion.minor,
                             systemInfo->gpus[0].detailedDriverVersion.subMinor,
                             systemInfo->gpus[0].detailedDriverVersion.patch);
    GPU gpuDriver(systemInfo->gpus[0].driverVendor, systemInfo->gpus[0].deviceId, gpuDriverVersion);
    scenario.mDevice.addGPU(gpuDriver);
    scenario.logScenario();

    bool rtn = rules->getUseANGLE(scenario);
    VERBOSE("Application \"%s\" should %s ANGLE\n", appName, rtn ? "use" : "NOT use");

    return rtn;
}

// This function is part of the version-2 API:
ANGLE_EXPORT void ANGLEFreeRulesHandle(const RulesHandle rulesHandle)
{
    RuleList *rules = static_cast<RuleList *>(rulesHandle);
    if (rules)
    {
        delete rules;
    }
}

// This function is part of the version-2 API:
ANGLE_EXPORT void ANGLEFreeSystemInfoHandle(const SystemInfoHandle systemInfoHandle)
{
    angle::SystemInfo *systemInfo = static_cast<angle::SystemInfo *>(systemInfoHandle);
    if (systemInfo)
    {
        delete systemInfo;
    }
}

}  // extern "C"
