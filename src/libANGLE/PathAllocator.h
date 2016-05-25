//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PathAllocator.h: Defines the gl::PathAllocator class, which is used to
// allocate contiguous ranges of GL path handles.

#ifndef LIBANGLE_PATHALLOCATOR_H
#define LIBANGLE_PATHALLOCATOR_H

#include "common/angleutils.h"
#include "angle_gl.h"

#include <map>

namespace gl
{

// Allocates contiguous ranges of path object handles.
class PathAllocator final : angle::NonCopyable
{
  public:
    static const GLuint kInvalidPath;

    PathAllocator();

    // Allocates a new path handle.
    GLuint allocate();

    // Allocates a handle starting at or above the value of |wanted|.
    // Note: may wrap if it starts near limit.
    GLuint allocateAtOrAbove(GLuint wanted);

    // Allocates |range| amount of contiguous paths.
    // Returns the first id to |first_id| or |kInvalidPath| if
    // allocation failed.
    GLuint allocateRange(GLuint range);

    // Marks an id as used. Returns false if handle was already used.
    bool markAsUsed(GLuint handle);

    // Release handle.
    void release(GLuint handle);

    // Release a |range| amount of contiguous handles, starting from |first|
    void releaseRange(GLuint first, GLuint range);

    // Checks whether or not a resource ID is in use.
    bool isUsed(GLuint handle) const;

  private:
    typedef std::map<GLuint, GLuint> RangeMap;

    RangeMap mUsed;
};

} // gl

#endif // LIBANGLE_PATHALLOCATOR_H