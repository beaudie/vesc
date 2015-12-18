//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "common/angleutils.h"
#include "common/debug.h"

#include <stdio.h>
#include <vector>

size_t FormatStringIntoVectorVarArg(std::vector<char>& outBuffer, const char *fmt, va_list vararg)
{
    va_list varargCopy;

    // Attempt to just print to the current buffer
    va_copy(varargCopy, vararg);
    int len = vsnprintf(&(outBuffer.front()), outBuffer.size(), fmt, varargCopy);
    if (len < 0 || static_cast<size_t>(len) >= outBuffer.size())
    {
        // Buffer was not large enough, calculate the required size and resize the buffer
        va_copy(varargCopy, vararg);
        len = vsnprintf(NULL, 0, fmt, varargCopy);
        outBuffer.resize(len + 1);

        // Print again
        va_copy(varargCopy, vararg);
        len = vsnprintf(&(outBuffer.front()), outBuffer.size(), fmt, varargCopy);
    }
    ASSERT(len >= 0);
    return static_cast<size_t>(len);
}

size_t FormatStringIntoVector(std::vector<char>& buffer, const char *fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    size_t length = FormatStringIntoVectorVarArg(buffer, fmt, vararg);
    va_end(vararg);
    return length;
}

std::string FormatStringVarArg(const char *fmt, va_list vararg)
{
    static std::vector<char> buffer(512);

    size_t len = FormatStringIntoVectorVarArg(buffer, fmt, vararg);
    return std::string(&buffer[0], len);
}

std::string FormatString(const char *fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    std::string result = FormatStringVarArg(fmt, vararg);
    va_end(vararg);
    return result;
}
