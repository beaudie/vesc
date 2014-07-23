//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBGLESV2_RENDERER_D3D_MEMORYBUFFER_H_
#define LIBGLESV2_RENDERER_D3D_MEMORYBUFFER_H_

#include <cstddef>

namespace rx
{

template <typename T>
class MemoryBuffer
{
  public:
    MemoryBuffer();
    ~MemoryBuffer();

    bool resize(size_t size);
    size_t size() const;

    const T *data() const;
    T *data();

  private:
    size_t mSize;
    T *mData;
};

}

#include "MemoryBuffer.inl"

#endif // LIBGLESV2_RENDERER_D3D_MEMORYBUFFER_H
