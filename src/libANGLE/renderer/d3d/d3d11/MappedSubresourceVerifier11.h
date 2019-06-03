//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MappedSubresourceVerifier11.h: Defines the rx::MappedSubresourceVerifier11
// class, a simple wrapper to D3D11 Texture2D mapped memory so that ASAN and
// MSAN can catch memory errors done with a pointer to the mapped texture
// memory.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"

namespace rx
{

class MappedSubresourceVerifier11 final : angle::NonCopyable
{
  public:
    MappedSubresourceVerifier11(const D3D11_TEXTURE2D_DESC &desc);
    MappedSubresourceVerifier11(const D3D11_TEXTURE3D_DESC &desc);
    ~MappedSubresourceVerifier11();

    bool wrap(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map)
    {
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
        return wrapImpl(mapType, map);
#else
        return true;
#endif
    }

    void unwrap()
    {
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
        unwrapImpl();
#endif
    }

  private:
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
    bool wrapImpl(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map);
    void unwrapImpl();

    uint8_t *mOrigData = nullptr;
#endif
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER)
    const UINT D3D11_MAPPED_SUBRESOURCE::*mPitchType;
    const size_t mPitchCount;
    angle::MemoryBuffer mWrapData;
#endif
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_
