//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PathMapper.h: Defines PathMapper which maps one range of path handles
// to another range.

#ifndef LIBANGLE_PATHMAPPER_H
#define LIBANGLE_PATHMAPPER_H

#include "common/angleutils.h"
#include "angle_gl.h"

#include <vector>
#include <map>

namespace gl
{

class PathMapper final : angle::NonCopyable
{
  public:
    // Creates a path mapping between closed intervals
    void createMapping(GLuint firstClientHandle,
                       GLuint lastClientHandle,
                       GLuint firstServiceHandle);

    // Returns true if any path exists in the closed interval
    // [firstClientHandle, lastClientHandle].
    bool hasPathsInRange(GLuint firstClientHandle, GLuint lastClientHandle) const;

    // Gets the path id corresponding the client path id.
    // Returns false if no such service path id was not found.
    bool getPath(GLuint clientHandle, GLuint* serviceHandle) const;

    struct RemovedRange
    {
      RemovedRange(GLuint serviceHandle, GLuint range)
        : mFirstServiceHandle(serviceHandle),
          mRange(range)
        {}
      GLuint mFirstServiceHandle;
      GLuint mRange;
    };

    // Removes a closed interval of paths [firstClientHandle, lastClientHandle].
    void removeMapping(GLuint firstClientHandle, GLuint lastClientHandle,
                       std::vector<RemovedRange> *removedRanges);

    void removeAll(std::vector<RemovedRange> *removedRanges);

    // Mapping between client handle and service handle.
    struct Mapping
    {
      Mapping(GLuint lastClient, GLuint firstService)
        : mLastClientHandle(lastClient), mFirstServiceHandle(firstService) {}

      GLuint mLastClientHandle;
      GLuint mFirstServiceHandle;
    };
    typedef std::map<GLuint, Mapping> PathRangeMap;

 private:
    // Checks for consistency inside the book-keeping structures.
    bool checkConsistency() const;

  private:
    PathRangeMap mPathMap;
};

} //gl

#endif // LIBANGLE_PATHMANAGER