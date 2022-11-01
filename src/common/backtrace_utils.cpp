//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils.cpp:
//   Implements the functions related to the backtrace info class.
//

#include "backtrace_utils.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    if defined(ANGLE_ENABLE_UNWIND_BACKTRACE_SUPPORT)
#        define ANGLE_UNWIND_BACKTRACE_ENABLED
#        include <dlfcn.h>
#        include <unwind.h>
#    endif
#endif

#if defined(ANGLE_UNWIND_BACKTRACE_ENABLED)
namespace
{

// Size limit for the backtrace obtained from the device.
constexpr uint32_t kMaxBacktraceSize = 16;

struct UnwindCallbackFnState
{
    UnwindCallbackFnState() : current(nullptr), end(nullptr) {}
    UnwindCallbackFnState(void **current, void **end) : current(current), end(end) {}
    void **current;
    void **end;
};

// Unwind callback function, which is called until the end of stack is reached.
_Unwind_Reason_Code unwindCallbackFn(struct _Unwind_Context *context, void *args)
{
    auto *state = reinterpret_cast<UnwindCallbackFnState *>(args);

    // Get the instruction pointer.
    uintptr_t ip = _Unwind_GetIP(context);
    if (ip == 0)
    {
        return _URC_NO_REASON;
    }

    // The buffer is populated at the current location with the instruction pointer address. The
    // current value is incremented to prepare for the next entry in the stack trace. Once "current"
    // gets to "end", the callback should stop.
    if (state->current == state->end)
    {
        return _URC_END_OF_STACK;
    }

    *state->current++ = reinterpret_cast<void *>(ip);
    return _URC_NO_REASON;
}
}  // namespace
#endif

namespace angle
{
BacktraceInfo::BacktraceInfo() {}
BacktraceInfo::~BacktraceInfo() {}

void BacktraceInfo::clear()
{
    mStackAddresses.clear();
    mStackSymbols.clear();
}

size_t BacktraceInfo::getSize() const
{
    ASSERT(mStackAddresses.size() == mStackSymbols.size());
    return mStackAddresses.size();
}

std::vector<void *> BacktraceInfo::getStackAddresses() const
{
    return mStackAddresses;
}
std::vector<std::string> BacktraceInfo::getStackSymbols() const
{
    return mStackSymbols;
}

void *BacktraceInfo::getStackAddress(size_t index) const
{
    ASSERT(index < mStackAddresses.size());
    return mStackAddresses[index];
}
std::string BacktraceInfo::getStackSymbol(size_t index) const
{
    ASSERT(index < mStackSymbols.size());
    return mStackSymbols[index];
}

#if defined(ANGLE_UNWIND_BACKTRACE_ENABLED)
void BacktraceInfo::populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount)
{
    ASSERT(mStackAddresses.empty() && mStackSymbols.empty());

    for (size_t i = 0; i < stackAddressCount; i++)
    {
        void *stackAddr = stackAddressBuffer[i];
        mStackAddresses.push_back(stackAddr);

        // Get the symbol if possible. dladdr() returns 0 on failure.
        Dl_info dlInfo;
        if (dladdr(stackAddr, &dlInfo) != 0 && dlInfo.dli_sname)
        {
            mStackSymbols.emplace_back(dlInfo.dli_sname);
        }
        else
        {
            mStackSymbols.emplace_back("unknown_symbol");
        }
    }

    ASSERT(mStackAddresses.size() == mStackSymbols.size());
}
#else
void BacktraceInfo::populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount) {}
#endif

#if defined(ANGLE_UNWIND_BACKTRACE_ENABLED)
BacktraceInfo getBacktraceInfo()
{
    void *stackAddrBuffer[kMaxBacktraceSize];

    UnwindCallbackFnState unwindFnState(stackAddrBuffer, stackAddrBuffer + kMaxBacktraceSize);
    _Unwind_Backtrace(unwindCallbackFn, &unwindFnState);

    // The number of the collected IPs is shown by how far "current" has moved.
    auto stackAddressCount = static_cast<size_t>(unwindFnState.current - stackAddrBuffer);

    BacktraceInfo backtraceInfo;
    backtraceInfo.populateBacktraceInfo(stackAddrBuffer, stackAddressCount);
    return backtraceInfo;
}
#else
BacktraceInfo getBacktraceInfo()
{
    return {};
}
#endif

// The following function has been defined in each platform separately.
// - void printBacktraceInfo(BacktraceInfo backtraceInfo);

}  // namespace angle
