//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MappedSubresourceVerifier11.cpp: Implements the
// rx::MappedSubresourceVerifier11 class, a simple wrapper to D3D11 Texture2D
// mapped memory so that ASAN and MSAN can catch memory errors done with a
// pointer to the mapped texture memory.

#include "libANGLE/renderer/d3d/d3d11/MappedSubresourceVerifier11.h"

#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

namespace rx
{

namespace
{

size_t getPitchCount(const D3D11_TEXTURE2D_DESC &desc)
{
    const d3d11::DXGIFormatSize &dxgiFormatInfo = d3d11::GetDXGIFormatSizeInfo(desc.Format);
    ASSERT(desc.Height % dxgiFormatInfo.blockHeight);
    return desc.Height / dxgiFormatInfo.blockHeight;
}

}  // namespace

MappedSubresourceVerifier11::MappedSubresourceVerifier11(const D3D11_TEXTURE2D_DESC &desc)
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    : mPitchType(&D3D11_MAPPED_SUBRESOURCE::RowPitch), mPitchCount(getPitchCount(desc))
#endif
{

    ASSERT(desc.CPUAccessFlags & (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE));
    ASSERT(desc.Width);
    ASSERT(desc.Height);
}

MappedSubresourceVerifier11::MappedSubresourceVerifier11(const D3D11_TEXTURE3D_DESC &desc)
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    : mPitchType(&D3D11_MAPPED_SUBRESOURCE::DepthPitch), mPitchCount(desc.Depth)
#endif
{
    ASSERT(desc.CPUAccessFlags & (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE));
    ASSERT(desc.Width);
    ASSERT(desc.Height);
    ASSERT(desc.Depth);
}

MappedSubresourceVerifier11::~MappedSubresourceVerifier11()
{
    ASSERT(!mOrigData);
}

bool MappedSubresourceVerifier11::wrap(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map)
{
    ASSERT(map && map->pData);
    ASSERT(mapType == D3D11_MAP_READ || mapType == D3D11_MAP_WRITE ||
           mapType == D3D11_MAP_READ_WRITE);

#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    if (!mWrapData.resize(mPitchCount * map->*mPitchType))
        return false;
#endif
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
    mOrigData = reinterpret_cast<uint8_t *>(map->pData);
#endif
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    if (mapType == D3D11_MAP_READ || mapType == D3D11_MAP_READ_WRITE)
    {
        std::copy(mOrigData, mOrigData + mWrapData.size(), mWrapData.data());
    }
    else if (mapType == D3D11_MAP_WRITE)
    {
        // Leave the mWrapData.data() undefined to get MSAN errors on reads.
    }
    map->pData = mWrapData.data();
#endif
    return true;
}

void MappedSubresourceVerifier11::unwrap()
{
    ASSERT(mOrigData);
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    std::copy(mWrapData.data(), mWrapData.data() + mWrapData.size(), mOrigData);
    mWrapData = angle::MemoryBuffer();
#endif
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
    mOrigData = nullptr;
#endif
}

}  // namespace rx
