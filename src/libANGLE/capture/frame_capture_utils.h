//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.h:
//   ANGLE frame capture utils interface.
//
#ifndef FRAME_CAPTURE_UTILS_H_
#define FRAME_CAPTURE_UTILS_H_

#include "libANGLE/Error.h"

namespace gl
{
class JsonSerializer;
class Context;
}  // namespace gl

namespace angle
{
Result SerializeContext(gl::JsonSerializer *bos, const gl::Context *context);
}  // namespace angle
#endif  // FRAME_CAPTURE_UTILS_H_
