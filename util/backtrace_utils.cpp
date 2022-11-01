//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils.cpp:
//   Implements backtrace utils functions common to all platforms.
//

#include "backtrace_utils.h"

namespace angle
{
UnwindedBacktraceInfo::UnwindedBacktraceInfo() {}
UnwindedBacktraceInfo::~UnwindedBacktraceInfo() {}

void UnwindedBacktraceInfo::clear()
{
    mStackAddresses.clear();
    mStackSymbols.clear();
}

void UnwindedBacktraceInfo::printBacktrace()
{
    // Return if no backtrace data is available.
    if (mStackAddresses.empty())
    {
        return;
    }

    WARN() << "Backtrace start";
    for (size_t i = 0; i < mStackAddresses.size(); i++)
    {
        WARN() << i << ":" << mStackAddresses[i] << " -> " << mStackSymbols[i];
    }
    WARN() << "Backtrace end";
}

std::vector<void *> UnwindedBacktraceInfo::getStackAddresses() const
{
    return mStackAddresses;
}
std::vector<std::string> UnwindedBacktraceInfo::getStackSymbols() const
{
    return mStackSymbols;
}

// The following functions have been defined in each platform separately.
// - void UnwindedBacktraceInfo::populateBacktraceInfo(void **stackAddressBuffer, size_t
// stackAddressCount);
// - UnwindedBacktraceInfo getBacktraceInfo();

}  // namespace angle
