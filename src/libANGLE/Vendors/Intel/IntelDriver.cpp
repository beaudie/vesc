#include "IntelDriver.h"

namespace rx
{
namespace Vendor_Intel
{
const struct intel_device_info *i965_get_device_info(uint32_t devid);

bool IsHaswell(uint32_t devid)
{
    const intel_device_info *devInfo = i965_get_device_info(devid);
    return devInfo && IS_HASWELL(devInfo);
}

bool IsBroadwell(uint32_t devid)
{
    const intel_device_info *devInfo = i965_get_device_info(devid);
    return devInfo && IS_GEN8(devInfo) && !IS_CHERRYVIEW(devInfo);
}

bool IsSkylake(uint32_t devid)
{
    const intel_device_info *devInfo = i965_get_device_info(devid);
    return devInfo && IS_SKL(devInfo);
}

// Referenced from https://cgit.freedesktop.org/vaapi/intel-driver/tree/src/i965_device_info.c
const struct intel_device_info *i965_get_device_info(uint32_t devid)
{
#undef CHIPSET
#define CHIPSET(id, family, dev, str) \
    case id:                          \
        return &dev##_device_info;
    switch (devid)
    {
// Copied from: https://cgit.freedesktop.org/vaapi/intel-driver/tree/src/i965_pciids.h
#include "i965_pciids.h"
        default:
            return nullptr;
    }
}
}  // namespace Vendor_Intel
}  // namespace rx