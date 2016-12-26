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

IntelDriverVersion::IntelDriverVersion()
    : mVersionPart1(0), mVersionPart2(0), mVersionPart3(0), mVersionPart4(0)
{
}

IntelDriverVersion::IntelDriverVersion(uint16_t part1,
                                       uint16_t part2,
                                       uint16_t part3,
                                       uint16_t part4)
    : mVersionPart1(part1), mVersionPart2(part2), mVersionPart3(part3), mVersionPart4(part4)
{
}

bool IntelDriverVersion::operator==(const IntelDriverVersion &version)
{
    return mVersionPart1 == version.mVersionPart1 && mVersionPart2 == version.mVersionPart2 &&
           mVersionPart3 == version.mVersionPart3 && mVersionPart4 == version.mVersionPart4;
}

bool IntelDriverVersion::operator!=(const IntelDriverVersion &version)
{
    return !(*this == version);
}

bool IntelDriverVersion::operator<(const IntelDriverVersion &version)
{
    // See http://www.intel.com/content/www/us/en/support/graphics-drivers/000005654.html to
    // understand the Intel graphics driver version number.
    // mVersionPart1 changes with OS version. mVersionPart2 changes with DirectX version.
    // mVersionPart3 stands for release year. mVersionPart4 is driver specific unique version
    // number.
    // For example: Intel driver version '20.19.15.4539'
    //              20   -> windows 10 driver
    //              19   -> DirectX 12 first version(12.0) supported
    //              15   -> Driver released in 2015
    //              4539 -> Driver specific unique version number
    // So, we only need to check the last part.
    return mVersionPart4 < version.mVersionPart4;
}

bool IntelDriverVersion::operator>=(const IntelDriverVersion &version)
{
    return !(*this < version);
}

IntelDriverVersion *getIntelDriverVersion(std::string version)
{
    if (version.empty())
        return new IntelDriverVersion();

    std::stringstream ss(version);
    uint16_t part1, part2, part3, part4;
    ss >> part1;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '.') >> part2;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '.') >> part3;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '.') >> part4;
    return new IntelDriverVersion(part1, part2, part3, part4);
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