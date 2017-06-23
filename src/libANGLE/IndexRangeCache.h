//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexRangeCache.h: Defines the gl::IndexRangeCache class which stores information about
// ranges of indices.

#ifndef LIBANGLE_INDEXRANGECACHE_H_
#define LIBANGLE_INDEXRANGECACHE_H_

#include "common/angleutils.h"
#include "common/mathutil.h"

#include "angle_gl.h"

#include <map>
#include <unordered_map>

namespace gl
{

using IndexRangeKey = std::array<size_t, 3>;

class IndexRangeCache
{
  public:
    void addRange(GLenum type,
                  size_t offset,
                  size_t count,
                  bool primitiveRestartEnabled,
                  const IndexRange &range);
    bool findRange(GLenum type,
                   size_t offset,
                   size_t count,
                   bool primitiveRestartEnabled,
                   IndexRange *outRange) const;

    void invalidateRange(size_t offset, size_t size);
    void clear();

  private:
    struct IndexRangeKeyHasher
    {
        size_t operator()(const IndexRangeKey &key) const;
    };

    using IndexRangeMap = std::unordered_map<IndexRangeKey, IndexRange, IndexRangeKeyHasher>;
    IndexRangeMap mIndexRangeCache;
};

}

#endif // LIBANGLE_INDEXRANGECACHE_H_
