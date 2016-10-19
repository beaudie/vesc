#include <set>
#include "IntelDriver.h"

namespace rx
{
namespace Vendor_Intel
{

inline bool DeviceIdInSet(uint32_t DeviceId, std::set<uint32_t> Set)
{
    return Set.find(DeviceId) != Set.end();
}

bool IsHaswell(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, Haswell);
}

bool IsBroadwell(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, Broadwell);
}

bool IsCherryView(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, CherryView);
}

bool IsSkylake(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, Skylake);
}

bool IsBroxton(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, Broxton);
}

bool IsKabylake(uint32_t DeviceId)
{
    return DeviceIdInSet(DeviceId, Kabylake);
}

}  // namespace Vendor_Intel
}  // namespace rx