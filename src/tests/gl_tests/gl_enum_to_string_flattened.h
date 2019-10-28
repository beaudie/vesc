//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl_enum_to_string_flattened.h:
//   Utility functions for converting GLenums to string.

#ifndef LIBANGLE_GL_ENUM_TO_STRING_FLATTENED_H_
#define LIBANGLE_GL_ENUM_TO_STRING_FLATTENED_H_

#include <ostream>
#include <string>

namespace gl
{
const char *GLenumToStringFlattened(unsigned int value);
}  // namespace gl

#endif  // LIBANGLE_GL_ENUM_TO_STRING_FLATTENED
