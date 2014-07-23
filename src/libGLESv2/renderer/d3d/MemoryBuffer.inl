//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>
#include <cstdlib>

namespace rx
{

template <typename T>
MemoryBuffer<T>::MemoryBuffer()
    : mSize(0),
      mData(NULL)
{
}

template <typename T>
MemoryBuffer<T>::~MemoryBuffer()
{
    free(mData);
    mData = NULL;
}

template <typename T>
bool MemoryBuffer<T>::resize(size_t size)
{
    if (size == 0)
    {
        free(mData);
        mData = NULL;
        mSize = 0;
    }
    else
    {
        T *newMemory = reinterpret_cast<T*>(malloc(sizeof(T)* size));
        if (newMemory == NULL)
        {
            return false;
        }

        if (mData)
        {
            // Copy the intersection of the old data and the new data
            std::copy(mData, mData + std::min(mSize, size), newMemory);
            free(mData);

            // Fill in the new data with the default value for T
            if (mSize > size)
            {
                std::fill(newMemory + size, newMemory + mSize, T());
            }
        }

        mData = newMemory;
        mSize = size;
    }

    return true;
}

template <typename T>
size_t MemoryBuffer<T>::size() const
{
    return mSize;
}

template <typename T>
const T *MemoryBuffer<T>::data() const
{
    return mData;
}

template <typename T>
T *MemoryBuffer<T>::data()
{
    return mData;
}

}
