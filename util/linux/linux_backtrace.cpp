//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// linux_backtrace.cpp:
//   Implements functions to extract the backtrace from the ANGLE code during execution on Linux.
//

#include "../backtrace_utils.h"

namespace angle
{

// It is possible to refactor the implementation of populating and acquiring the backtrace
// information from the PrintStackBacktrace() function.

void UnwindedBacktraceInfo::populateBacktraceInfo(void **stackAddressBuffer,
                                                  size_t stackAddressCount)
{}

UnwindedBacktraceInfo getBacktraceInfo()
{
    return {};
}

}  // namespace angle
