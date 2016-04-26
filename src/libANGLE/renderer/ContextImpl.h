//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextImpl:
//   Implementation-specific functionality associated with a GL Context.
//

#ifndef LIBANGLE_RENDERER_CONTEXTIMPL_H_
#define LIBANGLE_RENDERER_CONTEXTIMPL_H_

#include "common/angleutils.h"

namespace rx
{

class ContextImpl : angle::NonCopyable
{
  public:
    ContextImpl() {}
    virtual ~ContextImpl() {}
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CONTEXTIMPL_H_
