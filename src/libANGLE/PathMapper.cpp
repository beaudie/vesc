//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/PathMapper.h"

#include "common/angleutils.h"
#include "common/debug.h"

#include <algorithm>
#include <limits>

namespace gl
{
namespace
{

template <typename RangeIterator> inline
GLuint getRangeSize(const RangeIterator& it)
{
    return it->second.mLastClientHandle - it->first + 1;
}

template <typename RangeIterator> inline
GLuint getFirstClientHandle(const RangeIterator& it)
{
    return it->first;
}

template <typename RangeIterator> inline
GLuint getFirstServiceHandle(const RangeIterator& it)
{
    return it->second.mFirstServiceHandle;
}

template <typename RangeIterator> inline
GLuint getLastServiceHandle(const RangeIterator& it)
{
    return getFirstServiceHandle(it) + getRangeSize(it) - 1;
}

inline GLuint getLastClientHandle(PathMapper::PathRangeMap::const_iterator& it)
{
    return it->second.mLastClientHandle;
}


inline GLuint& getLastClientHandle(PathMapper::PathRangeMap::iterator& it)
{
    return it->second.mLastClientHandle;
}

template <typename T>
struct IteratorSelector {
  typedef typename T::iterator iterator;
};
template <typename T>
struct IteratorSelector<const T> {
  typedef typename T::const_iterator iterator;
};

// Returns the range position that contains |clientHandle| or
// |PathRangeMap::iterator::end()| if |clientHandle| is not found.
template <typename MapType>
typename IteratorSelector<MapType>::iterator getContainingRange(MapType& path_map, GLuint clientHandle)
{
    auto it = path_map.lower_bound(clientHandle);
    if (it != path_map.end() && getFirstClientHandle(it) == clientHandle)
        return it;

    if (it != path_map.begin())
    {
        --it;
        if (getLastClientHandle(it) >= clientHandle)
            return it;
    }
    return path_map.end();
}

// Returns the range position that contains |clientHandle|. If that is
// not available, returns the range that has smallest
// |firstClientHandle| that is bigger than |clientHandle|. Returns
// |PathRangeMap::iterator::end()| if there is no such range.
template <typename MapType>
typename IteratorSelector<MapType>::iterator getContainingOrNextRange(MapType& path_map, GLuint clientHandle)
{
    auto it = path_map.lower_bound(clientHandle);
    if (it != path_map.end() && getFirstClientHandle(it) == clientHandle)
        return it;

    if (it != path_map.begin())
    {
        --it;
        if (getLastClientHandle(it) >= clientHandle)
            return it;
        ++it;
    }
    return it;
}

}  // anonymous namespace

void PathMapper::createMapping(GLuint firstClientHandle,
                               GLuint lastClientHandle,
                               GLuint firstServiceHandle)
{
    ASSERT(firstServiceHandle > 0u);
    ASSERT(firstClientHandle > 0u);
    ASSERT(!hasPathsInRange(firstClientHandle, lastClientHandle));
    ASSERT(checkConsistency());

    auto range = getContainingRange(mPathMap, firstClientHandle - 1u);

    if (range != mPathMap.end() && getLastServiceHandle(range) == firstServiceHandle - 1u)
    {
        ASSERT(getLastClientHandle(range) == firstClientHandle - 1u);
        getLastClientHandle(range) = lastClientHandle;
    }
    else
    {
        auto result = mPathMap.insert(std::make_pair(firstClientHandle, Mapping(lastClientHandle, firstServiceHandle)));
        ASSERT(result.second);
        range = result.first;
    }

    auto next_range = range;
    ++next_range;
    if (next_range != mPathMap.end())
    {
        if (getLastClientHandle(range) == getFirstClientHandle(next_range) - 1u &&
            getLastServiceHandle(range) == getFirstServiceHandle(next_range) - 1u)
        {
            getLastClientHandle(range) = getLastClientHandle(next_range);
            mPathMap.erase(next_range);
        }
    }
    ASSERT(checkConsistency());
}

bool PathMapper::hasPathsInRange(GLuint firstClientHandle,
                                 GLuint lastClientHandle) const
{
    auto it = getContainingOrNextRange(mPathMap, firstClientHandle);
    if (it == mPathMap.end())
        return false;

    return getFirstClientHandle(it) <= lastClientHandle;
}

bool PathMapper::getPath(GLuint clientHandle, GLuint* serviceHandle) const
{
    auto range = getContainingRange(mPathMap, clientHandle);
    if (range == mPathMap.end())
        return false;

    *serviceHandle = getFirstServiceHandle(range) + clientHandle - getFirstClientHandle(range);
    return true;
}

void PathMapper::removeMapping(GLuint firstClientHandle, GLuint lastClientHandle,
                               std::vector<RemovedRange> *removedRanges)
{
    ASSERT(checkConsistency());

    auto it = getContainingOrNextRange(mPathMap, firstClientHandle);

    while (it != mPathMap.end() && getFirstClientHandle(it) <= lastClientHandle)
    {
        const GLuint deleteFirstClientHandle  = std::max(firstClientHandle, getFirstClientHandle(it));
        const GLuint deleteLastClientHandle   = std::min(lastClientHandle, getLastClientHandle(it));
        const GLuint deleteFirstServiceHandle = getFirstServiceHandle(it) + deleteFirstClientHandle - getFirstClientHandle(it);
        const GLuint deleteRange              = deleteLastClientHandle - deleteFirstClientHandle + 1u;

        removedRanges->push_back(RemovedRange(deleteFirstServiceHandle, deleteRange));

        auto current = it;
        ++it;

        const GLuint currentLastClientHandle = getLastClientHandle(current);

        if (getFirstClientHandle(current) < deleteFirstClientHandle)
            getLastClientHandle(current) = deleteFirstClientHandle - 1u;
        else
            mPathMap.erase(current);

        if (currentLastClientHandle > deleteLastClientHandle)
        {
            mPathMap.insert(std::make_pair(deleteLastClientHandle + 1u,
                Mapping(currentLastClientHandle, deleteFirstServiceHandle + deleteRange)));

            ASSERT(deleteLastClientHandle == lastClientHandle);
            // This is necessarily the last range to check. Return early due to
            // consistency. Iterator increment would skip the inserted range. The
            // algorithm would work ok, but it looks weird.
            ASSERT(checkConsistency());
            return;
        }
    }
    ASSERT(checkConsistency());
}

void PathMapper::removeAll(std::vector<RemovedRange> *removedRanges)
{
    for (auto it = mPathMap.begin(); it != mPathMap.end(); ++it)
    {
        removedRanges->push_back(RemovedRange(getFirstServiceHandle(it), getRangeSize(it)));
    }
    mPathMap.clear();
}

bool PathMapper::checkConsistency() const
{
    GLuint prevFirstClientHandle  = 0u;
    GLuint prevLastClientHandle   = 0u;
    GLuint prevFirstServiceHandle = 0u;
    for (auto range = mPathMap.begin(); range != mPathMap.end(); ++range)
    {
        // Code relies on ranges not starting at 0. Also, the above initialization
        // is only correct then.
        if (getFirstClientHandle(range) == 0u || getFirstServiceHandle(range) == 0u)
            return false;

        // Each range is consistent.
        if (getFirstClientHandle(range) > getLastClientHandle(range))
            return false;

        if (prevFirstClientHandle != 0u)
        {
            // No overlapping ranges. (The iteration is sorted).
            if (getFirstClientHandle(range) <= prevLastClientHandle)
                return false;

            // No mergeable ranges.
            const bool isMergeableClient  = getFirstClientHandle(range) - 1u == prevLastClientHandle;
            const bool isMergeableService = getFirstServiceHandle(range) - 1u == prevFirstServiceHandle;
            if (isMergeableClient && isMergeableService)
                return false;
        }
        prevFirstClientHandle  = getFirstClientHandle(range);
        prevLastClientHandle   = getLastClientHandle(range);
        prevFirstServiceHandle = getFirstServiceHandle(range);
    }
    return true;
}


} // gl