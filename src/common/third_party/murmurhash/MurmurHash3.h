// MurmurHash3.h: Compatibility shim for MurmurHash3 API atop PMurHash.

#include "PMurHash.h"

namespace angle
{
    static inline void MurmurHash3_x86_32(const void* key, int len, uint32_t seed, void* out) {
        *static_cast<uint32_t*>(out) = PMurHash32(seed, key, len);
    }
}
