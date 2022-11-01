//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils.h:
//   Tools to extract the backtrace from the ANGLE code during execution.
//

#ifndef COMMON_BACKTRACEUTILS_H_
#define COMMON_BACKTRACEUTILS_H_

#include <string>
#include <vector>
#include "debug.h"

namespace angle
{

// Used to store the backtrace information, such as the stack addresses and symbols.
class BacktraceInfo
{
  public:
    BacktraceInfo();
    ~BacktraceInfo();

    void clear();
    size_t getSize() const;

    [[nodiscard]] std::vector<void *> getStackAddresses() const;
    [[nodiscard]] std::vector<std::string> getStackSymbols() const;

    void *getStackAddress(size_t index) const;
    std::string getStackSymbol(size_t index) const;

    void populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount);

  private:
    std::vector<void *> mStackAddresses;
    std::vector<std::string> mStackSymbols;
};

// Used to obtain the stack addresses and symbols from the device. When
// ANGLE_ENABLE_UNWIND_BACKTRACE_SUPPORT is disabled, it returns an empty object.
BacktraceInfo getBacktraceInfo();

// Used to print the stack addresses and symbols embedded in the BacktraceInfo object.
void printBacktraceInfo(BacktraceInfo backtraceInfo);

}  // namespace angle

#endif  // COMMON_BACKTRACEUTILS_H_
