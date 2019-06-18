//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// getProcAddress loader table:
//   Mapping from a string entry point name to function address.
//

#ifndef OPENGL32_PROC_TABLE_H_
#define OPENGL32_PROC_TABLE_H_

#include <WGL/wgl.h>
#include <stddef.h>
#include <utility>

namespace wgl
{
using ProcEntry = std::pair<const char *, PROC>;

extern wgl::ProcEntry g_procTable[];
extern size_t g_numProcs;
}  // namespace wgl

#endif  // OPENGL32_PROC_TABLE_H_
