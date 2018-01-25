//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImmutableString.h: Wrapper for static or pool allocated char arrays, that are guaranteed to be
// valid and unchanged for the duration of the compilation.
//

#ifndef COMPILER_TRANSLATOR_IMMUTABLESTRING_H_
#define COMPILER_TRANSLATOR_IMMUTABLESTRING_H_

#include <string>

#include "common/string_utils.h"
#include "compiler/translator/Common.h"

namespace sh
{

namespace
{
constexpr size_t constStrlen(const char *str)
{
    if (str == nullptr)
    {
        return 0u;
    }
    size_t len = 0u;
    while (*(str + len) != '\0')
    {
        ++len;
    }
    return len;
}
}

class ImmutableString
{
  public:
    // The data pointer passed in must be one of:
    //  1. nullptr (only valid with length 0).
    //  2. a null-terminated static char array like a string literal.
    //  3. a null-terminated pool allocated char array. This can't be c_str() of a local TString,
    //     since when a TString goes out of scope it clears its first character.
    explicit constexpr ImmutableString(const char *data) : mData(data), mLength(constStrlen(data))
    {
    }

    constexpr ImmutableString(const char *data, size_t length) : mData(data), mLength(length) {}

    ImmutableString(const std::string &str)
        : mData(AllocatePoolCharArray(str.c_str(), str.size())), mLength(str.size())
    {
    }

    ImmutableString(const std::string &str, size_t begin, size_t length)
        : mData(AllocatePoolCharArray(str.c_str() + begin, length)), mLength(length)
    {
    }

    static ImmutableString PoolAllocate(const char *data, size_t length)
    {
        ASSERT(data != nullptr);
        return ImmutableString(AllocatePoolCharArray(data, length), length);
    }

    ImmutableString(const ImmutableString &) = default;
    ImmutableString &operator=(const ImmutableString &) = default;

    const char *data() const { return mData ? mData : ""; }
    size_t length() const { return mLength; }

    operator const char *() const { return data(); }

    bool empty() const { return mLength == 0; }
    bool beginsWith(const char *prefix) const { return angle::BeginsWith(data(), prefix); }
    bool contains(const char *substr) const { return strstr(data(), substr) != nullptr; }

    bool operator==(const ImmutableString &b) const
    {
        if (mLength != b.mLength)
        {
            return false;
        }
        return memcmp(data(), b.data(), mLength) == 0;
    }
    bool operator!=(const ImmutableString &b) const { return !(*this == b); }
    bool operator==(const char *b) const
    {
        if (b == nullptr)
        {
            return empty();
        }
        return strcmp(data(), b) == 0;
    }
    bool operator!=(const char *b) const { return !(*this == b); }
    bool operator==(const std::string &b) const { return *this == ImmutableString(b); }
    bool operator!=(const std::string &b) const { return !(*this == b); }

    bool operator<(const ImmutableString &b) const
    {
        if (mLength < b.mLength)
        {
            return true;
        }
        if (mLength > b.mLength)
        {
            return false;
        }
        return (memcmp(data(), b.data(), mLength) < 0);
    }

    struct FowlerNollVoHash
    {
        size_t operator()(const ImmutableString &a) const
        {
            if (a.empty())
            {
                return 0u;
            }
            const char *data             = a.mData;
            const size_t kFnvOffsetBasis = 0xcbf29ce484222325ull;
            const size_t kFnvPrime       = 1099511628211ull;
            size_t hash                  = kFnvOffsetBasis;
            while ((*data) != '\0')
            {
                hash = hash ^ (*data);
                hash = hash * kFnvPrime;
                ++data;
            }
            return hash;
        }
    };

  private:
    const char *mData;
    size_t mLength;
};

}  // namespace sh

std::ostream &operator<<(std::ostream &os, const sh::ImmutableString &str);

#endif  // COMPILER_TRANSLATOR_IMMUTABLESTRING_H_
