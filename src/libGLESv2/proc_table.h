//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// getProcAddress loader table:
//   Mapping from a string entry point name to function address.
//

#ifndef LIBGLESV2_PROC_TABLE_H_
#define LIBGLESV2_PROC_TABLE_H_

#include <EGL/egl.h>
#include <utility>

namespace egl
{
struct ProcEntry
{
    constexpr ProcEntry(const char *nameIn, void *procIn) : name(nameIn), proc(procIn) {}

    const char *name;
    void *proc;
};

const ProcEntry *GetProcTable();
size_t GetNumProcs();
}  // namespace egl

#endif  // LIBGLESV2_PROC_TABLE_H_
