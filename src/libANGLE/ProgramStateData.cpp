//
// Created by yuxinhu on 11/29/22.
//

#include "libANGLE/ProgramStateData.h"

namespace gl
{
ProgramStateData::ProgramStateData()
    : mLabel(),
      mLocationsUsedForXfbExtension(0),
      mBinaryRetrieveableHint(false),
      mSeparable(false),
      mNumViews(-1),
      mDrawIDLocation(-1),
      mBaseVertexLocation(-1),
      mBaseInstanceLocation(-1),
      mCachedBaseVertex(0),
      mCachedBaseInstance(0)
{}

ProgramStateData::~ProgramStateData() = default;
}  // namespace gl
