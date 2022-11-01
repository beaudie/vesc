//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils.h:
//   Tools to extract the backtrace from the ANGLE code during execution.
//

#ifndef UTIL_BACKTRACEUTILS_H_
#define UTIL_BACKTRACEUTILS_H_

#include <string>
#include <vector>
#include "common/debug.h"

namespace angle
{

// Used to store the backtrace information, such as the stack addresses and symbols.
class UnwindedBacktraceInfo
{
  public:
    UnwindedBacktraceInfo();
    ~UnwindedBacktraceInfo();

    void clear();
    void populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount);
    void printBacktrace();

    [[nodiscard]] std::vector<void *> getStackAddresses() const;
    [[nodiscard]] std::vector<std::string> getStackSymbols() const;

  private:
    std::vector<void *> mStackAddresses;
    std::vector<std::string> mStackSymbols;
};

// Used to obtain the stack addresses and symbols from the device. When
// ANGLE_ENABLE_UNWIND_BACKTRACE_SUPPORT is disabled, it returns an empty object.
UnwindedBacktraceInfo getBacktraceInfo();

}  // namespace angle

#endif  // UTIL_BACKTRACEUTILS_H_
