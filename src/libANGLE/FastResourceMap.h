//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FastResourceMap:
//   An optimized resource map which packs the first set of allocated objects into a
//   flat array, and then falls back to an unordered map for the higher handle values.
//

#ifndef LIBANGLE_FAST_RESOURCE_MAP_H_
#define LIBANGLE_FAST_RESOURCE_MAP_H_

#include "libANGLE/angletypes.h"

namespace gl
{

template <typename ResourceType>
class FastResourceMap final : angle::NonCopyable
{
  public:
    FastResourceMap();
    ~FastResourceMap();

    ResourceType *query(GLuint handle) const;

    // Returns true if the handle was reserved. Not necessarily if the resource is created.
    bool contains(GLuint handle) const;

    // Returns the element that was at this location.
    bool erase(GLuint handle, ResourceType **resourceOut);

    void assign(GLuint handle, ResourceType *resource);

    // Clears the map.
    void clear();

    using IndexAndResource = std::pair<GLuint, ResourceType *>;

    class Iterator final
    {
      public:
        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        Iterator &operator++();
        IndexAndResource operator->() const;
        IndexAndResource operator*() const;

      private:
        friend class FastResourceMap;
        Iterator(const FastResourceMap &origin,
                 size_t flatIndex,
                 typename ResourceMap<ResourceType>::const_iterator hashIndex);

        const FastResourceMap &mOrigin;
        size_t mFlatIndex;
        typename ResourceMap<ResourceType>::const_iterator mHashIndex;
    };

    // May return null values.
    Iterator begin() const;
    Iterator end() const;

    // Not a constant-time operation, should only be used for verification.
    bool empty() const;

  private:
    friend class Iterator;

    size_t nextNonNullResource(size_t flatIndex) const;

    // constexpr methods cannot contain reinterpret_cast, so we need a static method.
    static ResourceType *InvalidPointer();

    static constexpr size_t kInitialFlatResourcesSize = 32u;
    static constexpr size_t kFlatResourcesLimit       = 1024u;
    static constexpr intptr_t kInvalidPointer         = static_cast<intptr_t>(-1);

    std::vector<ResourceType *> mFlatResources;
    ResourceMap<ResourceType> mHashedResources;
};

template <typename ResourceType>
FastResourceMap<ResourceType>::FastResourceMap()
    : mFlatResources(kInitialFlatResourcesSize, InvalidPointer()), mHashedResources()
{
}

template <typename ResourceType>
FastResourceMap<ResourceType>::~FastResourceMap()
{
    ASSERT(empty());
}

template <typename ResourceType>
ResourceType *FastResourceMap<ResourceType>::query(GLuint handle) const
{
    if (handle < mFlatResources.size())
    {
        auto value = mFlatResources[handle];
        return (value == InvalidPointer() ? nullptr : value);
    }
    auto it = mHashedResources.find(handle);
    return (it == mHashedResources.end() ? nullptr : it->second);
}

template <typename ResourceType>
bool FastResourceMap<ResourceType>::contains(GLuint handle) const
{
    if (handle < mFlatResources.size())
    {
        return (mFlatResources[handle] != InvalidPointer());
    }
    return (mHashedResources.find(handle) != mHashedResources.end());
}

template <typename ResourceType>
bool FastResourceMap<ResourceType>::erase(GLuint handle, ResourceType **resourceOut)
{
    if (handle < mFlatResources.size())
    {
        auto &value = mFlatResources[handle];
        if (value == InvalidPointer())
        {
            return false;
        }
        *resourceOut = value;
        value        = InvalidPointer();
    }
    else
    {
        auto it = mHashedResources.find(handle);
        if (it == mHashedResources.end())
        {
            return false;
        }
        *resourceOut = it->second;
        mHashedResources.erase(it);
    }
    return true;
}

template <typename ResourceType>
void FastResourceMap<ResourceType>::assign(GLuint handle, ResourceType *resource)
{
    if (handle < kFlatResourcesLimit)
    {
        if (handle >= mFlatResources.size())
        {
            // Use power-of-two.
            size_t newSize = mFlatResources.size();
            while (newSize <= handle)
            {
                newSize *= 2;
            }
            mFlatResources.resize(newSize, nullptr);
        }
        ASSERT(mFlatResources.size() > handle);
        mFlatResources[handle] = resource;
    }
    else
    {
        mHashedResources[handle] = resource;
    }
}

template <typename ResourceType>
typename FastResourceMap<ResourceType>::Iterator FastResourceMap<ResourceType>::begin() const
{
    return Iterator(*this, nextNonNullResource(0), mHashedResources.begin());
}

template <typename ResourceType>
typename FastResourceMap<ResourceType>::Iterator FastResourceMap<ResourceType>::end() const
{
    return Iterator(*this, mFlatResources.size(), mHashedResources.end());
}

template <typename ResourceType>
bool FastResourceMap<ResourceType>::empty() const
{
    return (begin() == end());
}

template <typename ResourceType>
void FastResourceMap<ResourceType>::clear()
{
    mFlatResources.assign(kInitialFlatResourcesSize, InvalidPointer());
    mHashedResources.clear();
}

template <typename ResourceType>
size_t FastResourceMap<ResourceType>::nextNonNullResource(size_t flatIndex) const
{
    for (size_t index = flatIndex; index < mFlatResources.size(); index++)
    {
        if (mFlatResources[index] != nullptr && mFlatResources[index] != InvalidPointer())
        {
            return index;
        }
    }
    return mFlatResources.size();
}

template <typename ResourceType>
// static
ResourceType *FastResourceMap<ResourceType>::InvalidPointer()
{
    return reinterpret_cast<ResourceType *>(kInvalidPointer);
}

template <typename ResourceType>
FastResourceMap<ResourceType>::Iterator::Iterator(
    const FastResourceMap &origin,
    size_t flatIndex,
    typename ResourceMap<ResourceType>::const_iterator hashIndex)
    : mOrigin(origin), mFlatIndex(flatIndex), mHashIndex(hashIndex)
{
}

template <typename ResourceType>
bool FastResourceMap<ResourceType>::Iterator::operator==(const Iterator &other) const
{
    return (mFlatIndex == other.mFlatIndex && mHashIndex == other.mHashIndex);
}

template <typename ResourceType>
bool FastResourceMap<ResourceType>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <typename ResourceType>
typename FastResourceMap<ResourceType>::Iterator
    &FastResourceMap<ResourceType>::Iterator::operator++()
{
    if (mFlatIndex < mOrigin.mFlatResources.size())
    {
        mFlatIndex = mOrigin.nextNonNullResource(mFlatIndex + 1);
    }
    else
    {
        mHashIndex++;
    }
    return *this;
}

template <typename ResourceType>
typename FastResourceMap<ResourceType>::IndexAndResource FastResourceMap<ResourceType>::Iterator::
operator->() const
{
    if (mFlatIndex < mOrigin.mFlatResources.size())
    {
        return IndexAndResource(static_cast<GLuint>(mFlatIndex),
                                mOrigin.mFlatResources[mFlatIndex]);
    }
    return *mHashIndex;
}

template <typename ResourceType>
typename FastResourceMap<ResourceType>::IndexAndResource FastResourceMap<ResourceType>::Iterator::
operator*() const
{
    if (mFlatIndex < mOrigin.mFlatResources.size())
    {
        return IndexAndResource(static_cast<GLuint>(mFlatIndex),
                                mOrigin.mFlatResources[mFlatIndex]);
    }
    return *mHashIndex;
}

}  // namespace gl

#endif  // LIBANGLE_FAST_RESOURCE_MAP_H_
