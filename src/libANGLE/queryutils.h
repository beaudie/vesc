//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// queryutils.h: GL and EGL query and conversion utilities.

#ifndef LIBANGLE_QUERYUTILS_H_
#define LIBANGLE_QUERYUTILS_H_

#include <vector>

#include "angle_gl.h"

namespace gl
{

class Context;

template <typename QueryT>
std::vector<QueryT> QueryState(const Context *context, GLenum pname);

template <typename QueryT>
std::vector<QueryT> QueryStateIndexed(const Context *context, GLenum pname, size_t index);

}

#endif // LIBANGLE_QUERYUTILS_H_
