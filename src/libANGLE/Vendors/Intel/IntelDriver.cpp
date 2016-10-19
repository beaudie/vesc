#include "IntelDriver.h"

namespace rx
{

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