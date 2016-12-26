//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// driver_utils.h : provides more information about current driver.

#include <algorithm>
#include <sstream>
#include <string>

#include "libANGLE/renderer/driver_utils.h"

namespace rx
{
// Intel
// Referenced from https://cgit.freedesktop.org/vaapi/intel-driver/tree/src/i965_pciids.h
namespace
{
// gen7
const uint32_t Haswell[] = {
    0x0402, 0x0406, 0x040A, 0x040B, 0x040E, 0x0C02, 0x0C06, 0x0C0A, 0x0C0B, 0x0C0E,
    0x0A02, 0x0A06, 0x0A0A, 0x0A0B, 0x0A0E, 0x0D02, 0x0D06, 0x0D0A, 0x0D0B, 0x0D0E,  // hsw_gt1
    0x0412, 0x0416, 0x041A, 0x041B, 0x041E, 0x0C12, 0x0C16, 0x0C1A, 0x0C1B, 0x0C1E,
    0x0A12, 0x0A16, 0x0A1A, 0x0A1B, 0x0A1E, 0x0D12, 0x0D16, 0x0D1A, 0x0D1B, 0x0D1E,  // hsw_gt2
    0x0422, 0x0426, 0x042A, 0x042B, 0x042E, 0x0C22, 0x0C26, 0x0C2A, 0x0C2B, 0x0C2E,
    0x0A22, 0x0A26, 0x0A2A, 0x0A2B, 0x0A2E, 0x0D22, 0x0D26, 0x0D2A, 0x0D2B, 0x0D2E  // hsw_gt3
};

// gen8
const uint32_t Broadwell[] = {0x1602, 0x1606, 0x160A, 0x160B, 0x160D, 0x160E,
                              0x1612, 0x1616, 0x161A, 0x161B, 0x161D, 0x161E,
                              0x1622, 0x1626, 0x162A, 0x162B, 0x162D, 0x162E};

const uint32_t CherryView[] = {0x22B0, 0x22B1, 0x22B2, 0x22B3};

// gen9
const uint32_t Skylake[] = {0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913, 0x1915, 0x1916,
                            0x1917, 0x191A, 0x191B, 0x191D, 0x191E, 0x1921, 0x1923, 0x1926, 0x1927,
                            0x192A, 0x192B, 0x192D, 0x1932, 0x193A, 0x193B, 0x193D};

const uint32_t Broxton[] = {0x0A84, 0x1A84, 0x1A85, 0x5A84, 0x5A85};

// gen9p5
const uint32_t Kabylake[] = {0x5916, 0x5913, 0x5906, 0x5926, 0x5921, 0x5915, 0x590E,
                             0x591E, 0x5912, 0x5917, 0x5902, 0x591B, 0x593B, 0x590B,
                             0x591A, 0x590A, 0x591D, 0x5908, 0x5923, 0x5927};

}  // anonymous namespace

IntelDriverVersion::IntelDriverVersion(const std::string version)
    : mVersion(version),
      mIntVersion(0),
      mVersionPart1(0),
      mVersionPart2(0),
      mVersionPart3(0),
      mVersionPart4(0)
{
    initialize(version);
}

IntelDriverVersion::IntelDriverVersion(const int64_t version)
    : mVersion(std::string()),
      mIntVersion(version),
      mVersionPart1(0),
      mVersionPart2(0),
      mVersionPart3(0),
      mVersionPart4(0)
{
    initialize(version);
}

void IntelDriverVersion::initialize(const std::string version)
{
    if (version.empty())
        return;
    size_t pos1   = version.find_first_of('.', 0);
    mVersionPart1 = convertToInt(version.substr(0, pos1));
    size_t pos2   = version.find_first_of('.', pos1 + 1);
    mVersionPart2 = convertToInt(version.substr(pos1 + 1, pos2 - pos1 - 1));
    size_t pos3   = version.find_first_of('.', pos2 + 1);
    mVersionPart3 = convertToInt(version.substr(pos2 + 1, pos3 - pos2 - 1));
    mVersionPart4 = convertToInt(version.substr(pos3 + 1));
}

void IntelDriverVersion::initialize(const int64_t version)
{
    mVersionPart1 = (version >> 48) & 0xffff;
    mVersionPart2 = (version >> 32) & 0xffff;
    mVersionPart3 = (version >> 16) & 0xffff;
    mVersionPart4 = version & 0xffff;
}

int IntelDriverVersion::convertToInt(std::string str)
{
    int num;
    std::stringstream ss(str);
    ss >> num;
    return num;
}

std::string IntelDriverVersion::getVersion()
{
    if (!mVersion.empty())
        return mVersion;

    std::stringstream ss;
    ss << mVersionPart1 << '.' << mVersionPart2 << '.' << mVersionPart3 << '.' << mVersionPart4;
    std::string result = ss.str();
    return result;
}

int IntelDriverVersion::getVersionPart1()
{
    return mVersionPart1;
}

int IntelDriverVersion::getVersionPart2()
{
    return mVersionPart2;
}

int IntelDriverVersion::getVersionPart3()
{
    return mVersionPart3;
}

int IntelDriverVersion::getVersionPart4()
{
    return mVersionPart4;
}

bool operator==(IntelDriverVersion version1, IntelDriverVersion version2)
{
    return !(std::strcmp(version1.getVersion().c_str(), version2.getVersion().c_str()));
}

bool operator!=(IntelDriverVersion version1, IntelDriverVersion version2)
{
    return !(version1 == version2);
}

bool operator<(IntelDriverVersion version1, IntelDriverVersion version2)
{
    // mVersionPart4 will increase with driver release. The first three parts are determined by
    // compiler, GPU codename.
    return version1.getVersionPart1() <= version2.getVersionPart1() &&
           version1.getVersionPart2() <= version2.getVersionPart2() &&
           version1.getVersionPart3() <= version2.getVersionPart3() &&
           version1.getVersionPart4() < version2.getVersionPart4();
}

bool operator>=(IntelDriverVersion version1, IntelDriverVersion version2)
{
    return !(version1 < version2);
}

bool IsHaswell(uint32_t DeviceId)
{
    return std::find(std::begin(Haswell), std::end(Haswell), DeviceId) != std::end(Haswell);
}

bool IsBroadwell(uint32_t DeviceId)
{
    return std::find(std::begin(Broadwell), std::end(Broadwell), DeviceId) != std::end(Broadwell);
}

bool IsCherryView(uint32_t DeviceId)
{
    return std::find(std::begin(CherryView), std::end(CherryView), DeviceId) !=
           std::end(CherryView);
}

bool IsSkylake(uint32_t DeviceId)
{
    return std::find(std::begin(Skylake), std::end(Skylake), DeviceId) != std::end(Skylake);
}

bool IsBroxton(uint32_t DeviceId)
{
    return std::find(std::begin(Broxton), std::end(Broxton), DeviceId) != std::end(Broxton);
}

bool IsKabylake(uint32_t DeviceId)
{
    return std::find(std::begin(Kabylake), std::end(Kabylake), DeviceId) != std::end(Kabylake);
}

}  // namespace rx